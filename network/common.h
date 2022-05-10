#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <strings.h>
#include <stdint.h>

#include "debug.h"
#include "config.h"
#include "utils.h"

typedef int16_t counter_t;

typedef char strnum_t[16];

#define sockaddr_cast(addr) cast(struct sockaddr *, addr)

static inline void init_sockaddr_in(struct sockaddr_in *sockaddr, in_port_t port, in_addr_t in_addr)
{
        bzero(sockaddr, sizeof(*sockaddr));
        sockaddr->sin_family = PF_INET;
        sockaddr->sin_port = port;
        sockaddr->sin_addr.s_addr = in_addr;
}

static inline int setsockopt_no_val(int sk, int lvl, int optname)
{
        int enable = 1;
        return setsockopt(sk, lvl, optname, &enable, sizeof(enable));
}

static inline void SIGPIPE_handler(int sig)
{
        ERROR("Lost connection.");
}

static inline int set_default_broken_connection_handler()
{
        return signal(SIGPIPE, SIGPIPE_handler) == SIG_ERR;
}

static inline int enable_tcp_keep_alive_opt(int sk)
{
        if (setsockopt_no_val(sk, SOL_SOCKET, SO_KEEPALIVE) != 0)
                return -1;

        int ka_probs = TCP_KEEP_ALIVE_PROBES;
        int ka_time = TCP_KEEP_ALIVE_TIME;
        int ka_intvl = TCP_KEEP_ALIVE_INTVL;
        if (setsockopt(sk, IPPROTO_TCP, TCP_KEEPCNT, &ka_probs, sizeof(ka_probs)) != 0 ||
            setsockopt(sk, IPPROTO_TCP, TCP_KEEPIDLE, &ka_time, sizeof(ka_time)) != 0 ||
            setsockopt(sk, IPPROTO_TCP, TCP_KEEPINTVL, &ka_intvl, sizeof(ka_intvl)) != 0)
                return -1;
        return 0;
}

#endif // COMMON_H_INCLUDED