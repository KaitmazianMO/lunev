#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "threads/calc_int.h"
#include "config.h"
#include "common.h"
#include "utils.h"
#include "debug.h"

static void start_client_loop(counter_t n_threads);
static void run_client(counter_t n_threads);

int main(int argc, char *argv[])
{
        if (argc != 2)
                ERROR("Usage: %s <number-of-threads>", argv[0]);

        errno = 0;
        const counter_t n_threads = atoi(argv[1]);
        if (errno != 0 || n_threads <= 0)
                ERROR("Usage: %s <number-of-threads>", argv[0]);

        start_client_loop(n_threads);

        return 0;
}

static void start_client_loop(counter_t n_threads)
{
        for (;;) {
                INFO("Waiting for a new server.");
                run_client(n_threads);
        }
}

static void run_client(counter_t n_threads)
{
        int broadcast_sk = socket(PF_INET, SOCK_DGRAM, 0);
        if (broadcast_sk == -1)
                ERROR("no broadcast sk");

        if (setsockopt_no_val(broadcast_sk, SOL_SOCKET, SO_REUSEADDR) != 0)
                ERROR("Can't set reuseaddr opt to broadcast sk");

        struct sockaddr_in broadcast_addr;
        init_sockaddr_in(&broadcast_addr, htons(CLIENT_PORT), htonl(INADDR_ANY));
        if (bind(broadcast_sk, sockaddr_cast(&broadcast_addr), sizeof(broadcast_addr)) != 0)
                ERROR("Can't bind broadcast socket");

        char broadcast_msg[sizeof(BROADCAST_MSG)] = {};
        struct sockaddr_in remote_addr = {};
        socklen_t remote_addr_size = sizeof(remote_addr);
        if (recvfrom(broadcast_sk, broadcast_msg, sizeof(BROADCAST_MSG), 0,
            sockaddr_cast(&remote_addr), &remote_addr_size) != sizeof(BROADCAST_MSG))
                ERROR("Can't receive broadcast message");

        if (close(broadcast_sk) != 0)
                ERROR("Can't close broadcast sock");

        if (strcmp(BROADCAST_MSG, broadcast_msg))
                ERROR("Incorrect broadcast message(%s)", broadcast_msg);

        struct sockaddr_in serv_addr;
        init_sockaddr_in(&serv_addr, htons(SERVER_PORT), remote_addr.sin_addr.s_addr);

        int server_sk = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_sk == -1)
                ERROR("no client sk");

        if (connect(server_sk, sockaddr_cast(&serv_addr), sizeof(serv_addr)) != 0)
                ERROR("Can't connect to the server");

        INFO("Found server.");
        counter_t net_n_threads = htons(n_threads);
        if (send(server_sk, &net_n_threads, sizeof(net_n_threads), 0) != sizeof(net_n_threads))
                ERROR("Can't send nthreads(%d)", n_threads);

        strnum_t bounds[2];
        if (recv(server_sk, bounds, sizeof (bounds), 0) != sizeof (bounds))
                ERROR("Failed to receive bounds");

        INFO("Got bounds from %s to %s.", bounds[0], bounds[1]);
        errno = 0;
        double lower_bound = strtod(bounds[0], NULL);
        double upper_bound = strtod(bounds[1], NULL);
        if (errno != 0)
                ERROR("Can't convert string bounds to numbers.(%s,%s)", bounds[0], bounds[1]);

        double sum = calc_int_in_n_hreads(n_threads, f, lower_bound, upper_bound, DX);
        if (sum != sum)
                ERROR("Calculating failed");

        struct timeval send_timeout = {
                .tv_sec = CLIENT_SEND_TIMEOUT_SEC,
                .tv_usec = CLIENT_SEND_TIMEOUT_USEC
        };
        if (setsockopt(server_sk, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(send_timeout)) != 0)
                ERROR("Can't set send timeout for server socket");

        strnum_t sum_str = {};
        snprintf(sum_str, sizeof(sum_str) - 1, "%f", sum);
        INFO("Sending result %s.", sum_str);
        if (send(server_sk, &sum_str, sizeof(sum_str), 0) != sizeof(sum_str))
                ERROR("Can't send sum(%s) to the server", sum_str);

        if (close(server_sk) != 0)
                ERROR("Can't close server socket");

}