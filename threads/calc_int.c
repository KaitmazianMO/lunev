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

#define L1_CACHE_LINE_SIZE (64)
#define alignas(n) __attribute__ ((aligned (n)))

#define MAX(l, r) ((l) < (r) ? (r) : (l))

struct calc_context {
        double lower_bound, upper_bound;
        double sum;
} alignas(L1_CACHE_LINE_SIZE);

static math_func F = NULL;
static double DX = 0;

static void *calc_routine(void *cacl_context_);

static struct calc_context *make_contexts(unsigned n_threads, unsigned n_cores,
                                          double from, double to);

static pthread_t *make_threads(unsigned n_threads, unsigned n_cores,
                        unsigned threads_per_core, struct calc_context *cntxts);

static void split_range_into_contexts(struct calc_context *contexts, int64_t n,
                                      double from, double to);

double calc_int_in_n_hreads(unsigned n_threads, math_func f,
                            double from, double to, double dx)
{
        F = f;
        DX = dx;

        struct cpu_conf conf;
        if (get_cpu_conf(&conf) != 0) {
                INFO("Failed to get cpu info.");
                conf.threads = get_nprocs();
#ifndef HT
                conf.cores = conf.threads / 2;
#else
                conf.cores = conf.threads;
#endif
        }
        long threads_per_core = conf.threads / conf.cores;
        CONF("threads:            %ld", conf.threads);
        CONF("cores:              %ld", conf.cores);
        CONF("threads_per_core:   %ld", threads_per_core);
        CONF("sockets:            %ld", conf.sockets);
        CONF("L1_cache_line_size: %ld", conf.L1_cache_line_size);

        if (conf.threads < n_threads)
                n_threads = conf.threads;
        const int n_workers = MAX(n_threads, conf.cores);
        INFO("n_workers = %d", n_workers);

        INFO("Making contexts.");
        struct calc_context *contexts = make_contexts(n_threads, conf.cores, from, to);

        INFO("Making threads.");
        pthread_t *threads = make_threads(n_threads, conf.cores, threads_per_core, contexts);

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

struct calc_context *make_contexts(unsigned n_threads, unsigned n_cores,
                                   double from, double to)
{
        const unsigned n_workers = MAX(n_threads, n_cores);
        struct calc_context *contexts =
                aligned_alloc(L1_CACHE_LINE_SIZE, n_workers * sizeof(contexts[0]));
        if (contexts == NULL)
                ERROR("Failed to allocate %u contexts.", n_workers);

        split_range_into_contexts(contexts, n_threads, from, to);
        for (unsigned i = n_threads; i < n_cores; ++i)
                contexts[i] = contexts[i % n_threads];
        return contexts;
}

void split_range_into_contexts(struct calc_context *contexts,
                               int64_t n, double from, double to)
{
        double section_size = (to - from) / n;
        double section_lower = from;
        for (int i = 0; i < n; ++i) {
                contexts[i].lower_bound = section_lower;
                contexts[i].upper_bound = section_lower + section_size;
                section_lower += section_size;
        }
}

pthread_t *make_threads(unsigned n_threads, unsigned n_cores,
                        unsigned threads_per_core, struct calc_context *cntxts)
{
        int n_workers = MAX(n_threads, n_cores);
        pthread_t *threads = aligned_alloc(L1_CACHE_LINE_SIZE, n_workers * sizeof(*threads));
        if (threads == NULL)
                ERROR("Failed to allocate threads.");

        const unsigned n_logical_cores = n_cores * threads_per_core;
        pthread_attr_t attr;
        cpu_set_t cpu_set;
        int ht_step = 0;
        for (int i = 0; i < n_workers; ++i) {
                if (threads_per_core == 2 && n_cores <= i)
                        ht_step = 1;
                pthread_attr_init(&attr);
                CPU_ZERO(&cpu_set);
                CPU_SET(i*threads_per_core % n_logical_cores + ht_step, &cpu_set);
                if (errno = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set))
                        ERROR("Failed to set attr[%d] affinity.", i);
                if (errno = pthread_create(threads + i, &attr, calc_routine, cntxts + i))
                        ERROR("Failed to create thread[%d].", i);
                pthread_attr_destroy(&attr);
        }
        return threads;
}

static double calc_int_on_range(double from, double to)
{
        double sum = 0;
        for (double x = from; x < to; x += DX)
                sum += F(x)*DX;

        return sum;
}

void *calc_routine(void *cacl_context_)
{
        struct calc_context *cntx = (struct calc_context *)cacl_context_;
        double lower_bound = cntx->lower_bound;
        double upper_bound = cntx->upper_bound;
        cntx->sum = calc_int_on_range(lower_bound, upper_bound);
        return NULL;
}
