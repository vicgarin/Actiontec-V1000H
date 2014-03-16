/* vi: set sw=4 ts=4: */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>

#include <grp.h>
#include <pwd.h>

#ifdef FEATURE_UTMP
#include <utmp.h>
#endif

#include "busybox.h"

// login defines
#define LOGIN_PROMPT "\n%s login: "

#ifndef TIMEOUT
#define TIMEOUT 60
#endif

#ifndef FAIL_DELAY
#define FAIL_DELAY 3
#endif

/* Stuff global to this file */
#ifdef FEATURE_UTMP
static struct utmp utent;
#endif
static struct passwd pwent;

static int su_mode = 0;

#ifdef CHK_NOLOGIN
static void check_nologin();
#endif

#if defined TLG_FEATURE_SECURETTY
static int check_tty(const char *tty);
#else
/* #define check_tty(foo) 0 */
#define check_tty(foo) 1
#endif

static int is_my_tty(const char *tty);
static void login_prompt(const char *prompt, char *name, int namesize);

#ifdef FEATURE_MOTD
static void motd(void);
#endif

static int pw_auth(const char *cipher, const char *user);
static int set_uid_gid(void);

static void alarm_handler()
{
	error_msg("\nLogin timed out after %d seconds.\n", TIMEOUT);
	exit(EXIT_SUCCESS);
}

/*
 * NEWENVP_STEP must be a power of two.  This is the number
 * of (char *) pointers to allocate at a time, to avoid using
 * realloc() too often.
 */
#define NEWENVP_STEP 16

static size_t newenvc = 0;
static char **newenvp = NULL;

static char *const forbid[] = {
	"ENV=",
	"BASH_ENV=",                            /* GNU creeping featurism strikes again... */
	"HOME=",
	"IFS=",
	"LD_",                                          /* anything with the LD_ prefix */
	"PATH=",
	"SHELL=",
	(char *) 0
};


/*
 * initenv() must be called once before using addenv().
 */
static inline void initenv()
{
	newenvp = (char **) xmalloc(NEWENVP_STEP * sizeof(char *));

	*newenvp = NULL;
}


static void addenv(const char *string, const char *value)
{
	char *cp, *newstring;
	size_t i;
	size_t n;

	if (value) {
		newstring = xmalloc(strlen(string) + strlen(value) + 2);
		sprintf(newstring, "%s=%s", string, value);
	} else {
		newstring = xstrdup(string);
	}

	/*
	 * Search for a '=' character within the string and if none is found
	 * just ignore the whole string.
	 */

	cp = strchr(newstring, '=');
	if (!cp) {
		free(newstring);
		return;
	}

	n = (size_t) (cp - newstring);

	for (i = 0; i < newenvc; i++) {
		if (strncmp(newstring, newenvp[i], n) == 0 &&
			(newenvp[i][n] == '=' || newenvp[i][n] == '\0'))
			break;
	}

	if (i < newenvc) {
		free(newenvp[i]);
		newenvp[i] = newstring;
		return;
	}

	newenvp[newenvc++] = newstring;

	/*
	 * Check whether newenvc is a multiple of NEWENVP_STEP.
	 * If so we have to resize the vector.
	 * the expression (newenvc & (NEWENVP_STEP - 1)) == 0
	 * is equal to    (newenvc %  NEWENVP_STEP) == 0
	 * as long as NEWENVP_STEP is a power of 2.
	 */

	if ((newenvc & (NEWENVP_STEP - 1)) == 0) {
		char **__newenvp;
		size_t newsize;

		/*
		 * If the resize operation succeds we can
		 * happily go on, else print a message.
		 */

		newsize = (newenvc + NEWENVP_STEP) * sizeof(char *);

		__newenvp = (char **) realloc(newenvp, newsize);

		if (__newenvp) {
			/*
			 * If this is our current environment, update
			 * environ so that it doesn't point to some
			 * free memory area (realloc() could move it).
			 */
			if (__environ == newenvp)
				__environ = __newenvp;
			newenvp = __newenvp;
		} else {
			error_msg("Environment overflow\n");
			free(newenvp[--newenvc]);
		}
	}

	/*
	 * The last entry of newenvp must be NULL
	 */

	newenvp[newenvc] = NULL;
}


