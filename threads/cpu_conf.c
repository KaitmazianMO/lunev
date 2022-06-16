#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include "cpu_conf.h"

static int read_number_from_line(const char *line, int *n)
{
        return sscanf(line, "%*[^-123456789] %d", n);
}

int get_cpu_conf(struct cpu_conf *pconf)
{
        int nthreads = 1;
        int threads_per_core = 1;
        int sockets = 1;
        FILE *flscpu = popen("lscpu -y", "r");
        if (flscpu != NULL) {
                char *lscpu_dump = NULL;
                size_t dump_size = 0;
                getdelim(&lscpu_dump, &dump_size, '\0', flscpu);
                if (lscpu_dump == NULL)
                        return -errno;

                if (read_number_from_line(strstr(lscpu_dump, "CPU(s):"), &nthreads) <= 0 ||
                    read_number_from_line(strstr(lscpu_dump, "Thread(s) per core:"), &threads_per_core) <= 0 ||
                    read_number_from_line(strstr(lscpu_dump, "Socket(s):"), &sockets) <= 0)
                        return -1;
                free(lscpu_dump);
                fclose (flscpu);
        } else {
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
        }
        pconf->nthreads = nthreads;
        pconf->ncores = nthreads / threads_per_core;
        pconf->nsockets = sockets;
        pconf->L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        return 0;
}