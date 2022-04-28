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

#define DX           1e-6
#define LOWER_BOUND  0
#define UPPER_BOUND  2e2

double f(double x) { return x*cos(atan(x)); }

int main(int argc, char *argv[])
{
        if (argc != 2)
                ERROR("Usage: %s <number-of-threads>", argv[0]);

        errno = 0;
        char *endptr = NULL;
        const int n_threads = strtol(argv[1], &endptr, 10);
        if (n_threads <= 0 || *endptr != '\0' || errno)
                ERROR("Invalid number format.");

        double sum = calc_int_in_n_hreads(n_threads, f, LOWER_BOUND, UPPER_BOUND, DX);
        printf("result = %lg.\n", sum);

        return 0;
}
