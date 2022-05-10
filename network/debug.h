#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>

#define ERROR(...) do {                                 \
        error_at_line(EXIT_FAILURE, errno,              \
                __FILE__, __LINE__, __VA_ARGS__);       \
} while (0)


#define INFO(...) do {                                  \
        fprintf(stderr, __VA_ARGS__);                   \
        fprintf(stderr, "\n");                          \
} while (0)

#endif // DEBUG_H_INCLUDED