#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include "config.h"
#include "common.h"
#include "utils.h"
#include "debug.h"

int main(int argc, char *argv[])
{
        (void)f;

        int listener_sk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener_sk == -1)
                ERROR("no listener_sk");

        if (setsockopt_no_val(listener_sk, SOL_SOCKET, SO_REUSEADDR) != 0)
                ERROR("failed to set reuseaddr to listener");

        struct timeval accept_timeout = {
                .tv_sec = ACCEPT_TIMEOUT_SEC,
                .tv_usec = ACCEPT_TIMEOUT_USEC
        };
        if (setsockopt(listener_sk, SOL_SOCKET, SO_RCVTIMEO, &accept_timeout, sizeof(accept_timeout)) != 0)
                ERROR("Can't set recv timeout option to listener.");

        if (enable_tcp_keep_alive_opt(listener_sk) != 0)
                ERROR("Can't set tcp keep alive option to listener");

        struct sockaddr_in listener_addr;
        init_sockaddr_in(&listener_addr, htons(SERVER_PORT), htonl(INADDR_ANY));
        if (bind(listener_sk, sockaddr_cast(&listener_addr), sizeof(listener_addr)) != 0)
                ERROR("listener bind failed");

        if (listen(listener_sk, BACK_LOG) != 0)
                ERROR("listen failed with listener_sk");

        int broadcast_sk = socket(PF_INET, SOCK_DGRAM, 0);
        if (broadcast_sk == -1)
                ERROR("no broadcast_fd");

        if (setsockopt_no_val(broadcast_sk, SOL_SOCKET, SO_BROADCAST) != 0)
                ERROR("setsockopt failed with broadcast");

        if (setsockopt_no_val(broadcast_sk, SOL_SOCKET, SO_REUSEADDR) != 0)
                ERROR("failed to set reuseaddr to broadcast sk");

        struct sockaddr_in broadcast_addr;
        init_sockaddr_in(&broadcast_addr, htons(CLIENT_PORT), htonl(INADDR_BROADCAST));
        if (bind(broadcast_sk, sockaddr_cast(&broadcast_addr), sizeof(broadcast_addr)) != 0)
                ERROR("broadcast bind failed");

        INFO("Brodcasting...");
        if (sendto(broadcast_sk, BROADCAST_MSG, sizeof(BROADCAST_MSG), 0,
                   sockaddr_cast(&broadcast_addr), sizeof(broadcast_addr)) != sizeof(BROADCAST_MSG))
                ERROR("broadcast send failed");

        if (close(broadcast_sk) != 0)
                ERROR("broadcast sk close failed");

        INFO("Accepting clients...");
        errno = 0;
        int n_clients = 0;
        struct client {
                int sk;
                counter_t n_threads;
        } clients[N_CLIENTS_MAX];
        struct timeval no_timeout = {
                .tv_sec = 0,
                .tv_usec = 0
        };
        for (; n_clients < N_CLIENTS_MAX; ++n_clients) {
                clients[n_clients].sk = accept(listener_sk, NULL, NULL);
                if (clients[n_clients].sk != -1) {
                        if (setsockopt(clients[n_clients].sk, SOL_SOCKET, SO_RCVTIMEO,
                            &no_timeout, sizeof(no_timeout)) != 0)
                                ERROR("Can't set recv timeout for clinent[%d]", n_clients);
                        if (enable_tcp_keep_alive_opt(listener_sk) != 0)
                                ERROR("Can't set tcp keep alive option to client[%d]", n_clients);
                } else {
                        if (errno == EAGAIN)
                                break;
                        else
                                ERROR("Can't accept %d client.", n_clients);
                }
        }

        struct timeval time_start;
        gettimeofday(&time_start, NULL);

        if (n_clients == 0)
                ERROR("There is no any clients.");
        else if (n_clients == N_CLIENTS_MAX)
                ERROR("Too many clients(%d).", N_CLIENTS_MAX);
        else
                INFO("Found %d clinet(s).", n_clients);

        counter_t n_threads_net = 0;
        for (int i = 0; i < n_clients; ++i) {
                if (recv(clients[i].sk, &n_threads_net, sizeof(n_threads_net), 0) != sizeof(n_threads_net))
                        ERROR("recv failed with client_sk[%d]", i);
                clients[i].n_threads = ntohs(n_threads_net);
                INFO("Client #%d has %d thread(s)", i, clients[i].n_threads);
        }

        counter_t total_n_threads = 0;
        for (int i = 0; i < n_clients; ++i)
                total_n_threads += clients[i].n_threads;

        double section_size = (UPPER_BOUND - LOWER_BOUND) / total_n_threads;
        strnum_t bounds[2];
        int first_thread_number = 0;
        for (int i = 0; i < n_clients; ++i) {
                snprintf(bounds[0], sizeof(bounds) - 1, "%f", first_thread_number * section_size);
                snprintf(bounds[1], sizeof(bounds) - 1, "%f",
                        (first_thread_number + clients[i].n_threads) * section_size);
                if (send(clients[i].sk, bounds, sizeof(bounds), 0) != sizeof(bounds))
                        ERROR("Can't send bounds(%s:%s)", bounds[0], bounds[1]);
                first_thread_number += clients[i].n_threads;
        }

        strnum_t str_sum = {};
        double sum = 0;
        errno = 0;
        int ret = 0;
        for (int i = 0; i < n_clients; ++i) {
                ret = recv(clients[i].sk, str_sum, sizeof(str_sum), 0);
                if (ret == -1)
                        ERROR("Can't receive sum from client[%d].", i);
                else if (ret == 0)
                        ERROR("Connection aborted.");

                sum += strtod(str_sum, NULL);
                if (errno != 0)
                        ERROR("Can't convert str_sum to double");
        }

        INFO("result is %lg.", sum);
        struct timeval time_finish;
        gettimeofday(&time_finish, NULL);
        long long time_ns = (time_finish.tv_sec - time_start.tv_sec)*1e6 + time_finish.tv_usec - time_start.tv_usec;
        double time = cast(double, time_ns) / 1e6;
        INFO("Computing time is %lg(sec).", time);

        for (int i = 0; i < n_clients; ++i)
                if (close(clients[i].sk) != 0)
                        ERROR("Can't close client(%d).", i);

        return 0;
}