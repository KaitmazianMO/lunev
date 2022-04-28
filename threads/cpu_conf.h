#ifndef CPU_CONF_H_INCLUDED
#define CPU_CONF_H_INCLUDED

struct cpu_conf {
        int nsockets;
        int ncores;
        int threads_per_core;
        int nthreads;
        int L1_cache_line_size;
};

int get_cpu_conf(struct cpu_conf *c);

#endif // CPU_CONF_H_INCLUDED