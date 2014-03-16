/***************************************************************************
 * File Name    : smuxctl.c
 * Description  : contails smux driver configura utlity code
 ***************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <limits.h>
#include "busybox.h"

/* Passed in smux_ioctl_args structure to determine behaviour. Should be same as if_smux.h */
enum smux_ioctl_cmds {
  ADD_SMUX_CMD,
  REM_SMUX_CMD,
};

enum smux_proto_types {
  SMUX_PROTO_PPPOE,
  SMUX_PROTO_IPOE,
  SMUX_PROTO_BRIDGE,
  SMUX_PROTO_HIGHEST
};

struct smux_ioctl_args {
  int cmd; /* Should be one of the smux_ioctl_cmds enum above. */
  int  proto;
  char ifname[IFNAMSIZ]; /* real device */
  union {
    char ifname[IFNAMSIZ]; /* overlay device */
  } u;
};

/* same as sockios.h */
#define SIOCSIFSMUX	0x89C2

/* On entry, table points to the length of the current string plus
 * nul terminator plus data length for the subsequent entry.  The
 * return value is the last data entry for the matching string. */
static const char *xfind_str(const char *table, const char *str)
{
	while (strcasecmp(str, table+1) != 0) {
		if (!*(table += table[0])) {
			bb_show_usage();
		}
	}
	return table - 1;
}

static const char cmds[] = {
  5, ADD_SMUX_CMD, 7,
  'a', 'd', 'd', 0,
  3, REM_SMUX_CMD, 7,
  'r', 'e', 'm', 0,
};

static const char smux_protos[] = {
  SMUX_PROTO_PPPOE, 8,
  'p','p','p','o','e',0,
  SMUX_PROTO_IPOE, 7,
  'i','p','o','e',0,
  SMUX_PROTO_BRIDGE, 9,
  'b','r','i','d','g','e',0,
};

int smuxctl_main(int argc, char **argv)
{
  struct smux_ioctl_args ifr;
  const char *p;
  int fd;

  if (argc < 3) {
    bb_show_usage();
  }

  memset(&ifr, 0, sizeof(struct smux_ioctl_args));

  ++argv;
  p = xfind_str(cmds+2, *argv);
  ifr.cmd = *p;

  if (argc != p[-1]) {
    bb_show_usage();
  }

  if(ifr.cmd == ADD_SMUX_CMD) {
    ++argv;
    p = xfind_str(smux_protos+1, *argv);
    ifr.proto = *p;
    strcpy(ifr.ifname, argv[1]);
    strcpy(ifr.u.ifname, argv[2]);
  }
  else if(ifr.cmd == REM_SMUX_CMD) {
    strcpy(ifr.u.ifname, argv[1]);
  }

  if (((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		|| (ioctl(fd, SIOCSIFSMUX, &ifr) < 0)) {
    bb_perror_msg_and_die("socket or ioctl error for %s", *argv);
  }

  return 0;
}
