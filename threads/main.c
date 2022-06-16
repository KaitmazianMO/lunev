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
#include "integral.h"

#define DX           1e-4
#define LOWER_BOUND  0
#define UPPER_BOUND  2e4

double f(double x) { return x*cos(atan(x)); }

int main(int argc, char *argv[])
{
        if (argc != 2)
                ERROR("Usage: %s <factor>", argv[0]);

        errno = 0;
        char *endptr = NULL;
        const int factor = strtol(argv[1], &endptr, 10);
        if (factor <= 0 || *endptr != '\0' || errno)
                ERROR("Invalid number format.");

        struct Integral integral = {};
        struct Range range = { .from = LOWER_BOUND, .to = UPPER_BOUND};
        int_init(&integral, f, &range, DX);

        int ret = int_integrate(&integral, factor);
        if (ret == -1) {
                ERROR("Can't calculate integral sum.");
        }

        printf("result = %lg.\n", integral.sum);

        return 0;
}
