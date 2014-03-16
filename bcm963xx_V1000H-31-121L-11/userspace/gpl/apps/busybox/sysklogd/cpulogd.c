#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <sys/syslog.h>

#define DEFAULT_SAMPLE_INTERVAL  5  /* seconds */
#define DEFAULT_CPU_LOG_INTERVAL 60 /* seconds */

struct cpu_stats
{
    int idle_jiffies;
    int total_jiffies;
};

static struct cpu_stats start_sample;
static struct cpu_stats end_sample;

static int cpu_log_start = 0;
static int cpu_log_interval = DEFAULT_CPU_LOG_INTERVAL;

static void do_cpu_sample(struct cpu_stats *sample)
{
    char col[9][32];
    char line[256];
    FILE *fs = NULL;

    if (sample == NULL)
        return;

    memset(sample, 0, sizeof(struct cpu_stats));

    if ((fs = fopen("/proc/stat", "r")) == NULL)
    {
        return;
    }

    if (fgets(line, sizeof(line), fs))
    {
        /* title, user, nice, system, idle, iowait, irq, softirq */
        sscanf(line, "%s %s %s %s %s %s %s %s %s",
                     col[0], col[1], col[2], col[3], col[4], col[5],
                     col[6], col[7], col[8]);

        sample->total_jiffies = atoi(col[1]) + atoi(col[2]) +
                                atoi(col[3]) + atoi(col[4]) +
                                atoi(col[5]) + atoi(col[6]) + atoi(col[7]);
        sample->idle_jiffies = atoi(col[4]);
    }

    fclose(fs);
}

static void do_cpu_log(void)
{
    float cpu_percent = 0;
    int idle_period;
    int work_period;
    int total_period;

    idle_period = end_sample.idle_jiffies - start_sample.idle_jiffies;
    total_period = end_sample.total_jiffies - start_sample.total_jiffies;
    work_period = total_period - idle_period;

    if (work_period && total_period)
    {
        cpu_percent = (float)work_period / total_period * 100;
        syslog(LOG_NOTICE, "CPU utilization:    %.1f%%\n", cpu_percent);
    }
}

static void doCpulogd(void)
{
    int ret;
    fd_set fds;
    struct timeval tv;
    struct timeval *ptv;

    for (;;) {
        FD_ZERO(&fds);

        if (cpu_log_start)
            tv.tv_sec = DEFAULT_SAMPLE_INTERVAL;
        else
            tv.tv_sec = cpu_log_interval - DEFAULT_SAMPLE_INTERVAL;

        tv.tv_usec = 0;
        ptv = &tv;

        ret = select(0, &fds, NULL, NULL, ptv);
        if (ret < 0)
        {
            if (errno == EINTR)
            {
                /* alarm may have happened. */
                continue;
            }
            return;
        }
        else if (ret == 0)
        {
            cpu_log_start = !cpu_log_start;

            if (cpu_log_start)
                do_cpu_sample(&start_sample);
            else
            {
                do_cpu_sample(&end_sample);
                do_cpu_log();
            }

            continue;
        }
    }
}

static void set_realtime_priority(void)
{
    int sched;
    int max_prior;
    struct sched_param param;

    max_prior = sched_get_priority_max(SCHED_FIFO);
    if (max_prior != -1)
    {
        param.sched_priority = max_prior;
        if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
           printf("failed to set real-time priority to cpulogd\n");
    }
}

extern int cpulogd_main(int argc, char **argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "t:")) > 0)
    {
        switch (opt)
        {
        case 't':
            cpu_log_interval = atoi(optarg);
            if (cpu_log_interval <= DEFAULT_SAMPLE_INTERVAL)
                cpu_log_interval = DEFAULT_SAMPLE_INTERVAL + 5;
            break;
        default:
            break;
        }
    }

    /*
    * detach from the terminal so we don't catch the user typing control-c
    */
    if (setsid() == -1)
        printf("Could not detach from terminal\n");

    set_realtime_priority();

    doCpulogd();

    return EXIT_SUCCESS;
}