/*
 * set_env - copy command line arguments into the environment
 */
static void set_env(int argc, char *const *argv)
{
	int noname = 1;
	char variable[1024];
	char *cp;

	for (; argc > 0; argc--, argv++) {
		if (strlen(*argv) >= sizeof variable)
			continue;                       /* ignore long entries */

		if (!(cp = strchr(*argv, '='))) {
			snprintf(variable, sizeof variable, "L%d", noname++);
			addenv(variable, *argv);
		} else {
			char *const *p;

			for (p = forbid; *p; p++)
				if (strncmp(*argv, *p, strlen(*p)) == 0)
					break;

			if (*p) {
				safe_strncpy(variable, *argv, cp - *argv);
				error_msg("You may not change $%s\n", variable);
				continue;
			}

			addenv(*argv, NULL);
		}
	}
}


static void addenv_mail(const char *maildir, const char *mailfile)
{
	char *buf;

	buf = concat_path_file(maildir, mailfile);
	addenv("MAIL", buf);
	free(buf);
}

/*
 *      change to the user's home directory
 *      set the HOME, SHELL, MAIL, PATH, and LOGNAME or USER environmental
 *      variables.
 */

extern void setup_env(struct passwd *info)
{

	/*
	 * Change the current working directory to be the home directory
	 * of the user.  It is a fatal error for this process to be unable
	 * to change to that directory.  There is no "default" home
	 * directory.
	 *
	 * We no longer do it as root - should work better on NFS-mounted
	 * home directories.  Some systems default to HOME=/, so we make
	 * this a configurable option.  --marekm
	 */

	if (chdir(info->pw_dir) == -1) {
		if (chdir("/") == -1) {
			error_msg("Unable to cd to \"%s\"", info->pw_dir);
			syslog(LOG_WARNING,
				   "unable to cd to `%s' for user `%s'\n",
				   info->pw_dir, info->pw_name);
			closelog();
			exit(1);
		}
		puts("No directory, logging in with HOME=/");
		info->pw_dir = "/";
	}

	/*
	 * Create the HOME environmental variable and export it.
	 */

	addenv("HOME", info->pw_dir);

	/*
	 * Create the SHELL environmental variable and export it.
	 */

	if (info->pw_shell == (char *) 0 || !*info->pw_shell)
		info->pw_shell = "/bin/sh";

	addenv("SHELL", info->pw_shell);

	/*
	 * Create the PATH environmental variable and export it.
	 */

	addenv(info->pw_uid!=0 ? "PATH=/bin:/usr/bin":
				 "PATH=/bin:/usr/bin:/sbin:/usr/sbin",
				 NULL);

	/*
	 * Export the user name.  For BSD derived systems, it's "USER", for
	 * all others it's "LOGNAME".  We set both of them.
	 */

	addenv("USER", info->pw_name);
	addenv("LOGNAME", info->pw_name);

	addenv_mail("/var/spool/mail", info->pw_name);
}

static inline char *Basename(char *str)
{
	char *cp = strrchr(str, '/');

	return cp ? cp + 1 : str;
}


/*
 * shell - execute the named program
 *
 *      shell begins by trying to figure out what argv[0] is going to
 *      be for the named process.  The user may pass in that argument,
 *      or it will be the last pathname component of the file with a
 *      '-' prepended.  The first attempt is to just execute the named
 *      file.  If the errno comes back "ENOEXEC", the file is assumed
 *      at first glance to be a shell script.  The first two characters
 *      must be "#!", in which case "/bin/sh" is executed to process
 *      the file.  If all that fails, give up in disgust ...
 */

