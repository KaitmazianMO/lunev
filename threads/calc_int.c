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

static struct calc_context *make_contexts(unsigned n_threads, double from, double to,
                                          const struct cpu_conf *pconf);

static pthread_t *make_threads(unsigned n_threads, struct calc_context *cntxts,
                               const struct cpu_conf *pconf);

static void split_range_into_contexts(struct calc_context *contexts, int64_t n,
                                      double from, double to);

static void cpu_info_dump(struct cpu_conf *pconf);

double calc_int_in_n_hreads(unsigned n_threads, math_func f,
                            double from, double to, double dx)
{
        F = f;
        DX = dx;

        struct cpu_conf conf;
        if (get_cpu_conf(&conf) != 0)
                INFO("Failed to get cpu info. Using default info.");
        cpu_info_dump(&conf);

        const int n_workers = MAX(n_threads, conf.ncores);
        INFO("%d thread(s) will be launched.", n_workers);

        struct calc_context *contexts = make_contexts(n_threads, from, to, &conf);

        pthread_t *threads = make_threads(n_threads, contexts, &conf);

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

struct calc_context *make_contexts(unsigned n_threads, double from, double to,
                                   const struct cpu_conf *pconf)
{
        const unsigned n_workers = MAX(n_threads, pconf->ncores);
        struct calc_context *contexts =
                aligned_alloc(L1_CACHE_LINE_SIZE, n_workers * sizeof(contexts[0]));
        if (contexts == NULL)
                ERROR("Failed to allocate %u contexts.", n_workers);

        split_range_into_contexts(contexts, n_threads, from, to);
        /* Making extra load for linear scaling. */
        for (unsigned i = n_threads; i < pconf->ncores; ++i)
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

void thread_init(pthread_t *ptread, int idx, struct calc_context *cntxt,
                 const struct cpu_conf *pconf)
{
        pthread_attr_t attr;
        cpu_set_t cpu_set;
        int ht_idx = 0;
        if (pconf->threads_per_core == 2 && idx % pconf->ncores == 0)
                ht_idx = (idx / pconf->ncores) % 2;
        pthread_attr_init(&attr);
        CPU_ZERO(&cpu_set);
        CPU_SET(idx*pconf->threads_per_core % pconf->nthreads + ht_idx, &cpu_set);
        if (errno = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set))
                ERROR("Failed to set attr[%d] affinity.", idx);
        if (errno = pthread_create(ptread, &attr, calc_routine, cntxt))
                ERROR("Failed to create thread[%d].", idx);
        pthread_attr_destroy(&attr);       
}

pthread_t *make_threads(unsigned n_threads, struct calc_context *cntxts,
                               const struct cpu_conf *pconf)
{
        int n_workers = MAX(n_threads, pconf->ncores);
        pthread_t *threads = aligned_alloc(L1_CACHE_LINE_SIZE, n_workers * sizeof(*threads));
        if (threads == NULL)
                ERROR("Failed to allocate threads.");

        for (int i = 0; i < n_workers; ++i) {
                thread_init(threads + i, i, cntxts + i, pconf);
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

void cpu_info_dump(struct cpu_conf *pconf)
{
        CONF("threads:            %d", pconf->nthreads);
        CONF("cores:              %d", pconf->ncores);
        CONF("threads_per_core:   %d", pconf->threads_per_core);
        CONF("sockets:            %d", pconf->nsockets);
        CONF("L1_cache_line_size: %d", pconf->L1_cache_line_size);
}
