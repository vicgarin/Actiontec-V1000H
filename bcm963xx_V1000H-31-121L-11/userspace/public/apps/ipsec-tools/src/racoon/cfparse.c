/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 3 "./cfparse.y"

/*
 * Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002 and 2003 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/socket.h>

#include <netinet/in.h>
#ifdef HAVE_NETINET6_IPSEC
#  include <netinet6/ipsec.h>
#else
#  include <netinet/ipsec.h>
#endif

#ifdef ENABLE_HYBRID
#include <arpa/inet.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <grp.h>

#include "var.h"
#include "misc.h"
#include "vmbuf.h"
#include "plog.h"
#include "sockmisc.h"
#include "str2val.h"
#include "genlist.h"
#include "debug.h"

#include "admin.h"
#include "privsep.h"
#include "cfparse_proto.h"
#include "cftoken_proto.h"
#include "algorithm.h"
#include "localconf.h"
#include "policy.h"
#include "sainfo.h"
#include "oakley.h"
#include "pfkey.h"
#include "remoteconf.h"
#include "grabmyaddr.h"
#include "isakmp_var.h"
#include "handler.h"
#include "isakmp.h"
#ifdef ENABLE_HYBRID
#include "isakmp_xauth.h"
#include "isakmp_cfg.h"
#endif
#include "ipsec_doi.h"
#include "strnames.h"
#include "gcmalloc.h"
#ifdef HAVE_GSSAPI
#include "gssapi.h"
#endif
#include "vendorid.h"
#include "rsalist.h"

struct proposalspec {
	time_t lifetime;		/* for isakmp/ipsec */
	int lifebyte;			/* for isakmp/ipsec */
	struct secprotospec *spspec;	/* the head is always current spec. */
	struct proposalspec *next;	/* the tail is the most prefered. */
	struct proposalspec *prev;
};

struct secprotospec {
	int prop_no;
	int trns_no;
	int strength;		/* for isakmp/ipsec */
	int encklen;		/* for isakmp/ipsec */
	time_t lifetime;	/* for isakmp */
	int lifebyte;		/* for isakmp */
	int proto_id;		/* for ipsec (isakmp?) */
	int ipsec_level;	/* for ipsec */
	int encmode;		/* for ipsec */
	int vendorid;		/* for isakmp */
	char *gssid;
	struct sockaddr *remote;
	int algclass[MAXALGCLASS];

	struct secprotospec *next;	/* the tail is the most prefiered. */
	struct secprotospec *prev;
	struct proposalspec *back;
};

static int num2dhgroup[] = {
	0,
	OAKLEY_ATTR_GRP_DESC_MODP768,
	OAKLEY_ATTR_GRP_DESC_MODP1024,
	OAKLEY_ATTR_GRP_DESC_EC2N155,
	OAKLEY_ATTR_GRP_DESC_EC2N185,
	OAKLEY_ATTR_GRP_DESC_MODP1536,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	OAKLEY_ATTR_GRP_DESC_MODP2048,
	OAKLEY_ATTR_GRP_DESC_MODP3072,
	OAKLEY_ATTR_GRP_DESC_MODP4096,
	OAKLEY_ATTR_GRP_DESC_MODP6144,
	OAKLEY_ATTR_GRP_DESC_MODP8192
};

static struct remoteconf *cur_rmconf;
static int tmpalgtype[MAXALGCLASS];
static struct sainfo *cur_sainfo;
static int cur_algclass;

static struct proposalspec *newprspec __P((void));
static void insprspec __P((struct proposalspec *, struct proposalspec **));
static struct secprotospec *newspspec __P((void));
static void insspspec __P((struct secprotospec *, struct proposalspec **));
static void adminsock_conf __P((vchar_t *, vchar_t *, vchar_t *, int));

static int set_isakmp_proposal
	__P((struct remoteconf *, struct proposalspec *));
static void clean_tmpalgtype __P((void));
static int expand_isakmpspec __P((int, int, int *,
	int, int, time_t, int, int, int, char *, struct remoteconf *));
static int listen_addr __P((struct sockaddr *addr, int udp_encap));

void freeetypes (struct etypes **etypes);

#if 0
static int fix_lifebyte __P((u_long));
#endif


/* Line 268 of yacc.c  */
#line 240 "z.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PRIVSEP = 258,
     USER = 259,
     GROUP = 260,
     CHROOT = 261,
     PATH = 262,
     PATHTYPE = 263,
     INCLUDE = 264,
     IDENTIFIER = 265,
     VENDORID = 266,
     LOGGING = 267,
     LOGLEV = 268,
     PADDING = 269,
     PAD_RANDOMIZE = 270,
     PAD_RANDOMIZELEN = 271,
     PAD_MAXLEN = 272,
     PAD_STRICT = 273,
     PAD_EXCLTAIL = 274,
     LISTEN = 275,
     X_ISAKMP = 276,
     X_ISAKMP_NATT = 277,
     X_ADMIN = 278,
     STRICT_ADDRESS = 279,
     ADMINSOCK = 280,
     DISABLED = 281,
     MODECFG = 282,
     CFG_NET4 = 283,
     CFG_MASK4 = 284,
     CFG_DNS4 = 285,
     CFG_NBNS4 = 286,
     CFG_AUTH_SOURCE = 287,
     CFG_SYSTEM = 288,
     CFG_RADIUS = 289,
     CFG_PAM = 290,
     CFG_LOCAL = 291,
     CFG_NONE = 292,
     CFG_ACCOUNTING = 293,
     CFG_CONF_SOURCE = 294,
     CFG_MOTD = 295,
     CFG_POOL_SIZE = 296,
     CFG_AUTH_THROTTLE = 297,
     CFG_PFS_GROUP = 298,
     CFG_SAVE_PASSWD = 299,
     RETRY = 300,
     RETRY_COUNTER = 301,
     RETRY_INTERVAL = 302,
     RETRY_PERSEND = 303,
     RETRY_PHASE1 = 304,
     RETRY_PHASE2 = 305,
     NATT_KA = 306,
     ALGORITHM_CLASS = 307,
     ALGORITHMTYPE = 308,
     STRENGTHTYPE = 309,
     SAINFO = 310,
     FROM = 311,
     REMOTE = 312,
     ANONYMOUS = 313,
     INHERIT = 314,
     EXCHANGE_MODE = 315,
     EXCHANGETYPE = 316,
     DOI = 317,
     DOITYPE = 318,
     SITUATION = 319,
     SITUATIONTYPE = 320,
     CERTIFICATE_TYPE = 321,
     CERTTYPE = 322,
     PEERS_CERTFILE = 323,
     CA_TYPE = 324,
     VERIFY_CERT = 325,
     SEND_CERT = 326,
     SEND_CR = 327,
     IDENTIFIERTYPE = 328,
     MY_IDENTIFIER = 329,
     PEERS_IDENTIFIER = 330,
     VERIFY_IDENTIFIER = 331,
     DNSSEC = 332,
     CERT_X509 = 333,
     CERT_PLAINRSA = 334,
     NONCE_SIZE = 335,
     DH_GROUP = 336,
     KEEPALIVE = 337,
     PASSIVE = 338,
     INITIAL_CONTACT = 339,
     NAT_TRAVERSAL = 340,
     NAT_TRAVERSAL_LEVEL = 341,
     PROPOSAL_CHECK = 342,
     PROPOSAL_CHECK_LEVEL = 343,
     GENERATE_POLICY = 344,
     SUPPORT_PROXY = 345,
     PROPOSAL = 346,
     EXEC_PATH = 347,
     EXEC_COMMAND = 348,
     EXEC_SUCCESS = 349,
     EXEC_FAILURE = 350,
     GSS_ID = 351,
     GSS_ID_ENC = 352,
     GSS_ID_ENCTYPE = 353,
     COMPLEX_BUNDLE = 354,
     DPD = 355,
     DPD_DELAY = 356,
     DPD_RETRY = 357,
     DPD_MAXFAIL = 358,
     XAUTH_LOGIN = 359,
     PREFIX = 360,
     PORT = 361,
     PORTANY = 362,
     UL_PROTO = 363,
     ANY = 364,
     IKE_FRAG = 365,
     ESP_FRAG = 366,
     MODE_CFG = 367,
     PFS_GROUP = 368,
     LIFETIME = 369,
     LIFETYPE_TIME = 370,
     LIFETYPE_BYTE = 371,
     STRENGTH = 372,
     SCRIPT = 373,
     PHASE1_UP = 374,
     PHASE1_DOWN = 375,
     NUMBER = 376,
     SWITCH = 377,
     BOOLEAN = 378,
     HEXSTRING = 379,
     QUOTEDSTRING = 380,
     ADDRSTRING = 381,
     UNITTYPE_BYTE = 382,
     UNITTYPE_KBYTES = 383,
     UNITTYPE_MBYTES = 384,
     UNITTYPE_TBYTES = 385,
     UNITTYPE_SEC = 386,
     UNITTYPE_MIN = 387,
     UNITTYPE_HOUR = 388,
     EOS = 389,
     BOC = 390,
     EOC = 391,
     COMMA = 392
   };
#endif
/* Tokens.  */
#define PRIVSEP 258
#define USER 259
#define GROUP 260
#define CHROOT 261
#define PATH 262
#define PATHTYPE 263
#define INCLUDE 264
#define IDENTIFIER 265
#define VENDORID 266
#define LOGGING 267
#define LOGLEV 268
#define PADDING 269
#define PAD_RANDOMIZE 270
#define PAD_RANDOMIZELEN 271
#define PAD_MAXLEN 272
#define PAD_STRICT 273
#define PAD_EXCLTAIL 274
#define LISTEN 275
#define X_ISAKMP 276
#define X_ISAKMP_NATT 277
#define X_ADMIN 278
#define STRICT_ADDRESS 279
#define ADMINSOCK 280
#define DISABLED 281
#define MODECFG 282
#define CFG_NET4 283
#define CFG_MASK4 284
#define CFG_DNS4 285
#define CFG_NBNS4 286
#define CFG_AUTH_SOURCE 287
#define CFG_SYSTEM 288
#define CFG_RADIUS 289
#define CFG_PAM 290
#define CFG_LOCAL 291
#define CFG_NONE 292
#define CFG_ACCOUNTING 293
#define CFG_CONF_SOURCE 294
#define CFG_MOTD 295
#define CFG_POOL_SIZE 296
#define CFG_AUTH_THROTTLE 297
#define CFG_PFS_GROUP 298
#define CFG_SAVE_PASSWD 299
#define RETRY 300
#define RETRY_COUNTER 301
#define RETRY_INTERVAL 302
#define RETRY_PERSEND 303
#define RETRY_PHASE1 304
#define RETRY_PHASE2 305
#define NATT_KA 306
#define ALGORITHM_CLASS 307
#define ALGORITHMTYPE 308
#define STRENGTHTYPE 309
#define SAINFO 310
#define FROM 311
#define REMOTE 312
#define ANONYMOUS 313
#define INHERIT 314
#define EXCHANGE_MODE 315
#define EXCHANGETYPE 316
#define DOI 317
#define DOITYPE 318
#define SITUATION 319
#define SITUATIONTYPE 320
#define CERTIFICATE_TYPE 321
#define CERTTYPE 322
#define PEERS_CERTFILE 323
#define CA_TYPE 324
#define VERIFY_CERT 325
#define SEND_CERT 326
#define SEND_CR 327
#define IDENTIFIERTYPE 328
#define MY_IDENTIFIER 329
#define PEERS_IDENTIFIER 330
#define VERIFY_IDENTIFIER 331
#define DNSSEC 332
#define CERT_X509 333
#define CERT_PLAINRSA 334
#define NONCE_SIZE 335
#define DH_GROUP 336
#define KEEPALIVE 337
#define PASSIVE 338
#define INITIAL_CONTACT 339
#define NAT_TRAVERSAL 340
#define NAT_TRAVERSAL_LEVEL 341
#define PROPOSAL_CHECK 342
#define PROPOSAL_CHECK_LEVEL 343
#define GENERATE_POLICY 344
#define SUPPORT_PROXY 345
#define PROPOSAL 346
#define EXEC_PATH 347
#define EXEC_COMMAND 348
#define EXEC_SUCCESS 349
#define EXEC_FAILURE 350
#define GSS_ID 351
#define GSS_ID_ENC 352
#define GSS_ID_ENCTYPE 353
#define COMPLEX_BUNDLE 354
#define DPD 355
#define DPD_DELAY 356
#define DPD_RETRY 357
#define DPD_MAXFAIL 358
#define XAUTH_LOGIN 359
#define PREFIX 360
#define PORT 361
#define PORTANY 362
#define UL_PROTO 363
#define ANY 364
#define IKE_FRAG 365
#define ESP_FRAG 366
#define MODE_CFG 367
#define PFS_GROUP 368
#define LIFETIME 369
#define LIFETYPE_TIME 370
#define LIFETYPE_BYTE 371
#define STRENGTH 372
#define SCRIPT 373
#define PHASE1_UP 374
#define PHASE1_DOWN 375
#define NUMBER 376
#define SWITCH 377
#define BOOLEAN 378
#define HEXSTRING 379
#define QUOTEDSTRING 380
#define ADDRSTRING 381
#define UNITTYPE_BYTE 382
#define UNITTYPE_KBYTES 383
#define UNITTYPE_MBYTES 384
#define UNITTYPE_TBYTES 385
#define UNITTYPE_SEC 386
#define UNITTYPE_MIN 387
#define UNITTYPE_HOUR 388
#define EOS 389
#define BOC 390
#define EOC 391
#define COMMA 392




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 171 "./cfparse.y"

	unsigned long num;
	vchar_t *val;
	struct remoteconf *rmconf;
	struct sockaddr *saddr;
	struct sainfoalg *alg;