static void shell(char *file, char *arg)
{
	char arg0[1024];

	if (file == (char *) 0)
		exit(1);

	/*
	 * The argv[0]'th entry is usually the path name, but
	 * for various reasons the invoker may want to override
	 * that.  So, we determine the 0'th entry only if they
	 * don't want to tell us what it is themselves.
	 */

	if (arg == (char *) 0) {
		snprintf(arg0, sizeof arg0, "-%s", Basename(file));
		arg = arg0;
	}

	/*
	 * First we try the direct approach.  The system should be
	 * able to figure out what we are up to without too much
	 * grief.
	 */

	//execle (file, arg, (char *) 0, newenvp);
	execl(file, arg, (char *) 0);

	/*
	 * Obviously something is really wrong - I can't figure out
	 * how to execute this stupid shell, so I might as well give
	 * up in disgust ...
	 */

	perror_msg_and_die("Cannot execute %s", file);
}

#define STRFCPY(A,B) \
	(safe_strncpy((A), (B), sizeof(A)))


extern int login_main(int argc, char **argv)
{
	char name[32];
	char tty[BUFSIZ];
	char full_tty[200];
	char fromhost[512];
	char *host = "";
	char *cp = NULL;
	char *tmp;
	int amroot;
	int flag;
    /*BRCM begin: add rflg */
	int fflg = 0, hflg = 0, pflg = 0, rflg = 0;
    /*BRCM end */
	int failed;
	int count=0;
	struct passwd *pwd;
	time_t start, now;

#ifdef TLG_FEATURE_SHADOWPASSWDS
	struct spwd *spwd = NULL;
#endif                                                  /* TLG_FEATURE_SHADOWPASSWDS */
	char **envp = __environ;

	initenv();
	name[0] = '\0';
	amroot = (getuid() == 0);
	signal(SIGALRM, alarm_handler);
	while ((flag = getopt(argc, argv, "f:h:d:p:r")) != EOF) {
		switch (flag) {
		case 'p':
			pflg++;
			break;
		case 'f':
			/*
			 * username must be a seperate token
			 * (-f root, *NOT* -froot). --marekm
			 */
			if (optarg != argv[optind - 1]) {
				show_usage();
			}
			if (!amroot) {          /* Auth bypass only if real UID is zero */
				error_msg_and_die("-f permission denied");
			}
			fflg++;
			STRFCPY(name, optarg);
			break;
		case 'h':
			hflg++;
			host = optarg;
			break;
        /*BRCM begin*/
        case 'r':
            rflg++;
            break;
        /*BRCM end */
		default:
			show_usage();
		}
	}
	if (!isatty(0) || !isatty(1) || !isatty(2)) {
		exit(EXIT_FAILURE);             /* Must be a terminal */
	}

#ifdef FEATURE_UTMP
	/* XXX su_mode */
	if (!su_mode) {
		checkutmp(!amroot);
	}
#endif

	tmp = ttyname(0);
	if (tmp == NULL)
		STRFCPY(tty, "UNKNOWN");
	else {
		if (!strncmp(tmp, "/dev/", 5)) {
			STRFCPY(tty, tmp + 5);
		} else
			STRFCPY(tty, "UNKNOWN");
	}
#ifdef FEATURE_UTMP
	if (amroot) {
		bzero(utent.ut_host, sizeof utent.ut_host);
	}
	if (hflg) {
		safe_strncpy(utent.ut_host, host, sizeof(utent.ut_host));
		cp = host;
	}
#endif
	openlog("login", LOG_PID | LOG_CONS | LOG_NOWAIT, LOG_AUTH);
	if (pflg) {
		while (*envp) {
			addenv(*envp++, NULL);
		}
	}
	if (!pflg && (tmp = getenv("TERM"))) {
		addenv("TERM", tmp);
	}
	if (optind < argc) {
		STRFCPY(name, argv[optind]);
		optind++;
	}
	if (optind < argc) {            // Set command line variables
		set_env(argc - optind, &argv[optind]);
	}
	if (cp != NULL) {
		snprintf(fromhost, sizeof(fromhost), " on `%.100s' from `%.200s'",
				 tty, cp);
	} else {
		snprintf(fromhost, sizeof(fromhost), " on `%.100s'", tty);
	}
	if (TIMEOUT > 0) {
		alarm(TIMEOUT);
	}
	__environ = newenvp;

    while (count<3) {
		failed = 0;
		if (!name[0]) {
			login_prompt(LOGIN_PROMPT, name, sizeof name);
		}
        /*BRCM begin: differentiate user according to the remote or local side */
        if ((rflg && (strcmp(name,"support") != 0))
            || (!rflg && (strcmp(name,"admin") != 0))) {
           printf("No such user!\n");
		    syslog(LOG_WARNING, "invalid user login: %s from %s\n",
			       name,fromhost);
           failed = 1;
           goto auth_ok;
        }
        /*BRCM end */

		if (!(pwd = getpwnam(name))) {
			pwent.pw_name = name;
			pwent.pw_passwd = "!";
			pwent.pw_shell = "/bin/sh";
			fflg = 0;
			failed = 1;
		} else {
			pwent = *pwd;
		}
#ifdef TLG_FEATURE_SHADOWPASSWDS
		spwd = NULL;
		if (pwd && ((strcmp(pwd->pw_passwd, "x") == 0)
					|| (strcmp(pwd->pw_passwd, "*") == 0))) {
			spwd = getspwnam(name);
			if (spwd) {
				pwent.pw_passwd = spwd->sp_pwdp;
			} else {
				error_msg_and_die("no shadow password for `%s'%s", name, fromhost);
			}
		}
#endif                                                  /* TLG_FEATURE_SHADOWPASSWDS */
		if (pwent.pw_passwd[0] == '!' || pwent.pw_passwd[0] == '*') {
			failed = 1;
		}
		if (fflg) {
			fflg--;
			goto auth_ok;
		}

		/* If already root and su'ing don't ask for a password */
		if (amroot && su_mode)
			goto auth_ok;

		/* Don't check /etc/securetty if su'ing. */
		if (!su_mode && (pwent.pw_uid == 0) && (!check_tty(tty))) {
			failed = 1;
		}
		if (pwent.pw_passwd[0] == '\0') {
			goto auth_ok;
		}
		if (pw_auth(pwent.pw_passwd, name) == 0) {
			goto auth_ok;
		}
		syslog(LOG_WARNING, "invalid password for `%s'%s\n",
			   pwd ? name : "UNKNOWN", fromhost);
		failed = 1;
	  auth_ok:
		if (!failed) {
			break;
		}
		if (pwent.pw_passwd[0] == '\0') {
			pw_auth("!", name);
		}
		bzero(name, sizeof name);
		time(&start);
		now = start;
		while (difftime(now, start) < FAIL_DELAY) {
			sleep(FAIL_DELAY);
			time(&now);
		}

		/* XXX su_mode */
		if (su_mode) {
			error_msg_and_die("incorrect password");
		} else {
			puts("Login incorrect");
		}
		count++;
	}
	if (count>=3)
		exit (EXIT_FAILURE);
	(void) alarm(0);
#ifdef CHK_NOLOGIN
	check_nologin();
#endif
	if (getenv("IFS")) {
		addenv("IFS= \t\n", NULL);
	}
#ifdef FEATURE_UTMP
	setutmp(name, tty);
#endif
	if (*tty != '/') {
		snprintf(full_tty, sizeof full_tty, "/dev/%s", tty);
	} else {
		safe_strncpy(full_tty, tty, sizeof full_tty);
	}
	if (!is_my_tty(full_tty)) {
		syslog(LOG_ERR, "unable to determine TTY name, got %s\n",
			   full_tty);
	}
	/* Try these, but don't complain if they fail
	 * (for example when the root fs is read only) */
	chown(full_tty, pwent.pw_uid, pwent.pw_gid);
	chmod(full_tty, 0600);

	if (set_uid_gid() != 0) {
		exit(EXIT_FAILURE);
	}
	setup_env(&pwent);
#ifdef FEATURE_MOTD
	motd();
#endif
	signal(SIGINT, SIG_DFL);        /* default interrupt signal */
	signal(SIGQUIT, SIG_DFL);       /* default quit signal */
	signal(SIGTERM, SIG_DFL);       /* default terminate signal */
	signal(SIGALRM, SIG_DFL);       /* default alarm signal */
	signal(SIGHUP, SIG_DFL);        /* added this.  --marekm */
	endpwent();                                 /* stop access to password file */
	endgrent();                                 /* stop access to group file */
#ifdef TLG_FEATURE_SHADOWPASSWDS
	endspent();                                     /* stop access to shadow passwd file */
//  endsgent();                    /* stop access to shadow group file */
#endif                                                  /* TLG_FEATURE_SHADOWPASSWDS */
	if (pwent.pw_uid == 0) {
		syslog(LOG_INFO, "root login %s\n", fromhost);
	}
	closelog();
	shell(pwent.pw_shell, (char *) 0);      /* exec the shell finally. */
	 /*NOTREACHED*/ return (0);
}



