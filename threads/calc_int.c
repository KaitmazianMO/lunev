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
#include "cpu_conf.h"

#define L1_CAHE_LINE_SIZE (64)
#define alignas(n) __attribute__ ((aligned (n)))

#define MAX(l, r) ((l) < (r) ? (r) : (l))

struct calc_context {
        double lower_bound, upper_bound;
        double sum;
} alignas(L1_CAHE_LINE_SIZE);

static math_func F = NULL;
static double DX = 0;

static void *calc_routine(void *cacl_context_);

static void split_range_into_contexts(struct calc_context *contexts, unsigned n, double from, double to);

static pthread_t *make_threads_and_place_on_cores(unsigned n_threads, unsigned n_cores,
                                           unsigned threads_per_core, struct calc_context *cntxts);

double calc_int_in_n_hreads(unsigned n_threads, math_func f, double from, double to, double dx)
{
        F = f;
        DX = dx;
        
        struct cpu_conf conf;
        if (get_cpu_conf(&conf) != 0)
                ERROR("Failed to get cpu info.");

        long threads_per_core = conf.threads / conf.cores;
        CONF("threads:            %ld", conf.threads);
        CONF("cores:              %ld", conf.cores);
        CONF("threads_per_core:   %ld", threads_per_core);
        CONF("sockets:            %ld", conf.sockets);
        CONF("L1_cache_line_size: %ld", conf.L1_cache_line_size);

        const int n_workers = MAX(n_threads, conf.cores);
        INFO("n_workers = %d", n_workers);

        INFO("Making contexts.");
        struct calc_context *contexts = aligned_alloc(conf.L1_cache_line_size, n_workers * sizeof(*contexts));
        split_range_into_contexts(contexts, n_threads, from, to);
        /* contexts for extra load */
        for (int i = n_threads; i < conf.cores; ++i) {
                contexts[i] = contexts[i % n_threads];
        }  

        INFO("Making threads.");
        pthread_t *threads = make_threads_and_place_on_cores(n_threads, conf.cores, threads_per_core, contexts);
        for (int i = 0; i < n_workers; ++i) {
                if (errno = pthread_join(threads[i], NULL))
                        ERROR("Failed to join THREAD[%d].", i);
        }

        double sum = 0;
        for (int i = 0; i < n_threads; ++i) {
                sum += contexts[i].sum;
        }

        free(contexts);        
        free(threads);
        return sum;
}

void split_range_into_contexts(struct calc_context *contexts, unsigned n, double from, double to)
{
        double section_size = (to - from) / n;
        double section_lower = from;
        for (int i = 0; i < n; ++i) {
                contexts[i].lower_bound = section_lower;
                contexts[i].upper_bound = section_lower + section_size;
                section_lower += section_size;
        }
}

pthread_t *make_threads_and_place_on_cores(unsigned n_threads, unsigned n_cores, 
                                           unsigned threads_per_core, struct calc_context *cntxts)
{
        int n_workers = MAX(n_threads, n_cores);
        pthread_t *threads = aligned_alloc(L1_CAHE_LINE_SIZE, n_workers * sizeof(*threads));
        if (threads == NULL)
                ERROR("Failed to allocate threads.");

        int logical_cores = threads_per_core * n_cores;
        for (int i = 0; i < n_workers; ++i) {
                if (errno = pthread_create(threads + i, NULL, calc_routine, cntxts + i))
                        ERROR("Failed to create thread[%d].", i);
                if (i < n_threads) {
                        cpu_set_t set;
                        CPU_ZERO(&set);
                        CPU_SET(i*threads_per_core % logical_cores, &set);
                        if (errno = pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &set))
                                ERROR("Failed to set thread[%d] affinity.", i);
                }
        }
        return threads;
}

void *calc_routine(void *cacl_context_)
{
        struct calc_context *cntx = (struct calc_context *)cacl_context_;
        double lower_bound = cntx->lower_bound;
        double upper_bound = cntx->upper_bound;
        double sum = 0;
        for (double x = lower_bound; x < upper_bound; x += DX)
                sum += F(x)*DX;
        cntx->sum = sum;
        return NULL;
}