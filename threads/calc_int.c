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
#include "calc_int.h"

#define EXPECTED_L1_CACHE_LINE_SIZE (64)
#define alignas(n) __attribute__ ((aligned (n)))
#define cache_aligned alignas(EXPECTED_L1_CACHE_LINE_SIZE)

#define FREE(ptr)       \
do {                    \
        free(ptr);      \
        ptr = NULL;     \
} while (0)

#define MAX(l, r) ((l) < (r) ? (r) : (l))
#define MIN(l, r) ((l) < (r) ? (l) : (r))

static long int L1_cache_line_size = 0;
static long int n_cores = 0;

static math_func F = NULL;
static double DX = 0;

static void *calc_routine(void *cacl_context_);
static void *extra_load(void *cacl_context_);

struct calc_context {
        double lower_bound, upper_bound;
        double sum;
} cache_aligned;

double calc_int_in_n_hreads(unsigned n_threads, math_func f, double from, double to, double dx)
{
        F = f;
        DX = dx;
        L1_cache_line_size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
        n_cores = get_nprocs_conf();
        CONF("ncores: %ld", n_cores);
        CONF("L1_cache_line_size: %ld", L1_cache_line_size);

        double sum = NAN;
        pthread_t *threads = NULL;
        cpu_set_t *sets = NULL;
        pthread_attr_t *attrs = NULL;
        struct calc_context *contexts = NULL;

        INFO("Configurating CPU_SETS.");
        sets = aligned_alloc(L1_cache_line_size, n_cores * sizeof(*sets));
        if (!sets) {
                ERROR("Failed to allocate sets");
        }
        for (int i = 0; i < n_cores; ++i) {
                CPU_ZERO(sets + i);
                CPU_SET(i, sets + i);
        }

        INFO("Initializing ATTRS");
        attrs = aligned_alloc(L1_cache_line_size, n_cores * sizeof(*attrs));
        if (!attrs) {
                ERROR("Failed to allocate attrs");
        }
        for (int i = 0; i < n_cores; ++i) {
                if (errno = pthread_attr_init(attrs + i))
                        ERROR("pthread_attr_init failed");
                if (errno = pthread_attr_setaffinity_np(attrs + i, sizeof(sets[0]), sets + i))
                        ERROR("pthread_attr_setaffinity_np failed");
        }

        INFO("Initializing CONTEXTS.");
        const int n_workers = MAX(n_threads, n_cores);
        INFO("n_workers = %d", n_workers);
        contexts = aligned_alloc(L1_cache_line_size, n_workers * sizeof(*contexts));
        if (!contexts) {
                ERROR("Failed to allocate contexts");
        }
        double section_size = (to - from) / n_threads;
        double section_lower = from;
        int i = 0;
        for (; i < n_threads; ++i) {
                contexts[i].lower_bound = section_lower;
                contexts[i].upper_bound = section_lower + section_size;
                section_lower += section_size;
        }
        section_size = (from - to) / n_threads;
        section_lower = from;
        for (; i < n_cores; ++i) {
                contexts[i] = contexts[i % n_threads];
        }

        INFO("Creating THREADS.");
        threads = aligned_alloc(L1_cache_line_size, n_workers * sizeof (*threads));
        if (!threads) {
                ERROR("Failed to allocate threads");
        }
        for (i = 0; i < n_threads; ++i) {
                if (errno = pthread_create(threads + i, attrs + i, calc_routine, contexts + i))
                        ERROR("pthread_create failed with THREAD[%d].", i);
        }
        for (; i < n_cores; ++i) {
                if (errno = pthread_create(threads + i, NULL, extra_load, contexts + i))
                        ERROR("pthread_create failed with THREAD[%d].", i);
        }

        for (int i = 0; i < n_cores; ++i) {
                if (errno = pthread_attr_destroy(attrs + i))
                        ERROR("pthread_attr_destroy failed");
        }

        INFO("Joining THREADS.");
        for (int i = 0; i < n_workers; ++i) {
                if (errno = pthread_join(threads[i], NULL))
                        ERROR("pthread_join failed with THREAD[%d].", i);
        }

        INFO("Calculating sum.");
        sum = 0;
        for (int i = 0; i < n_workers; ++i) {
                sum += contexts[i].sum;
        }

        goto cleanup;
cleanup:
        FREE(threads);
        FREE(sets);
        FREE(attrs);
        FREE(contexts);
        return sum;
}

void *calc_routine(void *cacl_context_)
{
        struct calc_context *cntx = (struct calc_context *)cacl_context_;
        double cache_aligned lower_bound = cntx->lower_bound;
        double cache_aligned upper_bound = cntx->upper_bound;
        double cache_aligned x = lower_bound;
        double cache_aligned sum = 0;
        while (x < upper_bound) {
                sum += F(x)*DX;
                x += DX;
        }
        cntx->sum = sum;
        return NULL;
}

void *extra_load(void *cacl_context_)
{
        struct calc_context *cntx = (struct calc_context *)cacl_context_;
        double cache_aligned lower_bound = cntx->lower_bound;
        double cache_aligned upper_bound = cntx->upper_bound;
        double cache_aligned x = lower_bound;
        double cache_aligned sum = 0;
        while (x < upper_bound) {
                sum += F(x)*DX;
                x += DX;
        }
        cntx->sum = 0;
        return NULL;
}