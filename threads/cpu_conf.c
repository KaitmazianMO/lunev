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
        if (!line)
                return -1;

        const char *number_str = line;
        while(!isdigit(*number_str))
                number_str++;

        if (number_str != line && number_str[-1] == '-')
                --number_str;

        return sscanf(number_str, "%d", n);
}

int get_cpu_conf(struct cpu_conf *pconf)
{
        int nthreads = 1;
        int threads_per_core = 1;
        int sockets = 1;

        if (access("/sys/devices/system/cpu/smt/active", F_OK ) == 0) {
                INFO("File way.");
                FILE *fht_active = fopen("/sys/devices/system/cpu/smt/active", "r");
                if (fht_active == NULL)
                        return -errno;

                int active = 0;
                if (fscanf(fht_active, "%d", &active) < 0)
                        return -errno;
                if (active)
                        threads_per_core = 2;
                else
                        threads_per_core = 1;
                nthreads = get_nprocs();
        } else {
                INFO("Another way.");
                FILE *flscpu = popen("lscpu -y", "r");
                if (flscpu != NULL) {
                        char *lscpu_dump = NULL;
                        size_t dump_size = 0;
                        getdelim(&lscpu_dump, &dump_size, '\0', flscpu);
                        if (lscpu_dump == NULL)
                                return -errno;

                        if (read_line_number(strstr(lscpu_dump, "CPU(s):"), &nthreads) <= 0 ||
                            read_line_number(strstr(lscpu_dump, "Thread(s) per core:"), &threads_per_core) <= 0 ||
                            read_line_number(strstr(lscpu_dump, "Socket(s):"), &sockets) <= 0)
                                return -1;
                        free(lscpu_dump);
                        fclose (flscpu);
                }
        }
        pconf->threads = nthreads;
        pconf->cores = nthreads / threads_per_core;
        pconf->sockets = sockets;
        pconf->L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        return 0;
}