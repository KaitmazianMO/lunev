#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "debug.h"
#include "cpu_conf.h"

static int read_line_number(const char *line, int *n)
{
        const char *number_str = line;
        while(!isdigit(*number_str))
                number_str++;

        if (number_str != line && number_str[-1] == '-')
                --number_str;

        return sscanf(number_str, "%d", n);
}

static int get_cpu_conf_using_ht_active_file(struct cpu_conf *pconf)
{
        FILE *fht_active = fopen("/sys/devices/system/cpu/smt/active", "r");
        if (fht_active == NULL) {
                INFO("Can't open /sys/devices/system/cpu/smt/active.");
                return -errno;
        }
        int active = 0;
        fscanf(fht_active, "%d", &active);
        pconf->threads_per_core = active ? 2 : 1;
        pconf->nthreads = get_nprocs();
        pconf->ncores = pconf->nthreads / pconf->threads_per_core;
        pconf->L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        pconf->nsockets = -1; /* Have no idea how many sockets are there. */
        return 0;
}

static int get_cpu_info_using_lscpu(struct cpu_conf *pconf)
{
        FILE *flscpu = popen("lscpu -y", "r");
        if (flscpu == NULL) {
                INFO("popen with lscpu failed.");
                return -errno;
        }

        char *lscpu_dump = NULL;
        size_t dump_size = 0;
        getdelim(&lscpu_dump, &dump_size, '\0', flscpu);
        if (lscpu_dump == NULL) {
                INFO("Failed to read lscpu dump");
                return -errno;
        }

        if (read_line_number(strstr(lscpu_dump, "CPU(s):"), &pconf->nthreads) <= 0 ||
            read_line_number(strstr(lscpu_dump, "Thread(s) per core:"), &pconf->threads_per_core) <= 0 ||
            read_line_number(strstr(lscpu_dump, "Socket(s):"), &pconf->nsockets) <= 0) {
                INFO("Can't read number of cpus,threads or sockets");
                return -1;
        }
        pconf->L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        pconf->ncores = pconf->nthreads / pconf->threads_per_core;
        free(lscpu_dump);
        pclose(flscpu);
        return 0;
}

static void get_cpu_conf_default(struct cpu_conf *pconf)
{
        pconf->nthreads = get_nprocs();
        pconf->threads_per_core = 2;
        pconf->ncores = pconf->threads_per_core * pconf->nthreads;
        pconf->L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        pconf->nsockets = -1;
}

int get_cpu_conf(struct cpu_conf *pconf)
{
        if (access("/sys/devices/system/cpu/smt/active", F_OK ) == 0) {
                if (get_cpu_conf_using_ht_active_file(pconf) == 0)
                        return 0;
        } else {
                if (get_cpu_info_using_lscpu(pconf) == 0)
                        return 0;
        }

        get_cpu_conf_default(pconf);
        return -1;
}