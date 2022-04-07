#ifndef DEBUG_INCLUDED
#define DEBUG_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#define INFO(...)                                               \
do {                                                            \
        fprintf(stderr, __VA_ARGS__);                           \
        fprintf(stderr, "\n");                                  \
} while (0)

#define ERROR(...)                                              \
do {                                                            \
        INFO(__VA_ARGS__);                                      \
        exit(EXIT_FAILURE);                                     \
} while (0)

#endif // DEBUG_INCLUDED