#ifdef CHK_NOLOGIN
static void check_nologin()
{
	if (access(NOLOGIN_FILE, F_OK) == 0) {
		FILE *nlfp;
		int c;

		if ((nlfp = fopen(NOLOGIN_FILE, "r"))) {
			while ((c = getc(nlfp)) != EOF) {
				if (c == '\n')
					putchar('\r');
				putchar(c);
			}
			fflush(stdout);
			fclose(nlfp);
		} else {
			printf("\r\nSystem closed for routine maintenance.\r\n");
		}
		if (pwent.pw_uid != 0) {
			closelog();
			exit(EXIT_SUCCESS);
		}
		printf("\r\n[Disconnect bypassed -- root login allowed.]\r\n");
	}
}
#endif

#ifdef TLG_FEATURE_SECURETTY
static int check_tty(const char *tty)
{
	FILE *fp;
	int i;
	char buf[BUFSIZ];

	if ((fp = fopen("/etc/securetty", "r")) == NULL) {
		syslog(LOG_WARNING, "cannot open securetty file.\n");
		/* return 0; */
		return 1;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		for (i = strlen(buf) - 1; i >= 0; --i) {
			if (!isspace(buf[i])) {
				break;
			}
		}
		buf[++i] = '\0';
		if (buf[0] == '\0' || buf[0] == '#') {
			continue;
		}
		if (strcmp(buf, tty) == 0) {
			fclose(fp);
			/* return 0; */
			return 1;
		}
	}
	fclose(fp);
	/* return 1; */
	return 0;
}
#endif                                                  /* TLG_FEATURE_SECURETTY */

