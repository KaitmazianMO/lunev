#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

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
        FILE *flscpu = popen("lscpu -y", "r");
        if (flscpu == NULL)
                return -errno;

        char *lscpu_dump = NULL;
        size_t dump_size = 0;
        getdelim(&lscpu_dump, &dump_size, '\0', flscpu);
        if (lscpu_dump == NULL)
                return -errno;

        int nthreads = 0;
        int threads_per_core = 0;
        int cores_per_sock = 0;
        int sockets = 0;
        if (read_line_number(strstr(lscpu_dump, "CPU(s):"), &nthreads) <= 0 ||
            read_line_number(strstr(lscpu_dump, "Thread(s) per core:"), &threads_per_core) <= 0 ||
            read_line_number(strstr(lscpu_dump, "Core(s) per socket:"), &cores_per_sock) <= 0 ||
            read_line_number(strstr(lscpu_dump, "Socket(s):"), &sockets) <= 0)
                return -1;        

        free(lscpu_dump);
        fclose (flscpu);

        pconf->threads = nthreads;
        pconf->cores = cores_per_sock * sockets;
        pconf->sockets = sockets;
        pconf->L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        return 0;
}