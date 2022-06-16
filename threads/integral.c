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

#include "integral.h"
#include "static_thread.h"
#include "conf.h"
#include "debug.h"
#include "cpu_conf.h"

#define MAX(l, r) ((l) < (r) ? (r) : (l))

static void *int_routine(void *integral_);

static void cpu_info_dump(struct cpu_conf *pconf);

static inline void swap_double(double *a, double *b)
{
        double tmp = *a;
        *b = *a;
        *a = tmp;
}

void int_init(struct Integral *integral, integrand_f f,
              struct Range *range, double step)
{
        assert(integral);
        assert(f);
        assert(range);
        assert(step);
        if (step < 0)
                step *= -1;
        integral->f = f;
        integral->range = *range;
        integral->step = step;
        integral->sum = 0;
        if (integral->range.to < integral->range.from)
                swap_double(&integral->range.from, &integral->range.to);
}

struct Integral *int_split_linearly(struct Integral *integral, unsigned n_parts, unsigned n_extra)
{
        assert(integral->range.from < integral->range.to);
        assert(n_parts > 0);

        unsigned n_total = n_parts + n_extra;
        struct Integral *parts = 
                aligned_alloc(L1_CACHE_LINE_SIZE, n_total * sizeof(parts[0]));
        if (parts) {
                double section_size = (integral->range.to - integral->range.from) / n_parts;
                assert(section_size > 0);
                for (unsigned i = 0; i < n_parts; ++i) {
                        parts->f = integral->f;
                        parts->step = integral->step;
                        parts->sum = 0;
                        parts->range = (struct Range) {
                                .from = section_size * i,
                                .to = section_size * (i + 1)
                        };
                }
        }
        return parts;
}

double int_accamulate(struct Integral *integral, unsigned n)
{
        double sum = 0;
        for (unsigned i = 0; i < n; ++i)
                sum += integral->sum;
        return sum;
}

int int_integrate(struct Integral *integral, unsigned factor)
{
        assert(integral->f);
        assert(integral->range.from < integral->range.to);
        int ret = 0;
        /* To safe free with non-allocated data. */
        struct Integral *integral_parts = NULL;
        struct StaticThread *threads = NULL;

        struct cpu_conf conf;
        if (get_cpu_conf(&conf) != 0)
                INFO("Failed to get cpu info. Using default info.");
        cpu_info_dump(&conf);

        const unsigned n_workers = factor;
        const unsigned n_threads = MAX(n_workers, conf.nthreads);
        const unsigned n_extra = (n_threads > n_workers) ? (n_threads - n_workers)
                                                         : 0;
        integral_parts = int_split_linearly(integral, n_workers, n_extra);
        if (integral_parts == NULL) {
                INFO("Failed to alloc integral parts.");
                ret = -1;
                goto cleanup;
        }
        /* Making additional load for extra threads. */
        for (int i = n_workers; i < n_threads; ++i)
                integral_parts[i] = integral_parts[i % n_workers];

        threads = calloc(n_threads, sizeof(threads[0]));
        if (threads == NULL) {
                INFO("Faled to alloc threads.");
                ret = -1;
                goto cleanup;
        }
        int ht_idx = 0;
        for (int i = 0; i < n_threads; ++i) {
                if (i % conf.ncores == 0)
                        ht_idx = (i / conf.ncores) % conf.threads_per_core;
                int thread_place = (i*conf.threads_per_core + ht_idx) % conf.nthreads;
                if (static_thread_create(threads + i, thread_place, 
                    int_routine, integral_parts) != 0) {
                        INFO("Failed to create thread[%d].", i);
                        ret = -1;
                        goto cleanup;
                }
        }

        for (int i = 0; i < n_workers; ++i) {
                if (static_thread_join(threads + i, NULL)) {
                        INFO("Failed to join thread[%d]", i);
                        ret = -1;
                        goto cleanup;
                }
        }

        integral->sum = int_accamulate(integral_parts, n_workers);

cleanup:
        free(threads);
        free(integral_parts);
        return ret;
}


static void integrate_on_current_thread(struct Integral *integral)
{
        double sum = 0;
        double from = integral->range.from;
        double to = integral->range.to;
        double dx = integral->step;
        integrand_f f = integral->f;
        for (double x = from; x < to; x += dx)
                sum += f(x)*dx;

        integral->sum = sum;
}

void *int_routine(void *integral_)
{
        struct Integral *integral = (struct Integral *)integral_;
        integrate_on_current_thread(integral);
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
