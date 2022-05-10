#ifndef CALC_INT_INCLUDED
#define CALC_INT_INCLUDED

typedef double (*math_func)(double x);

double calc_int_in_n_hreads(unsigned n, math_func f, double from, double to, double dx);

#endif // CALC_INT_INCLUDED