#ifndef CALC_INT_INCLUDED
#define CALC_INT_INCLUDED

#include "conf.h"

typedef double (*integrand_f)(double x);

struct Range {
        double from;
        double to;
};

struct Integral {
        integrand_f f;
        struct Range range;
        double step;
        double sum;
} __attribute__ ((aligned (L1_CACHE_LINE_SIZE)));

void int_init(struct Integral *integral, integrand_f f, struct Range *range, double step);

/* Calculate integral sum using constant time divided by factor. */
int int_integrate(struct Integral *integral, unsigned factor);

/* Split the integral linearly. Returns array of integral parts of size n_parts + n_extra. */
struct Integral *int_split_linearly(struct Integral *integral, unsigned n_parts, unsigned n_extra);

double int_accamulate(struct Integral *integral, unsigned n);

#endif // CALC_INT_INCLUDED