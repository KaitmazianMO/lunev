#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef int (*debug_printer_f)(FILE *file, ...);
static debug_printer_f __attribute__((unused)) debug_printer;

#ifdef MY_DEBUG
#define INFO(...)                                               \
do {                                                            \
        if (debug_printer) {                                    \
                fprintf(stderr, __VA_ARGS__);                   \
                fprintf(stderr, "\n");                          \
        }                                                       \
} while (0)
#else
#define INFO(...) (void)0;
#endif

#define ERROR(...)                                                      \
do {                                                                    \
        fprintf(stderr, __VA_ARGS__);                                   \
        fprintf(stderr, "\n");                                          \
        fprintf(stderr, "errno(%d): %s", errno, strerror(errno));       \
        fprintf(stderr, "\n");                                          \
        exit(EXIT_FAILURE);                                             \
} while (0)

#define CONF(...)                                               \
do {                                                            \
        INFO("Conf: " __VA_ARGS__);                             \
} while(0)

#endif // DEBUG_INCLUDED