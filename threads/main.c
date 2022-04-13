#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sched.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include "debug.h"

#define PAGE_SIZE         4096
#define L1CACHE_LINE_SIZE 64

#define alignas(n) __attribute__ ((aligned (n)))
#define cache_aligned alignas(L1CACHE_LINE_SIZE)

#define MAX_SIZE (1 << 10)
static pthread_t cache_aligned THREADS[MAX_SIZE];
static cpu_set_t cache_aligned CPU_SETS[MAX_SIZE];
static pthread_attr_t cache_aligned ATTRS[MAX_SIZE];
static struct calc_context {
        double lower_bound, upper_bound;
        double sum;
} cache_aligned CONTEXTS[MAX_SIZE];

#define DX           1e-4
#define LOWER_BOUND  0
#define UPPER_BOUND  1e4
#define FUNC(x)      (pow(x, 1/x)*acos(sin(x)))

static void *calc_routine(void *cacl_context_);

int main(int argc, char *argv[])
{
        if (argc != 2)
                ERROR("Usage: %s <number-of-threads>", argv[0]);

        INFO("==================================================");

        errno = 0;
        char *endptr = NULL;
        const int n_threads = strtol(argv[1], &endptr, 10);
        if (n_threads <= 0 || *endptr != '\0' || errno)
                ERROR("Invalid number format.");

        if (MAX_SIZE < n_threads)
                ERROR("Too many threads, the max number is %d.", MAX_SIZE);

        const long int n_cores = get_nprocs_conf();
        CONF("ncores: %ld", n_cores);
        CONF("L1cache_size: %ld", sysconf(_SC_LEVEL1_DCACHE_LINESIZE));

        INFO("Configurating CPU_SETS.");
        cpu_set_t all_seted;
        CPU_ZERO(&all_seted);
        for (int i = 0; i < n_cores; ++i) {
                CPU_SET(i, &all_seted);
        }
        for (int i = 0; i < n_threads; ++i) {
                CPU_ZERO(CPU_SETS + i);
                if (i < n_threads)
                        CPU_SET(i, CPU_SETS + i);
                else
                        CPU_AND(CPU_SETS + i, &all_seted, CPU_SETS + i);
        }

        INFO("Initializing ATTRS");
        for (int i = 0; i < n_threads; ++i) {
                pthread_attr_init(ATTRS + i);
                pthread_attr_setaffinity_np(ATTRS + i, sizeof(CPU_SETS[0]), CPU_SETS + i);
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
                if (pthread_create(THREADS + i, ATTRS + i, calc_routine, CONTEXTS + i))
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

        INFO("Calcualted sum is %lg.", sum);

        INFO("==================================================");

        return 0;
}

void *calc_routine(void *cacl_context_)
{
        struct calc_context *cntx = (struct calc_context *)cacl_context_;
        double cache_aligned lower_bound = cntx->lower_bound;
        double cache_aligned upper_bound = cntx->upper_bound;
        double cache_aligned x = lower_bound;
        double cache_aligned sum = 0;
        while (x < upper_bound) {
                sum += FUNC(x)*DX;
                x += DX;
        }
        cntx->sum = sum;
        return NULL;
}