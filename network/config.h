#ifndef CONFIG_H_INCUDED
#define CONFIG_H_INCUDED

#define SERVER_PORT 4789
#define CLIENT_PORT 4790

#define TCP_KEEP_ALIVE_PROBES 2
#define TCP_KEEP_ALIVE_TIME   3
#define TCP_KEEP_ALIVE_INTVL  1

#define ACCEPT_TIMEOUT_SEC 2
#define ACCEPT_TIMEOUT_USEC 0
#define CLIENT_SEND_TIMEOUT_SEC 2
#define CLIENT_SEND_TIMEOUT_USEC 0

#define N_CLIENTS_MAX 256

#define BACK_LOG 255

#define BROADCAST_MSG "MAGIC"

#include <math.h>
static double f(double x) { return x*cos(atan(x)); }

#define DX           1e-4
#define LOWER_BOUND  0
#define UPPER_BOUND  5e5

#endif // CONFIG_H_INCUDED