/* Line 293 of yacc.c  */
#line 560 "z.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 572 "z.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   372

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  138
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  152
/* YYNRULES -- Number of rules.  */
#define YYNRULES  279
/* YYNRULES -- Number of states.  */
#define YYNSTATES  509

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   392

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    21,    23,    25,    27,    29,    31,    33,    38,    39,
      42,    43,    48,    49,    54,    55,    60,    61,    66,    67,
      72,    73,    79,    80,    85,    89,    93,    96,    97,   102,
     103,   108,   112,   114,   116,   121,   122,   125,   126,   131,
     132,   137,   138,   143,   144,   149,   150,   155,   160,   161,
     164,   165,   170,   171,   176,   177,   182,   183,   191,   192,
     197,   198,   203,   204,   208,   211,   212,   214,   219,   220,
     223,   224,   229,   230,   235,   236,   241,   242,   247,   248,
     253,   254,   259,   260,   265,   266,   271,   272,   277,   278,
     283,   284,   289,   290,   295,   296,   301,   302,   307,   308,
     313,   314,   319,   320,   325,   330,   331,   334,   335,   340,
     341,   347,   348,   353,   354,   360,   361,   367,   368,   374,
     375,   376,   385,   387,   390,   396,   399,   400,   404,   405,
     408,   409,   414,   415,   422,   423,   430,   431,   436,   437,
     442,   443,   449,   451,   452,   457,   460,   461,   463,   464,
     466,   468,   470,   472,   474,   475,   477,   478,   485,   486,
     491,   495,   498,   500,   501,   504,   505,   510,   511,   516,
     517,   522,   525,   526,   531,   532,   538,   539,   545,   546,
     552,   553,   558,   559,   564,   565,   570,   571,   576,   577,
     583,   584,   589,   590,   596,   597,   602,   603,   608,   609,
     614,   615,   620,   621,   626,   627,   632,   633,   639,   640,
     646,   647,   652,   653,   658,   659,   664,   665,   670,   671,
     676,   677,   682,   683,   688,   689,   694,   695,   700,   701,
     706,   707,   714,   715,   720,   721,   728,   729,   735,   736,
     739,   740,   746,   747,   752,   754,   756,   757,   759,   761,
     762,   765,   766,   771,   772,   779,   780,   787,   788,   793,
     794,   799,   800,   806,   808,   810,   812,   814,   816,   818
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     139,     0,    -1,    -1,   139,   140,    -1,   141,    -1,   149,
      -1,   153,    -1,   154,    -1,   155,    -1,   159,    -1,   161,
      -1,   169,    -1,   181,    -1,   201,    -1,   210,    -1,   231,
      -1,   151,    -1,     3,   135,   142,   136,    -1,    -1,   142,
     143,    -1,    -1,     4,   125,   144,   134,    -1,    -1,     4,
     121,   145,   134,    -1,    -1,     5,   125,   146,   134,    -1,
      -1,     5,   121,   147,   134,    -1,    -1,     6,   125,   148,
     134,    -1,    -1,     7,     8,   125,   150,   134,    -1,    -1,
      99,   122,   152,   134,    -1,     9,   125,   134,    -1,    97,
      98,   134,    -1,    10,   156,    -1,    -1,    11,   157,   125,
     134,    -1,    -1,    73,   125,   158,   134,    -1,    12,   160,
     134,    -1,   124,    -1,    13,    -1,    14,   135,   162,   136,
      -1,    -1,   162,   163,    -1,    -1,    15,   122,   164,   134,
      -1,    -1,    16,   122,   165,   134,    -1,    -1,    17,   121,
     166,   134,    -1,    -1,    18,   122,   167,   134,    -1,    -1,
      19,   122,   168,   134,    -1,    20,   135,   170,   136,    -1,
      -1,   170,   171,    -1,    -1,    21,   179,   172,   134,    -1,
      -1,    22,   179,   173,   134,    -1,    -1,    23,   174,   106,
     134,    -1,    -1,    25,   125,   125,   125,   121,   175,   134,
      -1,    -1,    25,   125,   176,   134,    -1,    -1,    25,    26,
     177,   134,    -1,    -1,    24,   178,   134,    -1,   126,   180,
      -1,    -1,   106,    -1,    27,   135,   182,   136,    -1,    -1,
     182,   183,    -1,    -1,    28,   126,   184,   134,    -1,    -1,
      29,   126,   185,   134,    -1,    -1,    30,   126,   186,   134,
      -1,    -1,    31,   126,   187,   134,    -1,    -1,    32,    33,
     188,   134,    -1,    -1,    32,    34,   189,   134,    -1,    -1,
      32,    35,   190,   134,    -1,    -1,    38,    37,   191,   134,
      -1,    -1,    38,    34,   192,   134,    -1,    -1,    38,    35,
     193,   134,    -1,    -1,    41,   121,   194,   134,    -1,    -1,
      43,   121,   195,   134,    -1,    -1,    44,   122,   196,   134,
      -1,    -1,    42,   121,   197,   134,    -1,    -1,    39,    36,
     198,   134,    -1,    -1,    39,    34,   199,   134,    -1,    -1,
      40,   125,   200,   134,    -1,    45,   135,   202,   136,    -1,
      -1,   202,   203,    -1,    -1,    46,   121,   204,   134,    -1,
      -1,    47,   121,   288,   205,   134,    -1,    -1,    48,   121,
     206,   134,    -1,    -1,    49,   121,   288,   207,   134,    -1,
      -1,    50,   121,   288,   208,   134,    -1,    -1,    51,   121,
     288,   209,   134,    -1,    -1,    -1,    55,   211,   213,   215,
     135,   216,   212,   136,    -1,    58,    -1,   214,   214,    -1,
      73,   126,   227,   228,   229,    -1,    73,   125,    -1,    -1,
      56,    73,   279,    -1,    -1,   216,   217,    -1,    -1,   113,
     278,   218,   134,    -1,    -1,   114,   115,   121,   288,   219,
     134,    -1,    -1,   114,   116,   121,   289,   220,   134,    -1,
      -1,    52,   221,   224,   134,    -1,    -1,    10,    73,   222,
     134,    -1,    -1,    74,    73,   125,   223,   134,    -1,   226,
      -1,    -1,   226,   225,   137,   224,    -1,    53,   230,    -1,
      -1,   105,    -1,    -1,   106,    -1,   107,    -1,   121,    -1,
     108,    -1,   109,    -1,    -1,   121,    -1,    -1,    57,   235,
      59,   235,   232,   234,    -1,    -1,    57,   235,   233,   234,
      -1,   135,   236,   136,    -1,    58,   180,    -1,   179,    -1,
      -1,   236,   237,    -1,    -1,    60,   238,   274,   134,    -1,
      -1,    62,    63,   239,   134,    -1,    -1,    64,    65,   240,
     134,    -1,    66,   275,    -1,    -1,    68,   125,   241,   134,
      -1,    -1,    69,    78,   125,   242,   134,    -1,    -1,    68,
      78,   125,   243,   134,    -1,    -1,    68,    79,   125,   244,
     134,    -1,    -1,    68,    77,   245,   134,    -1,    -1,    70,
     122,   246,   134,    -1,    -1,    71,   122,   247,   134,    -1,
      -1,    72,   122,   248,   134,    -1,    -1,    74,    73,   279,
     249,   134,    -1,    -1,   104,   279,   250,   134,    -1,    -1,
      75,    73,   279,   251,   134,    -1,    -1,    76,   122,   252,
     134,    -1,    -1,    80,   121,   253,   134,    -1,    -1,    81,
     254,   278,   134,    -1,    -1,    83,   122,   255,   134,    -1,
      -1,   110,   122,   256,   134,    -1,    -1,   111,   121,   257,
     134,    -1,    -1,   118,   125,   119,   258,   134,    -1,    -1,
     118,   125,   120,   259,   134,    -1,    -1,   112,   122,   260,
     134,    -1,    -1,    89,   122,   261,   134,    -1,    -1,    90,
     122,   262,   134,    -1,    -1,    84,   122,   263,   134,    -1,
      -1,    85,   122,   264,   134,    -1,    -1,    85,    86,   265,
     134,    -1,    -1,   100,   122,   266,   134,    -1,    -1,   101,
     121,   267,   134,    -1,    -1,   102,   121,   268,   134,    -1,
      -1,   103,   121,   269,   134,    -1,    -1,   114,   115,   121,
     288,   270,   134,    -1,    -1,    87,    88,   271,   134,    -1,
      -1,   114,   116,   121,   289,   272,   134,    -1,    -1,    91,
     273,   135,   280,   136,    -1,    -1,   274,    61,    -1,    -1,
      78,   125,   125,   276,   134,    -1,    -1,    79,   125,   277,
     134,    -1,    53,    -1,   121,    -1,    -1,   126,    -1,   125,
      -1,    -1,   280,   281,    -1,    -1,   117,   282,    54,   134,
      -1,    -1,   114,   115,   121,   288,   283,   134,    -1,    -1,
     114,   116,   121,   289,   284,   134,    -1,    -1,    81,   278,
     285,   134,    -1,    -1,    96,   125,   286,   134,    -1,    -1,
      52,    53,   230,   287,   134,    -1,   131,    -1,   132,    -1,
     133,    -1,   127,    -1,   128,    -1,   129,    -1,   130,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   250,   250,   252,   255,   256,   257,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   267,   272,   274,   276,
     280,   279,   290,   290,   292,   291,   302,   302,   303,   303,
     309,   308,   328,   328,   333,   347,   359,   363,   362,   368,
     367,   383,   386,   395,   408,   410,   412,   415,   415,   416,
     416,   417,   417,   418,   418,   419,   419,   424,   426,   428,
     432,   431,   437,   436,   446,   445,   451,   450,   460,   459,
     469,   468,   477,   477,   480,   492,   493,   497,   499,   501,
     505,   504,   516,   515,   527,   526,   538,   537,   549,   548,
     558,   557,   571,   570,   584,   583,   593,   592,   606,   605,
     619,   618,   636,   635,   645,   644,   654,   653,   663,   662,
     672,   671,   685,   684,   699,   701,   703,   707,   706,   712,
     711,   717,   716,   722,   721,   727,   726,   732,   731,   745,
     753,   744,   787,   792,   799,   856,   885,   889,   915,   917,
     921,   920,   926,   925,   931,   930,   942,   942,   947,   946,
     952,   951,   959,   964,   963,   970,  1026,  1027,  1030,  1031,
    1032,  1035,  1036,  1037,  1040,  1041,  1047,  1046,  1073,  1072,
    1096,  1167,  1173,  1182,  1184,  1188,  1187,  1192,  1192,  1193,
    1193,  1194,  1196,  1195,  1205,  1204,  1213,  1212,  1220,  1219,
    1243,  1242,  1252,  1252,  1253,  1253,  1254,  1254,  1256,  1255,
    1265,  1264,  1280,  1279,  1296,  1296,  1297,  1297,  1299,  1298,
    1304,  1304,  1305,  1305,  1306,  1306,  1313,  1313,  1317,  1317,
    1321,  1321,  1322,  1322,  1323,  1323,  1324,  1324,  1326,  1325,
    1334,  1333,  1342,  1341,  1350,  1349,  1359,  1358,  1368,  1367,
    1377,  1376,  1381,  1381,  1383,  1382,  1397,  1396,  1407,  1409,
    1433,  1432,  1442,  1441,  1463,  1471,  1483,  1484,  1485,  1487,
    1489,  1493,  1492,  1497,  1496,  1502,  1501,  1514,  1513,  1519,
    1518,  1528,  1527,  1625,  1626,  1627,  1630,  1631,  1632,  1633
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PRIVSEP", "USER", "GROUP", "CHROOT",
  "PATH", "PATHTYPE", "INCLUDE", "IDENTIFIER", "VENDORID", "LOGGING",
  "LOGLEV", "PADDING", "PAD_RANDOMIZE", "PAD_RANDOMIZELEN", "PAD_MAXLEN",
  "PAD_STRICT", "PAD_EXCLTAIL", "LISTEN", "X_ISAKMP", "X_ISAKMP_NATT",
  "X_ADMIN", "STRICT_ADDRESS", "ADMINSOCK", "DISABLED", "MODECFG",
  "CFG_NET4", "CFG_MASK4", "CFG_DNS4", "CFG_NBNS4", "CFG_AUTH_SOURCE",
  "CFG_SYSTEM", "CFG_RADIUS", "CFG_PAM", "CFG_LOCAL", "CFG_NONE",
  "CFG_ACCOUNTING", "CFG_CONF_SOURCE", "CFG_MOTD", "CFG_POOL_SIZE",
  "CFG_AUTH_THROTTLE", "CFG_PFS_GROUP", "CFG_SAVE_PASSWD", "RETRY",
  "RETRY_COUNTER", "RETRY_INTERVAL", "RETRY_PERSEND", "RETRY_PHASE1",
  "RETRY_PHASE2", "NATT_KA", "ALGORITHM_CLASS", "ALGORITHMTYPE",
  "STRENGTHTYPE", "SAINFO", "FROM", "REMOTE", "ANONYMOUS", "INHERIT",
  "EXCHANGE_MODE", "EXCHANGETYPE", "DOI", "DOITYPE", "SITUATION",
  "SITUATIONTYPE", "CERTIFICATE_TYPE", "CERTTYPE", "PEERS_CERTFILE",
  "CA_TYPE", "VERIFY_CERT", "SEND_CERT", "SEND_CR", "IDENTIFIERTYPE",
  "MY_IDENTIFIER", "PEERS_IDENTIFIER", "VERIFY_IDENTIFIER", "DNSSEC",
  "CERT_X509", "CERT_PLAINRSA", "NONCE_SIZE", "DH_GROUP", "KEEPALIVE",
  "PASSIVE", "INITIAL_CONTACT", "NAT_TRAVERSAL", "NAT_TRAVERSAL_LEVEL",
  "PROPOSAL_CHECK", "PROPOSAL_CHECK_LEVEL", "GENERATE_POLICY",
  "SUPPORT_PROXY", "PROPOSAL", "EXEC_PATH", "EXEC_COMMAND", "EXEC_SUCCESS",
  "EXEC_FAILURE", "GSS_ID", "GSS_ID_ENC", "GSS_ID_ENCTYPE",
  "COMPLEX_BUNDLE", "DPD", "DPD_DELAY", "DPD_RETRY", "DPD_MAXFAIL",
  "XAUTH_LOGIN", "PREFIX", "PORT", "PORTANY", "UL_PROTO", "ANY",
  "IKE_FRAG", "ESP_FRAG", "MODE_CFG", "PFS_GROUP", "LIFETIME",
  "LIFETYPE_TIME", "LIFETYPE_BYTE", "STRENGTH", "SCRIPT", "PHASE1_UP",
  "PHASE1_DOWN", "NUMBER", "SWITCH", "BOOLEAN", "HEXSTRING",
  "QUOTEDSTRING", "ADDRSTRING", "UNITTYPE_BYTE", "UNITTYPE_KBYTES",
  "UNITTYPE_MBYTES", "UNITTYPE_TBYTES", "UNITTYPE_SEC", "UNITTYPE_MIN",
  "UNITTYPE_HOUR", "EOS", "BOC", "EOC", "COMMA", "$accept", "statements",
  "statement", "privsep_statement", "privsep_stmts", "privsep_stmt", "$@1",
  "$@2", "$@3", "$@4", "$@5", "path_statement", "$@6", "special_statement",
  "$@7", "include_statement", "gssenc_statement", "identifier_statement",
  "identifier_stmt", "$@8", "$@9", "logging_statement", "log_level",
  "padding_statement", "padding_stmts", "padding_stmt", "$@10", "$@11",
  "$@12", "$@13", "$@14", "listen_statement", "listen_stmts",
  "listen_stmt", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20", "$@21",
  "ike_addrinfo_port", "ike_port", "modecfg_statement", "modecfg_stmts",
  "modecfg_stmt", "$@22", "$@23", "$@24", "$@25", "$@26", "$@27", "$@28",
  "$@29", "$@30", "$@31", "$@32", "$@33", "$@34", "$@35", "$@36", "$@37",
  "$@38", "timer_statement", "timer_stmts", "timer_stmt", "$@39", "$@40",
  "$@41", "$@42", "$@43", "$@44", "sainfo_statement", "$@45", "$@46",
  "sainfo_name", "sainfo_id", "sainfo_peer", "sainfo_specs", "sainfo_spec",
  "$@47", "$@48", "$@49", "$@50", "$@51", "$@52", "algorithms", "$@53",
  "algorithm", "prefix", "port", "ul_proto", "keylength",
  "remote_statement", "$@54", "$@55", "remote_specs_block", "remote_index",
  "remote_specs", "remote_spec", "$@56", "$@57", "$@58", "$@59", "$@60",
  "$@61", "$@62", "$@63", "$@64", "$@65", "$@66", "$@67", "$@68", "$@69",
  "$@70", "$@71", "$@72", "$@73", "$@74", "$@75", "$@76", "$@77", "$@78",
  "$@79", "$@80", "$@81", "$@82", "$@83", "$@84", "$@85", "$@86", "$@87",
  "$@88", "$@89", "$@90", "$@91", "exchange_types", "cert_spec", "$@92",
  "$@93", "dh_group_num", "identifierstring", "isakmpproposal_specs",
  "isakmpproposal_spec", "$@94", "$@95", "$@96", "$@97", "$@98", "$@99",
  "unittype_time", "unittype_byte", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   138,   139,   139,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   141,   142,   142,
     144,   143,   145,   143,   146,   143,   147,   143,   148,   143,
     150,   149,   152,   151,   153,   154,   155,   157,   156,   158,
     156,   159,   160,   160,   161,   162,   162,   164,   163,   165,
     163,   166,   163,   167,   163,   168,   163,   169,   170,   170,
     172,   171,   173,   171,   174,   171,   175,   171,   176,   171,
     177,   171,   178,   171,   179,   180,   180,   181,   182,   182,
     184,   183,   185,   183,   186,   183,   187,   183,   188,   183,
     189,   183,   190,   183,   191,   183,   192,   183,   193,   183,
     194,   183,   195,   183,   196,   183,   197,   183,   198,   183,
     199,   183,   200,   183,   201,   202,   202,   204,   203,   205,
     203,   206,   203,   207,   203,   208,   203,   209,   203,   211,
     212,   210,   213,   213,   214,   214,   215,   215,   216,   216,
     218,   217,   219,   217,   220,   217,   221,   217,   222,   217,
     223,   217,   224,   225,   224,   226,   227,   227,   228,   228,
     228,   229,   229,   229,   230,   230,   232,   231,   233,   231,
     234,   235,   235,   236,   236,   238,   237,   239,   237,   240,
     237,   237,   241,   237,   242,   237,   243,   237,   244,   237,
     245,   237,   246,   237,   247,   237,   248,   237,   249,   237,
     250,   237,   251,   237,   252,   237,   253,   237,   254,   237,
     255,   237,   256,   237,   257,   237,   258,   237,   259,   237,
     260,   237,   261,   237,   262,   237,   263,   237,   264,   237,
     265,   237,   266,   237,   267,   237,   268,   237,   269,   237,
     270,   237,   271,   237,   272,   237,   273,   237,   274,   274,
     276,   275,   277,   275,   278,   278,   279,   279,   279,   280,
     280,   282,   281,   283,   281,   284,   281,   285,   281,   286,
     281,   287,   281,   288,   288,   288,   289,   289,   289,   289
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     4,     0,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     5,     0,     4,     3,     3,     2,     0,     4,     0,
       4,     3,     1,     1,     4,     0,     2,     0,     4,     0,
       4,     0,     4,     0,     4,     0,     4,     4,     0,     2,
       0,     4,     0,     4,     0,     4,     0,     7,     0,     4,
       0,     4,     0,     3,     2,     0,     1,     4,     0,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     4,     0,     2,     0,     4,     0,
       5,     0,     4,     0,     5,     0,     5,     0,     5,     0,
       0,     8,     1,     2,     5,     2,     0,     3,     0,     2,
       0,     4,     0,     6,     0,     6,     0,     4,     0,     4,
       0,     5,     1,     0,     4,     2,     0,     1,     0,     1,
       1,     1,     1,     1,     0,     1,     0,     6,     0,     4,
       3,     2,     1,     0,     2,     0,     4,     0,     4,     0,
       4,     2,     0,     4,     0,     5,     0,     5,     0,     5,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     5,
       0,     4,     0,     5,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     5,     0,     5,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     6,     0,     4,     0,     6,     0,     5,     0,     2,
       0,     5,     0,     4,     1,     1,     0,     1,     1,     0,
       2,     0,     4,     0,     6,     0,     6,     0,     4,     0,
       4,     0,     5,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   129,     0,     0,     0,     3,     4,     5,    16,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      18,     0,     0,    37,     0,    36,    43,    42,     0,    45,
      58,    78,   115,     0,    75,    75,   172,   168,     0,    32,
       0,    30,    34,     0,    39,    41,     0,     0,     0,     0,
     132,     0,   136,     0,    76,   171,    74,     0,     0,    35,
       0,     0,     0,     0,    17,    19,     0,     0,     0,     0,
       0,     0,     0,     0,    44,    46,     0,     0,    64,    72,
       0,    57,    59,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    77,    79,     0,     0,     0,
       0,     0,     0,   114,   116,   135,   156,     0,     0,   133,
     166,   173,   169,    33,    22,    20,    26,    24,    28,    31,
      38,    40,    47,    49,    51,    53,    55,    60,    62,     0,
       0,    70,    68,    80,    82,    84,    86,    88,    90,    92,
      96,    98,    94,   110,   108,   112,   100,   106,   102,   104,
     117,     0,   121,     0,     0,     0,   157,   158,   256,   138,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    73,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   273,   274,   275,
     119,     0,   123,   125,   127,   159,   160,     0,   258,   257,
     137,   130,   167,   175,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   208,     0,     0,     0,
       0,     0,     0,   246,     0,     0,     0,     0,   256,     0,
       0,     0,     0,     0,   170,   174,    23,    21,    27,    25,
      29,    48,    50,    52,    54,    56,    61,    63,    65,    71,
       0,    69,    81,    83,    85,    87,    89,    91,    93,    97,
      99,    95,   111,   109,   113,   101,   107,   103,   105,   118,
       0,   122,     0,     0,     0,   162,   163,   161,   134,     0,
     146,     0,     0,     0,     0,   139,   248,   177,   179,     0,
       0,   181,   190,     0,     0,   182,     0,   192,   194,   196,
     256,   256,   204,   206,     0,   210,   226,   230,   228,   242,
     222,   224,     0,   232,   234,   236,   238,   200,   212,   214,
     220,     0,     0,     0,    66,   120,   124,   126,   128,   148,
       0,     0,   254,   255,   140,     0,     0,   131,     0,     0,
       0,     0,   252,     0,   186,   188,     0,   184,     0,     0,
       0,   198,   202,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   259,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   216,   218,     0,     0,   164,     0,
     152,   150,     0,     0,     0,   249,   176,   178,   180,   250,
       0,   191,     0,     0,   183,     0,   193,   195,   197,     0,
       0,   205,   207,   209,   211,   227,   231,   229,   243,   223,
     225,     0,   233,   235,   237,   239,   201,   213,   215,   221,
     240,   276,   277,   278,   279,   244,     0,     0,    67,   149,
     165,   155,   147,     0,     0,   141,   142,   144,     0,   253,
     187,   189,   185,   199,   203,     0,     0,     0,     0,   261,
     247,   260,     0,     0,   217,   219,     0,   151,     0,     0,
     251,   164,   267,   269,     0,     0,     0,   241,   245,   154,
     143,   145,   271,     0,     0,     0,     0,     0,     0,   268,
     270,   263,   265,   262,   272,     0,     0,   264,   266
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    16,    17,    50,    75,   173,   172,   175,   174,
     176,    18,    76,    19,    70,    20,    21,    22,    35,    53,
      78,    23,    38,    24,    56,    85,   177,   178,   179,   180,
     181,    25,    57,    92,   182,   183,   139,   396,   188,   186,
     140,    46,    65,    26,    58,   106,   189,   190,   191,   192,
     193,   194,   195,   198,   196,   197,   202,   204,   205,   203,
     200,   199,   201,    27,    59,   114,   206,   290,   211,   292,
     293,   294,    28,    43,   304,    62,    63,   118,   221,   305,
     402,   478,   479,   350,   397,   454,   399,   453,   400,   167,
     217,   298,   451,    29,   170,    68,   122,    47,   171,   255,
     306,   359,   360,   366,   415,   412,   413,   363,   368,   369,
     370,   419,   388,   420,   373,   374,   324,   376,   389,   390,
     446,   447,   391,   381,   382,   377,   379,   378,   384,   385,
     386,   387,   472,   380,   473,   332,   358,   311,   458,   410,
     354,   220,   431,   471,   486,   505,   506,   493,   494,   498,
     210,   445
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -385
static const yytype_int16 yypact[] =
{
    -385,    40,  -385,   -96,    60,   -72,    18,     5,   -62,   -48,
     -39,   -35,  -385,   -28,     4,   -32,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,    16,   -17,  -385,    32,  -385,  -385,  -385,   -15,  -385,
    -385,  -385,  -385,   -16,    27,    27,  -385,    84,    26,  -385,
       0,  -385,  -385,    39,  -385,  -385,    -8,    -9,    -6,    15,
    -385,   -18,   110,    97,  -385,  -385,  -385,   -28,    36,  -385,
      38,   -77,   -70,    48,  -385,  -385,    41,    42,    43,    61,
      62,    53,    63,    64,  -385,  -385,    65,    65,  -385,  -385,
      -7,  -385,  -385,    67,    68,    71,    72,    45,    35,    52,
      70,    66,    79,    80,    77,  -385,  -385,    81,    82,    83,
      85,    86,    87,  -385,  -385,  -385,   100,   136,    75,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,   105,
      88,  -385,    90,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,   -49,  -385,   -49,   -49,   -49,  -385,     3,   -12,  -385,
      36,    78,    89,    91,    92,    93,    94,    96,    98,    99,
     101,   102,   103,   104,   107,  -385,   108,    95,   109,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,  -385,  -385,  -385,
    -385,   129,  -385,  -385,  -385,  -385,  -385,   -63,  -385,  -385,
    -385,     7,  -385,  -385,   149,   148,    37,    -3,   138,   142,
     143,   144,   145,   146,   147,   150,  -385,   151,   152,   -45,
     133,   153,   154,  -385,   155,   157,   158,   159,   -12,   160,
     162,   163,     8,   106,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
     165,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
     134,  -385,   156,   161,   164,  -385,  -385,  -385,  -385,   166,
    -385,   171,   -22,    10,   131,  -385,  -385,  -385,  -385,   167,
     168,  -385,  -385,   169,   172,  -385,   174,  -385,  -385,  -385,
     -12,   -12,  -385,  -385,   -22,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,   135,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,   170,   175,    12,  -385,  -385,  -385,  -385,  -385,  -385,
     181,   176,  -385,  -385,  -385,   179,   182,  -385,   -33,   173,
     177,   180,  -385,   178,  -385,  -385,   183,  -385,   184,   185,
     186,  -385,  -385,   187,   188,   189,   190,   191,   192,   193,
     194,   195,   196,  -385,   197,   199,   200,   201,   202,   203,
     204,   205,   -49,   -24,  -385,  -385,   206,   207,   221,   209,
     208,  -385,   210,   -49,   -24,  -385,  -385,  -385,  -385,  -385,
     212,  -385,   213,   214,  -385,   215,  -385,  -385,  -385,   216,
     217,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,   -25,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,   218,   219,  -385,  -385,
    -385,  -385,  -385,   220,   222,  -385,  -385,  -385,   224,  -385,
    -385,  -385,  -385,  -385,  -385,   228,   -22,   229,    19,  -385,
    -385,  -385,   225,   226,  -385,  -385,   181,  -385,   227,   230,
    -385,   221,  -385,  -385,   234,   241,   233,  -385,  -385,  -385,
    -385,  -385,  -385,   231,   232,   -49,   -24,   235,   236,  -385,
    -385,  -385,  -385,  -385,  -385,   237,   238,  -385,  -385
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,    69,   239,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,   243,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -259,  -385,  -385,  -385,
    -385,  -385,  -257,  -385,  -385,  -385,   132,   242,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -321,  -227,  -385,  -385,  -385,  -385,  -385,  -385,  -385,  -385,
    -163,  -384
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -154
static const yytype_int16 yytable[] =
{
     212,   213,   214,   375,    71,    72,    73,    79,    80,    81,
      82,    83,    86,    87,    88,    89,    90,   299,    36,   141,
     457,   337,    93,    94,    95,    96,    97,   465,   405,    33,
      44,   352,    98,    99,   100,   101,   102,   103,   104,    30,
       2,   327,    60,     3,   124,   295,   296,     4,   125,     5,
       6,   126,     7,    32,     8,   127,   466,    61,   297,   300,
       9,   107,   108,   109,   110,   111,   112,    10,    31,   150,
     151,   467,   152,    39,   312,   313,   314,   328,   147,   148,
     149,   301,   207,   208,   209,    11,   153,    40,   154,   468,
      49,    34,   469,   371,   372,    12,    41,    13,    45,   353,
      42,   406,    48,   441,   442,   443,   444,   115,   116,   215,
     216,   470,   502,   218,   219,   309,   310,    52,   142,    55,
     302,   303,   315,   341,   342,   355,   356,    91,    84,    37,
     105,   394,   395,    64,   484,   485,    74,    14,   223,    15,
     224,    51,   225,    67,   226,   482,   227,   228,   229,   230,
     231,   113,   232,   233,   234,   137,   138,    54,   235,   236,
      69,   237,   238,   239,    77,   240,   117,   241,   242,   243,
      61,   121,   123,   128,   134,   129,   130,   131,   244,   245,
     246,   247,   248,   132,   133,   135,   136,   156,   249,   250,
     251,    45,   252,   143,   144,   155,   253,   145,   146,   159,
     157,   158,   160,   161,   162,   166,   163,   164,   165,   168,
     169,   184,   307,   308,   254,   187,   316,   489,   320,   321,
     270,   329,   185,   256,   492,   257,   258,   259,   260,   440,
     261,   343,   262,   263,   398,   264,   265,   266,   267,   349,
     456,   268,   269,   271,   351,   272,   273,   274,   275,   276,
     277,   278,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   291,   317,   318,   319,   357,   345,   322,
     383,   323,     0,   325,   326,   330,   331,   333,   334,   335,
     336,   481,   338,   339,    66,   340,   344,   497,     0,     0,
     346,   392,   361,   362,   364,   347,   393,   365,   348,   367,
     403,   401,   222,   404,     0,   409,   119,   407,     0,   120,
       0,   408,   411,     0,     0,     0,     0,   414,   416,   417,
     418,   421,   422,   423,   424,   425,   426,   427,   428,   429,
     430,   432,   501,   433,   434,   435,   436,   437,   438,   439,
     448,   449,   450,   452,   455,  -153,   459,   460,   461,   462,
     463,   464,   474,   475,   483,   495,   477,   476,   480,   487,
     488,   490,   496,     0,   491,   499,   500,     0,     0,   503,
     504,   507,   508
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-385))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
     163,   164,   165,   324,     4,     5,     6,    15,    16,    17,
      18,    19,    21,    22,    23,    24,    25,    10,    13,    26,
     404,   248,    28,    29,    30,    31,    32,    52,    61,    11,
      58,    53,    38,    39,    40,    41,    42,    43,    44,   135,
       0,    86,    58,     3,   121,   108,   109,     7,   125,     9,
      10,   121,    12,   125,    14,   125,    81,    73,   121,    52,
      20,    46,    47,    48,    49,    50,    51,    27,     8,    34,
      35,    96,    37,   135,    77,    78,    79,   122,    33,    34,
      35,    74,   131,   132,   133,    45,    34,   135,    36,   114,
     122,    73,   117,   320,   321,    55,   135,    57,   126,   121,
     135,   134,    98,   127,   128,   129,   130,   125,   126,   106,
     107,   136,   496,   125,   126,    78,    79,   134,   125,   134,
     113,   114,   125,   115,   116,   115,   116,   136,   136,   124,
     136,   119,   120,   106,   115,   116,   136,    97,    60,    99,
      62,   125,    64,    59,    66,   466,    68,    69,    70,    71,
      72,   136,    74,    75,    76,    86,    87,   125,    80,    81,
     134,    83,    84,    85,   125,    87,    56,    89,    90,    91,
      73,   135,   134,   125,   121,   134,   134,   134,   100,   101,
     102,   103,   104,   122,   122,   122,   122,   121,   110,   111,
     112,   126,   114,   126,   126,   125,   118,   126,   126,   122,
     121,   121,   121,   121,   121,   105,   121,   121,   121,    73,
     135,   106,    63,    65,   136,   125,    78,   476,    73,    73,
     125,    88,   134,   134,   481,   134,   134,   134,   134,   392,
     134,   125,   134,   134,    53,   134,   134,   134,   134,    73,
     403,   134,   134,   134,    73,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   122,   122,   122,   136,   134,   122,
     135,   121,    -1,   122,   122,   122,   122,   122,   121,   121,
     121,    53,   122,   121,    45,   122,   121,    54,    -1,    -1,
     134,   121,   125,   125,   125,   134,   121,   125,   134,   125,
     121,   125,   170,   121,    -1,   125,    63,   134,    -1,    67,
      -1,   134,   134,    -1,    -1,    -1,    -1,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   495,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   121,   134,   134,   137,   134,   134,   134,   134,
     134,   134,   134,   134,   125,   121,   134,   137,   134,   134,
     134,   134,   121,    -1,   134,   134,   134,    -1,    -1,   134,
     134,   134,   134
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   139,     0,     3,     7,     9,    10,    12,    14,    20,
      27,    45,    55,    57,    97,    99,   140,   141,   149,   151,
     153,   154,   155,   159,   161,   169,   181,   201,   210,   231,
     135,     8,   125,    11,    73,   156,    13,   124,   160,   135,
     135,   135,   135,   211,    58,   126,   179,   235,    98,   122,
     142,   125,   134,   157,   125,   134,   162,   170,   182,   202,
      58,    73,   213,   214,   106,   180,   180,    59,   233,   134,
     152,     4,     5,     6,   136,   143,   150,   125,   158,    15,
      16,    17,    18,    19,   136,   163,    21,    22,    23,    24,
      25,   136,   171,    28,    29,    30,    31,    32,    38,    39,
      40,    41,    42,    43,    44,   136,   183,    46,    47,    48,
      49,    50,    51,   136,   203,   125,   126,    56,   215,   214,
     235,   135,   234,   134,   121,   125,   121,   125,   125,   134,
     134,   134,   122,   122,   121,   122,   122,   179,   179,   174,
     178,    26,   125,   126,   126,   126,   126,    33,    34,    35,
      34,    35,    37,    34,    36,   125,   121,   121,   121,   122,
     121,   121,   121,   121,   121,   121,   105,   227,    73,   135,
     232,   236,   145,   144,   147,   146,   148,   164,   165,   166,
     167,   168,   172,   173,   106,   134,   177,   125,   176,   184,
     185,   186,   187,   188,   189,   190,   192,   193,   191,   199,
     198,   200,   194,   197,   195,   196,   204,   131,   132,   133,
     288,   206,   288,   288,   288,   106,   107,   228,   125,   126,
     279,   216,   234,    60,    62,    64,    66,    68,    69,    70,
      71,    72,    74,    75,    76,    80,    81,    83,    84,    85,
      87,    89,    90,    91,   100,   101,   102,   103,   104,   110,
     111,   112,   114,   118,   136,   237,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     125,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     205,   134,   207,   208,   209,   108,   109,   121,   229,    10,
      52,    74,   113,   114,   212,   217,   238,    63,    65,    78,
      79,   275,    77,    78,    79,   125,    78,   122,   122,   122,
      73,    73,   122,   121,   254,   122,   122,    86,   122,    88,
     122,   122,   273,   122,   121,   121,   121,   279,   122,   121,
     122,   115,   116,   125,   121,   134,   134,   134,   134,    73,
     221,    73,    53,   121,   278,   115,   116,   136,   274,   239,
     240,   125,   125,   245,   125,   125,   241,   125,   246,   247,
     248,   279,   279,   252,   253,   278,   255,   263,   265,   264,
     271,   261,   262,   135,   266,   267,   268,   269,   250,   256,
     257,   260,   121,   121,   119,   120,   175,   222,    53,   224,
     226,   125,   218,   121,   121,    61,   134,   134,   134,   125,
     277,   134,   243,   244,   134,   242,   134,   134,   134,   249,
     251,   134,   134,   134,   134,   134,   134,   134,   134,   134,
     134,   280,   134,   134,   134,   134,   134,   134,   134,   134,
     288,   127,   128,   129,   130,   289,   258,   259,   134,   134,
     121,   230,   134,   225,   223,   134,   288,   289,   276,   134,
     134,   134,   134,   134,   134,    52,    81,    96,   114,   117,
     136,   281,   270,   272,   134,   134,   137,   134,   219,   220,
     134,    53,   278,   125,   115,   116,   282,   134,   134,   224,
     134,   134,   230,   285,   286,   121,   121,    54,   287,   134,
     134,   288,   289,   134,   134,   283,   284,   134,   134
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 20:

