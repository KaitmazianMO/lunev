#ifndef CPU_CONF_H_INCLUDED
#define CPU_CONF_H_INCLUDED

struct cpu_conf {
        long sockets;
        long cores;
        long threads;
        long L1_cache_line_size;
};

int get_cpu_conf(struct cpu_conf *c);

#endif // CPU_CONF_H_INCLUDED