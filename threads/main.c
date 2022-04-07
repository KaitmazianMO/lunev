#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include "debug.h"

#define MAX_SIZE (1 << 20)
static pthread_t THREADS[MAX_SIZE];
static cpu_set_t CPU_SETS[MAX_SIZE];
static struct calc_context {
        double lower_bound, upper_bound;
        double sum;
} CONTEXTS[MAX_SIZE];

#define DX           1e-4
#define LOWER_BOUND  0
#define UPPER_BOUND  1e5
#define FUNC(x)      (pow(x, 1/x)*acos(sin(x)))

static void *calc_routine(void *cacl_context_);

int main(int argc, char *argv[])
{
        if (argc != 2)
                ERROR("Usage: %s <number-of-threads>", argv[0]);

        errno = 0;
        char *endptr = NULL;
        const int n_threads = strtol(argv[1], &endptr, 10);
        if (n_threads <= 0 || *endptr != '\0' || errno)
                ERROR("Invalid number format.");

        if (MAX_SIZE < n_threads)
                ERROR("Too many threads, the max number is %d.", MAX_SIZE);

        INFO("Configurating CPU_SETS.");
        const int n_cores = get_nprocs_conf();
        for (int i = 0; i < n_threads; ++i) {
                CPU_ZERO(CPU_SETS + i);
                CPU_SET(i % n_cores, CPU_SETS + i);
        }

        INFO("Initializing CONTEXTS.");
        double section_size = (UPPER_BOUND - LOWER_BOUND) / n_threads;
        double section_lower = LOWER_BOUND;
        for (int i = 0; i < n_threads; ++i) {
                CONTEXTS[i].lower_bound = section_lower;
                CONTEXTS[i].upper_bound = section_lower + section_size;
                section_lower += section_size;
        }

        INFO("Creating THREADS.");
        for (int i = 0; i < n_threads; ++i) {
                if (pthread_create(THREADS + i, NULL, calc_routine, CONTEXTS + i))
                        ERROR("pthread_create failed with THREAD[%d].", i);
                if (pthread_setaffinity_np(THREADS[i], sizeof(CPU_SETS[0]), CPU_SETS + i)) {
                        ERROR("pthread_setaffinity_np failed with THREAD[%d].", i);
                }
        }

        INFO("Joining THREADS.");
        for (int i = 0; i < n_threads; ++i) {
                if (pthread_join(THREADS[i], NULL))
                        ERROR("pthread_join failed with THREAD[%d].", i);
        }

        INFO("Calculating sum.");
        double sum = 0;
        for (int i = 0; i < n_threads; ++i) {
                sum += CONTEXTS[i].sum;
        }

        printf ("sum = %lf\n", sum);
        return 0;
}

void *calc_routine(void *cacl_context_)
{
        struct calc_context *cntx = (struct calc_context *)cacl_context_;
        double lower_bound = cntx->lower_bound;
        double upper_bound = cntx->upper_bound;
        double x = lower_bound;
        double sum = 0;
        while (x < upper_bound) {
                sum += FUNC(x)*DX;
                x += DX;
        }
        cntx->sum = sum;
}