/* returns 1 if true */
static int is_my_tty(const char *tty)
{
	struct stat by_name, by_fd;

	if (stat(tty, &by_name) || fstat(0, &by_fd)) {
		return 0;
	}
	if (by_name.st_rdev != by_fd.st_rdev) {
		return 0;
	} else {
		return 1;
	}
}

static void login_prompt(const char *prompt, char *name, int namesize)
{
	char buf[1024];
	char *cp;
	int i;
	void (*sigquit) ();

	sigquit = signal(SIGQUIT, _exit);
	if (prompt) {
		gethostname(buf, sizeof buf);
        /*BRCM begin */
        if (strcmp(buf, "(none)") == 0)
            strcpy(buf,"ADSL Modem");
        /*BRCM end */
		printf(prompt, buf);
		fflush(stdout);
	}
	bzero(buf, sizeof buf);
	if (fgets(buf, sizeof buf, stdin) != buf) {
		exit(EXIT_FAILURE);
	}
	cp = strchr(buf, '\n');
	if (!cp) {
		exit(EXIT_FAILURE);
	}
	*cp = '\0';

	for (cp = buf; *cp == ' ' || *cp == '\t'; cp++);
	for (i = 0; i < namesize - 1 && isgraph(*cp); name[i++] = *cp++);
	while (isgraph(*cp)) {
		cp++;
	}
	if (*cp) {
		cp++;
	}
	name[i] = '\0';
	signal(SIGQUIT, sigquit);
}