/* Line 1806 of yacc.c  */
#line 280 "./cfparse.y"
    {
			struct passwd *pw;

			if ((pw = getpwnam((yyvsp[(2) - (2)].val)->v)) == NULL) {
				yyerror("unknown user \"%s\"", (yyvsp[(2) - (2)].val)->v);
				return -1;
			}
			lcconf->uid = pw->pw_uid;
		}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 290 "./cfparse.y"
    { lcconf->uid = (yyvsp[(2) - (2)].num); }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 292 "./cfparse.y"
    {
			struct group *gr;

			if ((gr = getgrnam((yyvsp[(2) - (2)].val)->v)) == NULL) {
				yyerror("unknown group \"%s\"", (yyvsp[(2) - (2)].val)->v);
				return -1;
			}
			lcconf->gid = gr->gr_gid;
		}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 302 "./cfparse.y"
    { lcconf->gid = (yyvsp[(2) - (2)].num); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 303 "./cfparse.y"
    { lcconf->chroot = (yyvsp[(2) - (2)].val)->v; }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 309 "./cfparse.y"
    {
			if ((yyvsp[(2) - (3)].num) >= LC_PATHTYPE_MAX) {
				yyerror("invalid path type %d", (yyvsp[(2) - (3)].num));
				return -1;
			}

			/* free old pathinfo */
			if (lcconf->pathinfo[(yyvsp[(2) - (3)].num)])
				racoon_free(lcconf->pathinfo[(yyvsp[(2) - (3)].num)]);

			/* set new pathinfo */
			lcconf->pathinfo[(yyvsp[(2) - (3)].num)] = strdup((yyvsp[(3) - (3)].val)->v);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 328 "./cfparse.y"
    { lcconf->complex_bundle = (yyvsp[(2) - (2)].num); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 334 "./cfparse.y"
    {
			char path[MAXPATHLEN];

			getpathname(path, sizeof(path),
				LC_PATHTYPE_INCLUDE, (yyvsp[(2) - (3)].val)->v);
			vfree((yyvsp[(2) - (3)].val));
			if (yycf_switch_buffer(path) != 0)
				return -1;
		}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 348 "./cfparse.y"
    {
			if ((yyvsp[(2) - (3)].num) >= LC_GSSENC_MAX) {
				yyerror("invalid GSS ID encoding %d", (yyvsp[(2) - (3)].num));
				return -1;
			}
			lcconf->gss_id_enc = (yyvsp[(2) - (3)].num);
		}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 363 "./cfparse.y"
    {
			/*XXX to be deleted */
		}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 368 "./cfparse.y"
    {
			/*XXX to be deleted */
			(yyvsp[(2) - (2)].val)->l--;	/* nuke '\0' */
			lcconf->ident[(yyvsp[(1) - (2)].num)] = (yyvsp[(2) - (2)].val);
			if (lcconf->ident[(yyvsp[(1) - (2)].num)] == NULL) {
				yyerror("failed to set my ident: %s",
					strerror(errno));
				return -1;
			}
		}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 387 "./cfparse.y"
    {
			/*
			 * XXX ignore it because this specification
			 * will be obsoleted.
			 */
			yywarn("see racoon.conf(5), such a log specification will be obsoleted.");
			vfree((yyvsp[(1) - (1)].val));
		}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 396 "./cfparse.y"
    {
			/*
			 * set the loglevel by configuration file only when
			 * the command line did not specify any loglevel.
			 */
			if (loglevel <= LLV_BASE)
				loglevel += (yyvsp[(1) - (1)].num);
		}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 415 "./cfparse.y"
    { lcconf->pad_random = (yyvsp[(2) - (2)].num); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 416 "./cfparse.y"
    { lcconf->pad_randomlen = (yyvsp[(2) - (2)].num); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 417 "./cfparse.y"
    { lcconf->pad_maxsize = (yyvsp[(2) - (2)].num); }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 418 "./cfparse.y"
    { lcconf->pad_strict = (yyvsp[(2) - (2)].num); }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 419 "./cfparse.y"
    { lcconf->pad_excltail = (yyvsp[(2) - (2)].num); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 432 "./cfparse.y"
    {
			listen_addr ((yyvsp[(2) - (2)].saddr), 0);
		}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 437 "./cfparse.y"
    {
#ifdef ENABLE_NATT
			listen_addr ((yyvsp[(2) - (2)].saddr), 1);
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 446 "./cfparse.y"
    {
			yyerror("admin directive is obsoleted.");
		}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 451 "./cfparse.y"
    {
#ifdef ENABLE_ADMINPORT
			adminsock_conf((yyvsp[(2) - (5)].val), (yyvsp[(3) - (5)].val), (yyvsp[(4) - (5)].val), (yyvsp[(5) - (5)].num));
#else
			yywarn("admin port support not compiled in");
#endif
		}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 460 "./cfparse.y"
    {
#ifdef ENABLE_ADMINPORT
			adminsock_conf((yyvsp[(2) - (2)].val), NULL, NULL, -1);
#else
			yywarn("admin port support not compiled in");
#endif
		}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 469 "./cfparse.y"
    {
#ifdef ENABLE_ADMINPORT
			adminsock_path = NULL;
#else
			yywarn("admin port support not compiled in");
#endif
		}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 477 "./cfparse.y"
    { lcconf->strict_address = TRUE; }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 481 "./cfparse.y"
    {
			char portbuf[10];

			snprintf(portbuf, sizeof(portbuf), "%ld", (yyvsp[(2) - (2)].num));
			(yyval.saddr) = str2saddr((yyvsp[(1) - (2)].val)->v, portbuf);
			vfree((yyvsp[(1) - (2)].val));
			if (!(yyval.saddr))
				return -1;
		}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 492 "./cfparse.y"
    { (yyval.num) = PORT_ISAKMP; }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 493 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 505 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
		 if (inet_pton(AF_INET, (yyvsp[(2) - (2)].val)->v,
		     &isakmp_cfg_config.network4) != 1)
			yyerror("bad IPv4 network address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 516 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if (inet_pton(AF_INET, (yyvsp[(2) - (2)].val)->v,
			    &isakmp_cfg_config.netmask4) != 1)
				yyerror("bad IPv4 netmask address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 527 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if (inet_pton(AF_INET, (yyvsp[(2) - (2)].val)->v,
			    &isakmp_cfg_config.dns4) != 1)
				yyerror("bad IPv4 DNS address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 538 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			if (inet_pton(AF_INET, (yyvsp[(2) - (2)].val)->v,
			    &isakmp_cfg_config.nbns4) != 1)
				yyerror("bad IPv4 WINS address.");
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 549 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_SYSTEM;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 558 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBRADIUS
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_RADIUS;
#else /* HAVE_LIBRADIUS */
			yyerror("racoon not configured with --with-libradius");
#endif /* HAVE_LIBRADIUS */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 571 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBPAM
			isakmp_cfg_config.authsource = ISAKMP_CFG_AUTH_PAM;
#else /* HAVE_LIBPAM */
			yyerror("racoon not configured with --with-libpam");
#endif /* HAVE_LIBPAM */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 584 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_NONE;
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 593 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBRADIUS
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_RADIUS;
#else /* HAVE_LIBRADIUS */
			yyerror("racoon not configured with --with-libradius");
#endif /* HAVE_LIBRADIUS */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 606 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBPAM
			isakmp_cfg_config.accounting = ISAKMP_CFG_ACCT_PAM;
#else /* HAVE_LIBPAM */
			yyerror("racoon not configured with --with-libpam");
#endif /* HAVE_LIBPAM */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 619 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			size_t len;

			isakmp_cfg_config.pool_size = (yyvsp[(2) - (2)].num);

			len = (yyvsp[(2) - (2)].num) * sizeof(*isakmp_cfg_config.port_pool);
			isakmp_cfg_config.port_pool = racoon_malloc(len);
			if (isakmp_cfg_config.port_pool == NULL)
				yyerror("cannot allocate memory for pool");
			bzero(isakmp_cfg_config.port_pool, len);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 636 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.pfs_group = (yyvsp[(2) - (2)].num);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 645 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.save_passwd = (yyvsp[(2) - (2)].num);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 654 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.auth_throttle = (yyvsp[(2) - (2)].num);
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 663 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			isakmp_cfg_config.confsource = ISAKMP_CFG_CONF_LOCAL;
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 672 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
#ifdef HAVE_LIBRADIUS
			isakmp_cfg_config.confsource = ISAKMP_CFG_CONF_RADIUS;
#else /* HAVE_LIBRADIUS */
			yyerror("racoon not configured with --with-libradius");
#endif /* HAVE_LIBRADIUS */
#else /* ENABLE_HYBRID */
			yyerror("racoon not configured with --enable-hybrid");
#endif /* ENABLE_HYBRID */
		}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 685 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			strncpy(&isakmp_cfg_config.motd[0], (yyvsp[(2) - (2)].val)->v, MAXPATHLEN);
			isakmp_cfg_config.motd[MAXPATHLEN] = '\0';
			vfree((yyvsp[(2) - (2)].val));
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 707 "./cfparse.y"
    {
			lcconf->retry_counter = (yyvsp[(2) - (2)].num);
		}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 712 "./cfparse.y"
    {
			lcconf->retry_interval = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
		}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 717 "./cfparse.y"
    {
			lcconf->count_persend = (yyvsp[(2) - (2)].num);
		}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 722 "./cfparse.y"
    {
			lcconf->retry_checkph1 = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
		}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 727 "./cfparse.y"
    {
			lcconf->wait_ph2complete = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
		}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 732 "./cfparse.y"
    {
#ifdef ENABLE_NATT
			lcconf->natt_ka_interval = (yyvsp[(2) - (3)].num) * (yyvsp[(3) - (3)].num);
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 745 "./cfparse.y"
    {
			cur_sainfo = newsainfo();
			if (cur_sainfo == NULL) {
				yyerror("failed to allocate sainfo");
				return -1;
			}
		}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 753 "./cfparse.y"
    {
			struct sainfo *check;

			/* default */
			if (cur_sainfo->algs[algclass_ipsec_enc] == 0) {
				yyerror("no encryption algorithm at %s",
					sainfo2str(cur_sainfo));
				return -1;
			}
			if (cur_sainfo->algs[algclass_ipsec_auth] == 0) {
				yyerror("no authentication algorithm at %s",
					sainfo2str(cur_sainfo));
				return -1;
			}
			if (cur_sainfo->algs[algclass_ipsec_comp] == 0) {
				yyerror("no compression algorithm at %s",
					sainfo2str(cur_sainfo));
				return -1;
			}

			/* duplicate check */
			check = getsainfo(cur_sainfo->idsrc,
					  cur_sainfo->iddst,
					  cur_sainfo->id_i);
			if (check && (!check->idsrc && !cur_sainfo->idsrc)) {
				yyerror("duplicated sainfo: %s",
					sainfo2str(cur_sainfo));
				return -1;
			}
			inssainfo(cur_sainfo);
		}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 788 "./cfparse.y"
    {
			cur_sainfo->idsrc = NULL;
			cur_sainfo->iddst = NULL;
		}
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 793 "./cfparse.y"
    {
			cur_sainfo->idsrc = (yyvsp[(1) - (2)].val);
			cur_sainfo->iddst = (yyvsp[(2) - (2)].val);
		}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 800 "./cfparse.y"
    {
			char portbuf[10];
			struct sockaddr *saddr;

			if (((yyvsp[(5) - (5)].num) == IPPROTO_ICMP || (yyvsp[(5) - (5)].num) == IPPROTO_ICMPV6)
			 && ((yyvsp[(4) - (5)].num) != IPSEC_PORT_ANY || (yyvsp[(4) - (5)].num) != IPSEC_PORT_ANY)) {
				yyerror("port number must be \"any\".");
				return -1;
			}

			snprintf(portbuf, sizeof(portbuf), "%lu", (yyvsp[(4) - (5)].num));
			saddr = str2saddr((yyvsp[(2) - (5)].val)->v, portbuf);
			vfree((yyvsp[(2) - (5)].val));
			if (saddr == NULL)
				return -1;

			switch (saddr->sa_family) {
			case AF_INET:
				if ((yyvsp[(5) - (5)].num) == IPPROTO_ICMPV6) {
					yyerror("upper layer protocol mismatched.\n");
					racoon_free(saddr);
					return -1;
				}
				(yyval.val) = ipsecdoi_sockaddr2id(saddr,
#if 0
					(yyvsp[(3) - (5)].num) == (sizeof(struct in_addr) << 3) &&
						(yyvsp[(1) - (5)].num) == IDTYPE_ADDRESS
					  ? ~0 : (yyvsp[(3) - (5)].num),
#else
					(yyvsp[(3) - (5)].num) == ~0 ? (sizeof(struct in_addr) << 3): (yyvsp[(3) - (5)].num),
#endif
					(yyvsp[(5) - (5)].num));
				break;
#ifdef INET6
			case AF_INET6:
				if ((yyvsp[(5) - (5)].num) == IPPROTO_ICMP) {
					yyerror("upper layer protocol mismatched.\n");
					racoon_free(saddr);
					return -1;
				}
				(yyval.val) = ipsecdoi_sockaddr2id(saddr,
					(yyvsp[(3) - (5)].num) == (sizeof(struct in6_addr) << 3) &&
						(yyvsp[(1) - (5)].num) == IDTYPE_ADDRESS
					  ? ~0 : (yyvsp[(3) - (5)].num),
					(yyvsp[(5) - (5)].num));
				break;
#endif
			default:
				yyerror("invalid family: %d", saddr->sa_family);
				(yyval.val) = NULL;
				break;
			}
			racoon_free(saddr);
			if ((yyval.val) == NULL)
				return -1;
		}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 857 "./cfparse.y"
    {
			struct ipsecdoi_id_b *id_b;

			if ((yyvsp[(1) - (2)].num) == IDTYPE_ASN1DN) {
				yyerror("id type forbidden: %d", (yyvsp[(1) - (2)].num));
				(yyval.val) = NULL;
				return -1;
			}

			(yyvsp[(2) - (2)].val)->l--;

			(yyval.val) = vmalloc(sizeof(*id_b) + (yyvsp[(2) - (2)].val)->l);
			if ((yyval.val) == NULL) {
				yyerror("failed to allocate identifier");
				return -1;
			}

			id_b = (struct ipsecdoi_id_b *)(yyval.val)->v;
			id_b->type = idtype2doi((yyvsp[(1) - (2)].num));

			id_b->proto_id = 0;
			id_b->port = 0;

			memcpy((yyval.val)->v + sizeof(*id_b), (yyvsp[(2) - (2)].val)->v, (yyvsp[(2) - (2)].val)->l);
		}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 885 "./cfparse.y"
    {
			cur_sainfo->id_i = NULL;
		}
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 890 "./cfparse.y"
    {
			struct ipsecdoi_id_b *id_b;
			vchar_t *idv;

			if (set_identifier(&idv, (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
			cur_sainfo->id_i = vmalloc(sizeof(*id_b) + idv->l);
			if (cur_sainfo->id_i == NULL) {
				yyerror("failed to allocate identifier");
				return -1;
			}

			id_b = (struct ipsecdoi_id_b *)cur_sainfo->id_i->v;
			id_b->type = idtype2doi((yyvsp[(2) - (3)].num));

			id_b->proto_id = 0;
			id_b->port = 0;

			memcpy(cur_sainfo->id_i->v + sizeof(*id_b),
			       idv->v, idv->l);
			vfree(idv);
		}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 921 "./cfparse.y"
    {
			cur_sainfo->pfs_group = (yyvsp[(2) - (2)].num);
		}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 926 "./cfparse.y"
    {
			cur_sainfo->lifetime = (yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num);
		}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 931 "./cfparse.y"
    {
#if 1
			yyerror("byte lifetime support is deprecated");
			return -1;
#else
			cur_sainfo->lifebyte = fix_lifebyte((yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num));
			if (cur_sainfo->lifebyte == 0)
				return -1;
#endif
		}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 942 "./cfparse.y"
    {
			cur_algclass = (yyvsp[(1) - (1)].num);
		}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 947 "./cfparse.y"
    {
			yyerror("it's deprecated to specify a identifier in phase 2");
		}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 952 "./cfparse.y"
    {
			yyerror("it's deprecated to specify a identifier in phase 2");
		}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 960 "./cfparse.y"
    {
			inssainfoalg(&cur_sainfo->algs[cur_algclass], (yyvsp[(1) - (1)].alg));
		}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 964 "./cfparse.y"
    {
			inssainfoalg(&cur_sainfo->algs[cur_algclass], (yyvsp[(1) - (1)].alg));
		}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 971 "./cfparse.y"
    {
			int defklen;

			(yyval.alg) = newsainfoalg();
			if ((yyval.alg) == NULL) {
				yyerror("failed to get algorithm allocation");
				return -1;
			}

			(yyval.alg)->alg = algtype2doi(cur_algclass, (yyvsp[(1) - (2)].num));
			if ((yyval.alg)->alg == -1) {
				yyerror("algorithm mismatched");
				racoon_free((yyval.alg));
				(yyval.alg) = NULL;
				return -1;
			}

			defklen = default_keylen(cur_algclass, (yyvsp[(1) - (2)].num));
			if (defklen == 0) {
				if ((yyvsp[(2) - (2)].num)) {
					yyerror("keylen not allowed");
					racoon_free((yyval.alg));
					(yyval.alg) = NULL;
					return -1;
				}
			} else {
				if ((yyvsp[(2) - (2)].num) && check_keylen(cur_algclass, (yyvsp[(1) - (2)].num), (yyvsp[(2) - (2)].num)) < 0) {
					yyerror("invalid keylen %d", (yyvsp[(2) - (2)].num));
					racoon_free((yyval.alg));
					(yyval.alg) = NULL;
					return -1;
				}
			}

			if ((yyvsp[(2) - (2)].num))
				(yyval.alg)->encklen = (yyvsp[(2) - (2)].num);
			else
				(yyval.alg)->encklen = defklen;

			/* check if it's supported algorithm by kernel */
			if (!(cur_algclass == algclass_ipsec_auth && (yyvsp[(1) - (2)].num) == algtype_non_auth)
			 && pk_checkalg(cur_algclass, (yyvsp[(1) - (2)].num), (yyval.alg)->encklen)) {
				int a = algclass2doi(cur_algclass);
				int b = algtype2doi(cur_algclass, (yyvsp[(1) - (2)].num));
				if (a == IPSECDOI_ATTR_AUTH)
					a = IPSECDOI_PROTO_IPSEC_AH;
				yyerror("algorithm %s not supported by the kernel (missing module?)",
					s_ipsecdoi_trns(a, b));
				racoon_free((yyval.alg));
				(yyval.alg) = NULL;
				return -1;
			}
		}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 1026 "./cfparse.y"
    { (yyval.num) = ~0; }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 1027 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 1030 "./cfparse.y"
    { (yyval.num) = IPSEC_PORT_ANY; }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 1031 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 1032 "./cfparse.y"
    { (yyval.num) = IPSEC_PORT_ANY; }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 1035 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 1036 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 1037 "./cfparse.y"
    { (yyval.num) = IPSEC_ULPROTO_ANY; }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 1040 "./cfparse.y"
    { (yyval.num) = 0; }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 1041 "./cfparse.y"
    { (yyval.num) = (yyvsp[(1) - (1)].num); }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 1047 "./cfparse.y"
    {
			struct remoteconf *new;
			struct proposalspec *prspec;

			new = copyrmconf((yyvsp[(4) - (4)].saddr));
			if (new == NULL) {
				yyerror("failed to get remoteconf for %s.", saddr2str ((yyvsp[(4) - (4)].saddr)));
				return -1;
			}

			new->remote = (yyvsp[(2) - (4)].saddr);
			new->inherited_from = getrmconf_strict((yyvsp[(4) - (4)].saddr), 1);
			new->proposal = NULL;
			new->prhead = NULL;
			cur_rmconf = new;

			prspec = newprspec();
			if (prspec == NULL || !cur_rmconf->inherited_from 
				|| !cur_rmconf->inherited_from->proposal)
				return -1;
			prspec->lifetime = cur_rmconf->inherited_from->proposal->lifetime;
			prspec->lifebyte = cur_rmconf->inherited_from->proposal->lifebyte;
			insprspec(prspec, &cur_rmconf->prhead);
		}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 1073 "./cfparse.y"
    {
			struct remoteconf *new;
			struct proposalspec *prspec;

			new = newrmconf();
			if (new == NULL) {
				yyerror("failed to get new remoteconf.");
				return -1;
			}

			new->remote = (yyvsp[(2) - (2)].saddr);
			cur_rmconf = new;

			prspec = newprspec();
			if (prspec == NULL)
				return -1;
			prspec->lifetime = oakley_get_defaultlifetime();
			insprspec(prspec, &cur_rmconf->prhead);
		}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 1097 "./cfparse.y"
    {
			/* check a exchange mode */
			if (cur_rmconf->etypes == NULL) {
				yyerror("no exchange mode specified.\n");
				return -1;
			}

			if (cur_rmconf->idvtype == IDTYPE_UNDEFINED)
				cur_rmconf->idvtype = IDTYPE_ADDRESS;


			if (cur_rmconf->idvtype == IDTYPE_ASN1DN) {
				if (cur_rmconf->mycertfile) {
					if (cur_rmconf->idv)
						yywarn("Both CERT and ASN1 ID "
						       "are set. Hope this is OK.\n");
					/* TODO: Preparse the DN here */
				} else if (cur_rmconf->idv) {
					/* OK, using asn1dn without X.509. */
				} else {
					yyerror("ASN1 ID not specified "
						"and no CERT defined!\n");
					return -1;
				}
			}
			
			if (cur_rmconf->prhead->spspec == NULL
				&& cur_rmconf->inherited_from
				&& cur_rmconf->inherited_from->prhead) {
				cur_rmconf->prhead->spspec = cur_rmconf->inherited_from->prhead->spspec;
			}
			if (set_isakmp_proposal(cur_rmconf, cur_rmconf->prhead) != 0)
				return -1;

			/* DH group settting if aggressive mode is there. */
			if (check_etypeok(cur_rmconf, ISAKMP_ETYPE_AGG) != NULL) {
				struct isakmpsa *p;
				int b = 0;

				/* DH group */
				for (p = cur_rmconf->proposal; p; p = p->next) {
					if (b == 0 || (b && b == p->dh_group)) {
						b = p->dh_group;
						continue;
					}
					yyerror("DH group must be equal "
						"in all proposals "
						"when aggressive mode is "
						"used.\n");
					return -1;
				}
				cur_rmconf->dh_group = b;

				if (cur_rmconf->dh_group == 0) {
					yyerror("DH group must be set in the proposal.\n");
					return -1;
				}

				/* DH group settting if PFS is required. */
				if (oakley_setdhgroup(cur_rmconf->dh_group,
						&cur_rmconf->dhgrp) < 0) {
					yyerror("failed to set DH value.\n");
					return -1;
				}
			}

			insrmconf(cur_rmconf);
		}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 1168 "./cfparse.y"
    {
			(yyval.saddr) = newsaddr(sizeof(struct sockaddr));
			(yyval.saddr)->sa_family = AF_UNSPEC;
			((struct sockaddr_in *)(yyval.saddr))->sin_port = htons((yyvsp[(2) - (2)].num));
		}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 1174 "./cfparse.y"
    {
			(yyval.saddr) = (yyvsp[(1) - (1)].saddr);
			if ((yyval.saddr) == NULL) {
				yyerror("failed to allocate sockaddr");
				return -1;
			}
		}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 1188 "./cfparse.y"
    {
			cur_rmconf->etypes = NULL;
		}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 1192 "./cfparse.y"
    { cur_rmconf->doitype = (yyvsp[(2) - (2)].num); }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 1193 "./cfparse.y"
    { cur_rmconf->sittype = (yyvsp[(2) - (2)].num); }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1196 "./cfparse.y"
    {
			yywarn("This directive without certtype will be removed!\n");
			yywarn("Please use 'peers_certfile x509 \"%s\";' instead\n", (yyvsp[(2) - (2)].val)->v);
			cur_rmconf->getcert_method = ISAKMP_GETCERT_LOCALFILE;
			cur_rmconf->peerscertfile = strdup((yyvsp[(2) - (2)].val)->v);
			vfree((yyvsp[(2) - (2)].val));
		}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1205 "./cfparse.y"
    {
			cur_rmconf->cacerttype = (yyvsp[(2) - (3)].num);
			cur_rmconf->getcacert_method = ISAKMP_GETCERT_LOCALFILE;
			cur_rmconf->cacertfile = strdup((yyvsp[(3) - (3)].val)->v);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1213 "./cfparse.y"
    {
			cur_rmconf->getcert_method = ISAKMP_GETCERT_LOCALFILE;
			cur_rmconf->peerscertfile = strdup((yyvsp[(3) - (3)].val)->v);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1220 "./cfparse.y"
    {
			char path[MAXPATHLEN];
			int ret = 0;

			getpathname(path, sizeof(path),
				LC_PATHTYPE_CERT, (yyvsp[(3) - (3)].val)->v);
			vfree((yyvsp[(3) - (3)].val));

			if (cur_rmconf->getcert_method == ISAKMP_GETCERT_DNS) {
				yyerror("Different peers_certfile method "
					"already defined: %d!\n",
					cur_rmconf->getcert_method);
				return -1;
			}
			cur_rmconf->getcert_method = ISAKMP_GETCERT_LOCALFILE;
			if (rsa_parse_file(cur_rmconf->rsa_public, path, RSA_TYPE_PUBLIC)) {
				yyerror("Couldn't parse keyfile.\n", path);
				return -1;
			}
			plog(LLV_DEBUG, LOCATION, NULL, "Public PlainRSA keyfile parsed: %s\n", path);
		}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1243 "./cfparse.y"
    {
			if (cur_rmconf->getcert_method) {
				yyerror("Different peers_certfile method already defined!\n");
				return -1;
			}
			cur_rmconf->getcert_method = ISAKMP_GETCERT_DNS;
			cur_rmconf->peerscertfile = NULL;
		}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1252 "./cfparse.y"
    { cur_rmconf->verify_cert = (yyvsp[(2) - (2)].num); }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1253 "./cfparse.y"
    { cur_rmconf->send_cert = (yyvsp[(2) - (2)].num); }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1254 "./cfparse.y"
    { cur_rmconf->send_cr = (yyvsp[(2) - (2)].num); }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1256 "./cfparse.y"
    {
			if (set_identifier(&cur_rmconf->idv, (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
			cur_rmconf->idvtype = (yyvsp[(2) - (3)].num);
		}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1265 "./cfparse.y"
    {
#ifdef ENABLE_HYBRID
			/* formerly identifier type login */
			cur_rmconf->idvtype = IDTYPE_LOGIN;
			if (set_identifier(&cur_rmconf->idv, IDTYPE_LOGIN, (yyvsp[(2) - (2)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				return -1;
			}
			/* cur_rmconf->use_xauth = 1; */
#else
			yyerror("racoon not configured with --enable-hybrid");
#endif
		}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1280 "./cfparse.y"
    {
			struct idspec  *id;
			id = newidspec();
			if (id == NULL) {
				yyerror("failed to allocate idspec");
				return -1;
			}
			if (set_identifier(&id->id, (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].val)) != 0) {
				yyerror("failed to set identifer.\n");
				racoon_free(id);
				return -1;
			}
			id->idtype = (yyvsp[(2) - (3)].num);
			genlist_append (cur_rmconf->idvl_p, id);
		}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1296 "./cfparse.y"
    { cur_rmconf->verify_identifier = (yyvsp[(2) - (2)].num); }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1297 "./cfparse.y"
    { cur_rmconf->nonce_size = (yyvsp[(2) - (2)].num); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1299 "./cfparse.y"
    {
			yyerror("dh_group cannot be defined here.");
			return -1;
		}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1304 "./cfparse.y"
    { cur_rmconf->passive = (yyvsp[(2) - (2)].num); }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1305 "./cfparse.y"
    { cur_rmconf->ike_frag = (yyvsp[(2) - (2)].num); }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1306 "./cfparse.y"
    { 
#ifdef SADB_X_EXT_NAT_T_FRAG
			cur_rmconf->esp_frag = (yyvsp[(2) - (2)].num); 
#else
			yywarn("Your kernel does not support esp_frag");
#endif
		}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1313 "./cfparse.y"
    { 
			cur_rmconf->script[SCRIPT_PHASE1_UP] = 
			    script_path_add(vdup((yyvsp[(2) - (3)].val)));
		}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1317 "./cfparse.y"
    { 
			cur_rmconf->script[SCRIPT_PHASE1_DOWN] = 
			    script_path_add(vdup((yyvsp[(2) - (3)].val)));
		}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1321 "./cfparse.y"
    { cur_rmconf->mode_cfg = (yyvsp[(2) - (2)].num); }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1322 "./cfparse.y"
    { cur_rmconf->gen_policy = (yyvsp[(2) - (2)].num); }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1323 "./cfparse.y"
    { cur_rmconf->support_proxy = (yyvsp[(2) - (2)].num); }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1324 "./cfparse.y"
    { cur_rmconf->ini_contact = (yyvsp[(2) - (2)].num); }
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1326 "./cfparse.y"
    {
#ifdef ENABLE_NATT
			cur_rmconf->nat_traversal = (yyvsp[(2) - (2)].num);
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1334 "./cfparse.y"
    {
#ifdef ENABLE_NATT
			cur_rmconf->nat_traversal = (yyvsp[(2) - (2)].num);
#else
			yyerror("NAT-T support not compiled in.");
#endif
		}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1342 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1350 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd_interval = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1359 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd_retry = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1368 "./cfparse.y"
    {
#ifdef ENABLE_DPD
			cur_rmconf->dpd_maxfails = (yyvsp[(2) - (2)].num);
#else
			yyerror("DPD support not compiled in.");
#endif
		}
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1377 "./cfparse.y"
    {
			cur_rmconf->prhead->lifetime = (yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num);
		}
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1381 "./cfparse.y"
    { cur_rmconf->pcheck_level = (yyvsp[(2) - (2)].num); }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1383 "./cfparse.y"
    {
#if 1
			yyerror("byte lifetime support is deprecated in Phase1");
			return -1;
#else
			yywarn("the lifetime of bytes in phase 1 "
				"will be ignored at the moment.");
			cur_rmconf->prhead->lifebyte = fix_lifebyte((yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num));
			if (cur_rmconf->prhead->lifebyte == 0)
				return -1;
#endif
		}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1397 "./cfparse.y"
    {
			struct secprotospec *spspec;

			spspec = newspspec();
			if (spspec == NULL)
				return -1;
			insspspec(spspec, &cur_rmconf->prhead);
		}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1410 "./cfparse.y"
    {
			struct etypes *new;
			new = racoon_malloc(sizeof(struct etypes));
			if (new == NULL) {
				yyerror("filed to allocate etypes");
				return -1;
			}
			new->type = (yyvsp[(2) - (2)].num);
			new->next = NULL;
			if (cur_rmconf->etypes == NULL)
				cur_rmconf->etypes = new;
			else {
				struct etypes *p;
				for (p = cur_rmconf->etypes;
				     p->next != NULL;
				     p = p->next)
					;
				p->next = new;
			}
		}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1433 "./cfparse.y"
    {
			cur_rmconf->certtype = (yyvsp[(1) - (3)].num);
			cur_rmconf->mycertfile = strdup((yyvsp[(2) - (3)].val)->v);
			vfree((yyvsp[(2) - (3)].val));
			cur_rmconf->myprivfile = strdup((yyvsp[(3) - (3)].val)->v);
			vfree((yyvsp[(3) - (3)].val));
		}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1442 "./cfparse.y"
    {
			char path[MAXPATHLEN];
			int ret = 0;

			getpathname(path, sizeof(path),
				LC_PATHTYPE_CERT, (yyvsp[(2) - (2)].val)->v);
			vfree((yyvsp[(2) - (2)].val));

			cur_rmconf->certtype = (yyvsp[(1) - (2)].num);
			cur_rmconf->send_cr = FALSE;
			cur_rmconf->send_cert = FALSE;
			cur_rmconf->verify_cert = FALSE;
			if (rsa_parse_file(cur_rmconf->rsa_private, path, RSA_TYPE_PRIVATE)) {
				yyerror("Couldn't parse keyfile.\n", path);
				return -1;
			}
			plog(LLV_DEBUG, LOCATION, NULL, "Private PlainRSA keyfile parsed: %s\n", path);
		}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1464 "./cfparse.y"
    {
			(yyval.num) = algtype2doi(algclass_isakmp_dh, (yyvsp[(1) - (1)].num));
			if ((yyval.num) == -1) {
				yyerror("must be DH group");
				return -1;
			}
		}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1472 "./cfparse.y"
    {
			if (ARRAYLEN(num2dhgroup) > (yyvsp[(1) - (1)].num) && num2dhgroup[(yyvsp[(1) - (1)].num)] != 0) {
				(yyval.num) = num2dhgroup[(yyvsp[(1) - (1)].num)];
			} else {
				yyerror("must be DH group");
				(yyval.num) = 0;
				return -1;
			}
		}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1483 "./cfparse.y"
    { (yyval.val) = NULL; }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1484 "./cfparse.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val); }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1485 "./cfparse.y"
    { (yyval.val) = (yyvsp[(1) - (1)].val); }
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1493 "./cfparse.y"
    {
			yyerror("strength directive is obsoleted.");
		}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1497 "./cfparse.y"
    {
			cur_rmconf->prhead->spspec->lifetime = (yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num);
		}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1502 "./cfparse.y"
    {
#if 1
			yyerror("byte lifetime support is deprecated");
			return -1;
#else
			cur_rmconf->prhead->spspec->lifebyte = fix_lifebyte((yyvsp[(3) - (4)].num) * (yyvsp[(4) - (4)].num));
			if (cur_rmconf->prhead->spspec->lifebyte == 0)
				return -1;
#endif
		}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1514 "./cfparse.y"
    {
			cur_rmconf->prhead->spspec->algclass[algclass_isakmp_dh] = (yyvsp[(2) - (2)].num);
		}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1519 "./cfparse.y"
    {
			if (cur_rmconf->prhead->spspec->vendorid != VENDORID_GSSAPI) {
				yyerror("wrong Vendor ID for gssapi_id");
				return -1;
			}
			cur_rmconf->prhead->spspec->gssid = strdup((yyvsp[(2) - (2)].val)->v);
		}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1528 "./cfparse.y"
    {
			int doi;
			int defklen;

			doi = algtype2doi((yyvsp[(1) - (3)].num), (yyvsp[(2) - (3)].num));
			if (doi == -1) {
				yyerror("algorithm mismatched 1");
				return -1;
			}

			switch ((yyvsp[(1) - (3)].num)) {
			case algclass_isakmp_enc:
			/* reject suppressed algorithms */
#ifndef HAVE_OPENSSL_RC5_H
				if ((yyvsp[(2) - (3)].num) == algtype_rc5) {
					yyerror("algorithm %s not supported",
					    s_attr_isakmp_enc(doi));
					return -1;
				}
#endif
#ifndef HAVE_OPENSSL_IDEA_H
				if ((yyvsp[(2) - (3)].num) == algtype_idea) {
					yyerror("algorithm %s not supported",
					    s_attr_isakmp_enc(doi));
					return -1;
				}
#endif

				cur_rmconf->prhead->spspec->algclass[algclass_isakmp_enc] = doi;
				defklen = default_keylen((yyvsp[(1) - (3)].num), (yyvsp[(2) - (3)].num));
				if (defklen == 0) {
					if ((yyvsp[(3) - (3)].num)) {
						yyerror("keylen not allowed");
						return -1;
					}
				} else {
					if ((yyvsp[(3) - (3)].num) && check_keylen((yyvsp[(1) - (3)].num), (yyvsp[(2) - (3)].num), (yyvsp[(3) - (3)].num)) < 0) {
						yyerror("invalid keylen %d", (yyvsp[(3) - (3)].num));
						return -1;
					}
				}
				if ((yyvsp[(3) - (3)].num))
					cur_rmconf->prhead->spspec->encklen = (yyvsp[(3) - (3)].num);
				else
					cur_rmconf->prhead->spspec->encklen = defklen;
				break;
			case algclass_isakmp_hash:
				cur_rmconf->prhead->spspec->algclass[algclass_isakmp_hash] = doi;
				break;
			case algclass_isakmp_ameth:
				cur_rmconf->prhead->spspec->algclass[algclass_isakmp_ameth] = doi;
				/*
				 * We may have to set the Vendor ID for the
				 * authentication method we're using.
				 */
				switch ((yyvsp[(2) - (3)].num)) {
				case algtype_gssapikrb:
					if (cur_rmconf->prhead->spspec->vendorid !=
					    VENDORID_UNKNOWN) {
						yyerror("Vendor ID mismatch "
						    "for auth method");
						return -1;
					}
					/*
					 * For interoperability with Win2k,
					 * we set the Vendor ID to "GSSAPI".
					 */
					cur_rmconf->prhead->spspec->vendorid =
					    VENDORID_GSSAPI;
					break;
				case algtype_rsasig:
					if (cur_rmconf->certtype == ISAKMP_CERT_PLAINRSA) {
						if (rsa_list_count(cur_rmconf->rsa_private) == 0) {
							yyerror ("Private PlainRSA key not set. "
								"Use directive 'certificate_type plainrsa ...'\n");
							return -1;
						}
						if (rsa_list_count(cur_rmconf->rsa_public) == 0) {
							yyerror ("Public PlainRSA keys not set. "
								"Use directive 'peers_certfile plainrsa ...'\n");
							return -1;
						}
					}
					break;
				default:
					break;
				}
				break;
			default:
				yyerror("algorithm mismatched 2");
				return -1;
			}
		}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1625 "./cfparse.y"
    { (yyval.num) = 1; }
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1626 "./cfparse.y"
    { (yyval.num) = 60; }
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1627 "./cfparse.y"
    { (yyval.num) = (60 * 60); }
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1630 "./cfparse.y"
    { (yyval.num) = 1; }
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1631 "./cfparse.y"
    { (yyval.num) = 1024; }
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1632 "./cfparse.y"
    { (yyval.num) = (1024 * 1024); }
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1633 "./cfparse.y"
    { (yyval.num) = (1024 * 1024 * 1024); }
    break;



/* Line 1806 of yacc.c  */
#line 4197 "z.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 1635 "./cfparse.y"


static struct proposalspec *
newprspec()
{
	struct proposalspec *new;

	new = racoon_calloc(1, sizeof(*new));
	if (new == NULL)
		yyerror("failed to allocate proposal");

	return new;
}

/*
 * insert into head of list.
 */
static void
insprspec(prspec, head)
	struct proposalspec *prspec;
	struct proposalspec **head;
{
	if (*head != NULL)
		(*head)->prev = prspec;
	prspec->next = *head;
	*head = prspec;
}

static struct secprotospec *
newspspec()
{
	struct secprotospec *new;

	new = racoon_calloc(1, sizeof(*new));
	if (new == NULL) {
		yyerror("failed to allocate spproto");
		return NULL;
	}

	new->encklen = 0;	/*XXX*/

	/*
	 * Default to "uknown" vendor -- we will override this
	 * as necessary.  When we send a Vendor ID payload, an
	 * "unknown" will be translated to a KAME/racoon ID.
	 */
	new->vendorid = VENDORID_UNKNOWN;

	return new;
}

/*
 * insert into head of list.
 */
static void
insspspec(spspec, head)
	struct secprotospec *spspec;
	struct proposalspec **head;
{
	spspec->back = *head;

	if ((*head)->spspec != NULL)
		(*head)->spspec->prev = spspec;
	spspec->next = (*head)->spspec;
	(*head)->spspec = spspec;
}

/* set final acceptable proposal */
static int
set_isakmp_proposal(rmconf, prspec)
	struct remoteconf *rmconf;
	struct proposalspec *prspec;
{
	struct proposalspec *p;
	struct secprotospec *s;
	int prop_no = 1; 
	int trns_no = 1;
	int32_t types[MAXALGCLASS];

	p = prspec;
	if (p->next != 0) {
		plog(LLV_ERROR, LOCATION, NULL,
			"multiple proposal definition.\n");
		return -1;
	}

	/* mandatory check */
	if (p->spspec == NULL) {
		yyerror("no remote specification found: %s.\n",
			saddr2str(rmconf->remote));
		return -1;
	}
	for (s = p->spspec; s != NULL; s = s->next) {
		/* XXX need more to check */
		if (s->algclass[algclass_isakmp_enc] == 0) {
			yyerror("encryption algorithm required.");
			return -1;
		}
		if (s->algclass[algclass_isakmp_hash] == 0) {
			yyerror("hash algorithm required.");
			return -1;
		}
		if (s->algclass[algclass_isakmp_dh] == 0) {
			yyerror("DH group required.");
			return -1;
		}
		if (s->algclass[algclass_isakmp_ameth] == 0) {
			yyerror("authentication method required.");
			return -1;
		}
	}

	/* skip to last part */
	for (s = p->spspec; s->next != NULL; s = s->next)
		;

	while (s != NULL) {
		plog(LLV_DEBUG2, LOCATION, NULL,
			"lifetime = %ld\n", (long)
			(s->lifetime ? s->lifetime : p->lifetime));
		plog(LLV_DEBUG2, LOCATION, NULL,
			"lifebyte = %d\n",
			s->lifebyte ? s->lifebyte : p->lifebyte);
		plog(LLV_DEBUG2, LOCATION, NULL,
			"encklen=%d\n", s->encklen);

		memset(types, 0, ARRAYLEN(types));
		types[algclass_isakmp_enc] = s->algclass[algclass_isakmp_enc];
		types[algclass_isakmp_hash] = s->algclass[algclass_isakmp_hash];
		types[algclass_isakmp_dh] = s->algclass[algclass_isakmp_dh];
		types[algclass_isakmp_ameth] =
		    s->algclass[algclass_isakmp_ameth];

		/* expanding spspec */
		clean_tmpalgtype();
		trns_no = expand_isakmpspec(prop_no, trns_no, types,
				algclass_isakmp_enc, algclass_isakmp_ameth + 1,
				s->lifetime ? s->lifetime : p->lifetime,
				s->lifebyte ? s->lifebyte : p->lifebyte,
				s->encklen, s->vendorid, s->gssid,
				rmconf);
		if (trns_no == -1) {
			plog(LLV_ERROR, LOCATION, NULL,
				"failed to expand isakmp proposal.\n");
			return -1;
		}

		s = s->prev;
	}

	if (rmconf->proposal == NULL) {
		plog(LLV_ERROR, LOCATION, NULL,
			"no proposal found.\n");
		return -1;
	}

	return 0;
}

static void
clean_tmpalgtype()
{
	int i;
	for (i = 0; i < MAXALGCLASS; i++)
		tmpalgtype[i] = 0;	/* means algorithm undefined. */
}

static int
expand_isakmpspec(prop_no, trns_no, types,
		class, last, lifetime, lifebyte, encklen, vendorid, gssid,
		rmconf)
	int prop_no, trns_no;
	int *types, class, last;
	time_t lifetime;
	int lifebyte;
	int encklen;
	int vendorid;
	char *gssid;
	struct remoteconf *rmconf;
{
	struct isakmpsa *new;

	/* debugging */
    {
	int j;
	char tb[10];
	plog(LLV_DEBUG2, LOCATION, NULL,
		"p:%d t:%d\n", prop_no, trns_no);
	for (j = class; j < MAXALGCLASS; j++) {
		snprintf(tb, sizeof(tb), "%d", types[j]);
		plog(LLV_DEBUG2, LOCATION, NULL,
			"%s%s%s%s\n",
			s_algtype(j, types[j]),
			types[j] ? "(" : "",
			tb[0] == '0' ? "" : tb,
			types[j] ? ")" : "");
	}
	plog(LLV_DEBUG2, LOCATION, NULL, "\n");
    }

#define TMPALGTYPE2STR(n) \
	s_algtype(algclass_isakmp_##n, types[algclass_isakmp_##n])
		/* check mandatory values */
		if (types[algclass_isakmp_enc] == 0
		 || types[algclass_isakmp_ameth] == 0
		 || types[algclass_isakmp_hash] == 0
		 || types[algclass_isakmp_dh] == 0) {
			yyerror("few definition of algorithm "
				"enc=%s ameth=%s hash=%s dhgroup=%s.\n",
				TMPALGTYPE2STR(enc),
				TMPALGTYPE2STR(ameth),
				TMPALGTYPE2STR(hash),
				TMPALGTYPE2STR(dh));
			return -1;
		}
#undef TMPALGTYPE2STR

	/* set new sa */
	new = newisakmpsa();
	if (new == NULL) {
		yyerror("failed to allocate isakmp sa");
		return -1;
	}
	new->prop_no = prop_no;
	new->trns_no = trns_no++;
	new->lifetime = lifetime;
	new->lifebyte = lifebyte;
	new->enctype = types[algclass_isakmp_enc];
	new->encklen = encklen;
	new->authmethod = types[algclass_isakmp_ameth];
	new->hashtype = types[algclass_isakmp_hash];
	new->dh_group = types[algclass_isakmp_dh];
	new->vendorid = vendorid;
#ifdef HAVE_GSSAPI
	if (new->authmethod == OAKLEY_ATTR_AUTH_METHOD_GSSAPI_KRB) {
		if (gssid != NULL) {
			new->gssid = vmalloc(strlen(gssid));
			memcpy(new->gssid->v, gssid, new->gssid->l);
			racoon_free(gssid);
		} else {
			/*
			 * Allocate the default ID so that it gets put
			 * into a GSS ID attribute during the Phase 1
			 * exchange.
			 */
			new->gssid = gssapi_get_default_gss_id();
		}
	}
#endif
	insisakmpsa(new, rmconf);

	return trns_no;
}

static int
listen_addr (struct sockaddr *addr, int udp_encap)
{
	struct myaddrs *p;

	p = newmyaddr();
	if (p == NULL) {
		yyerror("failed to allocate myaddrs");
		return -1;
	}
	p->addr = addr;
	if (p->addr == NULL) {
		yyerror("failed to copy sockaddr ");
		delmyaddr(p);
		return -1;
	}
	p->udp_encap = udp_encap;

	insmyaddr(p, &lcconf->myaddrs);

	lcconf->autograbaddr = 0;
	return 0;
}

#if 0
/*
 * fix lifebyte.
 * Must be more than 1024B because its unit is kilobytes.
 * That is defined RFC2407.
 */
static int
fix_lifebyte(t)
	unsigned long t;
{
	if (t < 1024) {
		yyerror("byte size should be more than 1024B.");
		return 0;
	}

	return(t / 1024);
}
#endif

int
cfparse()
{
	int error;

	yycf_init_buffer();

	if (yycf_switch_buffer(lcconf->racoon_conf) != 0)
		return -1;

	error = yyparse();
	if (error != 0) {
		if (yyerrorcount) {
			plog(LLV_ERROR, LOCATION, NULL,
				"fatal parse failure (%d errors)\n",
				yyerrorcount);
		} else {
			plog(LLV_ERROR, LOCATION, NULL,
				"fatal parse failure.\n");
		}
		return -1;
	}

	if (error == 0 && yyerrorcount) {
		plog(LLV_ERROR, LOCATION, NULL,
			"parse error is nothing, but yyerrorcount is %d.\n",
				yyerrorcount);
		exit(1);
	}

	yycf_clean_buffer();

	plog(LLV_DEBUG2, LOCATION, NULL, "parse successed.\n");

	return 0;
}

int
cfreparse()
{
	flushph2();
	flushph1();
	flushrmconf();
	flushsainfo();
	clean_tmpalgtype();
	yycf_init_buffer();

	if (yycf_switch_buffer(lcconf->racoon_conf) != 0)
		return -1;

	return(cfparse());
}

#ifdef ENABLE_ADMINPORT
static void
adminsock_conf(path, owner, group, mode_dec)
	vchar_t *path;
	vchar_t *owner;
	vchar_t *group;
	int mode_dec;
{
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	mode_t mode = 0;
	uid_t uid;
	gid_t gid;
	int isnum;

	adminsock_path = path->v;

	if (owner == NULL)
		return;

	errno = 0;
	uid = atoi(owner->v);
	isnum = !errno;
	if (((pw = getpwnam(owner->v)) == NULL) && !isnum)
		yyerror("User \"%s\" does not exist", owner->v);

	if (pw)
		adminsock_owner = pw->pw_uid;
	else
		adminsock_owner = uid;

	if (group == NULL)
		return;

	errno = 0;
	gid = atoi(group->v);
	isnum = !errno;
	if (((gr = getgrnam(group->v)) == NULL) && !isnum)
		yyerror("Group \"%s\" does not exist", group->v);

	if (gr)
		adminsock_group = gr->gr_gid;
	else
		adminsock_group = gid;

	if (mode_dec == -1)
		return;

	if (mode_dec > 777)
		yyerror("Mode 0%03o is invalid", mode_dec);
	if (mode_dec >= 400) { mode += 0400; mode_dec -= 400; }
	if (mode_dec >= 200) { mode += 0200; mode_dec -= 200; }
	if (mode_dec >= 100) { mode += 0200; mode_dec -= 100; }

	if (mode_dec > 77)
		yyerror("Mode 0%03o is invalid", mode_dec);
	if (mode_dec >= 40) { mode += 040; mode_dec -= 40; }
	if (mode_dec >= 20) { mode += 020; mode_dec -= 20; }
	if (mode_dec >= 10) { mode += 020; mode_dec -= 10; }

	if (mode_dec > 7)
		yyerror("Mode 0%03o is invalid", mode_dec);
	if (mode_dec >= 4) { mode += 04; mode_dec -= 4; }
	if (mode_dec >= 2) { mode += 02; mode_dec -= 2; }
	if (mode_dec >= 1) { mode += 02; mode_dec -= 1; }
	
	adminsock_mode = mode;

	return;
}
#endif

