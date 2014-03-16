/*
 *  Report system information including
 *      number of processes, uptime, system average load
 *      and memory information.
 *
 */


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include "busybox.h"

static const int FSHIFT = 16;              /* nr of bits of precision */
#define FIXED_1         (1<<FSHIFT)     /* 1.0 as fixed-point */
#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

static int sysinfo_mem(void)
{
	struct sysinfo info;
	sysinfo(&info);

	/* Kernels prior to 2.4.x will return info.mem_unit==0, so cope... */
	if (info.mem_unit==0) {
		info.mem_unit=1;
	}
	info.mem_unit*=1024;

	/* TODO:  Make all this stuff not overflow when mem >= 4 Gib */
	info.totalram/=info.mem_unit;
	info.freeram/=info.mem_unit;
	info.totalswap/=info.mem_unit;
	info.freeswap/=info.mem_unit;
	info.sharedram/=info.mem_unit;
	info.bufferram/=info.mem_unit;

	printf("%6s%13s%13s%13s%13s%13s\n", "", "total", "used", "free",
			"shared", "buffers");

	printf("%6s%13ld%13ld%13ld%13ld%13ld\n", "Mem:", info.totalram,
			info.totalram-info.freeram, info.freeram,
			info.sharedram, info.bufferram);

	printf("%6s%13ld%13ld%13ld\n", "Swap:", info.totalswap,
			info.totalswap-info.freeswap, info.freeswap);

	printf("%6s%13ld%13ld%13ld\n", "Total:", info.totalram+info.totalswap,
			(info.totalram-info.freeram)+(info.totalswap-info.freeswap),
			info.freeram+info.freeswap);
	return EXIT_SUCCESS;
}



static int sysinfo_uptime(void)
{
	int updays, uphours, upminutes;
	struct sysinfo info;
	struct tm *current_time;
	time_t current_secs;

	time(&current_secs);
	current_time = localtime(&current_secs);

	sysinfo(&info);

    printf("Number of processes: %d\n", info.procs);
	printf(" %2d:%02d%s  up ",
			current_time->tm_hour%12 ? current_time->tm_hour%12 : 12,
			current_time->tm_min, current_time->tm_hour > 11 ? "pm" : "am");
	updays = (int) info.uptime / (60*60*24);
	if (updays)
		printf("%d day%s, ", updays, (updays != 1) ? "s" : "");
	upminutes = (int) info.uptime / 60;
	uphours = (upminutes / 60) % 24;
	upminutes %= 60;
	if(uphours)
		printf("%2d:%02d, ", uphours, upminutes);
	else
		printf("%d min, ", upminutes);

	printf("\nload average: 1 min:%ld.%02ld, 5 min:%ld.%02ld, 15 min:%ld.%02ld\n",
			LOAD_INT(info.loads[0]), LOAD_FRAC(info.loads[0]),
			LOAD_INT(info.loads[1]), LOAD_FRAC(info.loads[1]),
			LOAD_INT(info.loads[2]), LOAD_FRAC(info.loads[2]));

	return EXIT_SUCCESS;
}

extern int sysinfo_main(int argc, char **argv)
{
    sysinfo_uptime();
    sysinfo_mem();

    return EXIT_SUCCESS;

}