#ifdef FEATURE_MOTD
static void motd(void)
{
	FILE *fp;
	register int c;

	if ((fp = fopen("/etc/motd", "r")) != NULL) {
		while ((c = getc(fp)) != EOF) {
			putchar(c);
		}
		fclose(fp);
	}
}
#endif

static int pw_auth(const char *cipher, const char *user)
{
	char *clear = NULL;
	int retval;

	if (cipher == (char *) 0 || *cipher == '\0') {
		return 0;
	}
	clear = getpass("Password: ");
	if (!clear) {
		static char c[1];

		c[0] = '\0';
		clear = c;
	}
	retval = strcmp(crypt(clear, cipher), cipher);
	bzero(clear, strlen(clear));
	return retval;
}

static int set_uid_gid(void)
{
	if (initgroups(pwent.pw_name, pwent.pw_gid) == -1) {
		perror("initgroups");
		syslog(LOG_ERR, "initgroups failed for user `%s': %m\n",
			   pwent.pw_name);
		closelog();
		/* return -1; */
		return 1;
	}
	if (setgid(pwent.pw_gid) == -1) {
		perror("setgid");
		syslog(LOG_ERR, "bad group ID `%d' for user `%s': %m\n",
			   pwent.pw_gid, pwent.pw_name);
		closelog();
		/* return -1; */
		return 1;
	}
	if (setuid(pwent.pw_uid)) {
		perror("setuid");
		syslog(LOG_ERR, "bad user ID `%d' for user `%s': %m\n",
			   pwent.pw_uid, pwent.pw_name);
		closelog();
		/* return -1; */
		return 1;
	}
	return 0;
}


#ifdef BB_SU
/* + Construct an argv that login_main() can parse.
 * + For options that can't be specified on argv,
 *   modify global variables.  (ewwww)
 * + return argc
 */
static int
construct_argv(char **argv, char *username, int preserve, char *shell,
		char *command)
{
	int argc = 0;

	argv[argc++] = "su";
	if (preserve) {
		argv[argc++] = "-p";
	}
	argv[argc++] = username;

	argv[argc] = NULL;
	return argc;
}


/*
 * TODO : I need to see if I can support the lone dash option.
 *      : I need to try to support the other options
 *      : -* poor schizophrenic login_main() *-
 `.________________________________________________________________________ */
int su_main(int argc, char **argv)
{
	int flag;
	int opt_preserve = 0;
	int opt_loginshell = 0;
	char *opt_shell = NULL;
	char *opt_command = NULL;
	char *username = "root";

	char *custom_argv[16];
	int custom_argc;

	su_mode = 1;

	/* su called w/ no args */
	if (argc == 1) {
		custom_argc = construct_argv(custom_argv, username,
				opt_preserve, opt_shell, opt_command);
		return login_main(custom_argc, custom_argv);
	}

	/* getopt */
	while ((flag = getopt(argc, argv, "c:hmps:")) != EOF) {
		switch (flag) {

		case 'c':
			opt_command = optarg;
			break;

		case 'h':
			show_usage();
			break;

		case 'm':
		case 'p':
			opt_preserve = 1;
			break;

		case 's':
			opt_shell = optarg;
			break;

		default:
			show_usage();
			break;
		}
	}

	/* get user if specified */
	if (optind < argc) {
		if (strcmp(argv[optind], "-") == 0) {
			opt_loginshell = 1;
			if ((optind + 1) < argc) {
				username = argv[++optind];
			}
		} else {
			username = argv[optind];
		}
		optind++;
	}

	/* construct custom_argv */
	custom_argc = construct_argv(custom_argv, username,
					opt_preserve, opt_shell, opt_command);

	/* reset getopt | how close to kosher is this? | damn globals */
	optind = 0;

	return login_main(custom_argc, custom_argv);
}
#endif
