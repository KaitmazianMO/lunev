#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TEST_FAILED             0
#define TEST_PASSED             1
#define TEST_ENDED_PREMATURELY  2

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

#define COLOR_OK       COLOR_GREEN
#define COLOR_ERROR    COLOR_RED
#define COLOR_SEGFAULT COLOR_MAGENTA

int N_ERRORS = 0;
int EXECUTABLE_TEST_FAILED = 0;

#define PRINT(...)               fprintf(stderr, __VA_ARGS__)
#define PRINT_OK(fmt, ...)       fprintf(stderr, COLOR_OK fmt COLOR_OK, __VA_ARGS__)
#define PRINT_ERROR(fmt, ...)    fprintf(stderr, COLOR_ERROR fmt COLOR_RESET, __VA_ARGS__)
#define PRINT_SEGFAULT(fmt, ...) fprintf(stderr, COLOR_SEGFAULT fmt COLOR_RESET, __VA_ARGS__)

#pragma GCC diagnostic ignored "-Wformat-zero-length"
#define ASSERT_EQUAL(L, R, ...) \
    if (L != R) {   \
        PRINT_ERROR("%s: ASSERTION EQUAL FAILED %s != %s\n", __FUNCTION__, #L, #R);    \
        if (PRINT(__VA_ARGS__) != 0) PRINT("\n");  \
        N_ERRORS++; \
        EXECUTABLE_TEST_FAILED = 1;   \
    }

#define ONE_ARG_FUNCTION_ASSERT_EQUAL(L, func, arg1)  \
    if (L != func(arg1)) {   \
        PRINT_ERROR("%s: ASSERTION EQUAL FAILED %s(%s) != %s\n", __FUNCTION__, #func, #arg1, #L);    \
        PRINT_ERROR("Function %s works bad with arg %s\n", #func, #arg1);    \
        N_ERRORS++; \
        EXECUTABLE_TEST_FAILED = 1;   \
    }

#define TWO_ARG_FUNCTION_ASSERT_EQUAL(L, func, arg1, arg2, ...)  \
    if (L != func(arg1, arg2)) {   \
        PRINT_ERROR("%s: ASSERTION EQUAL FAILED %s(%s, %s) != %s\n", __FUNCTION__, #func, #arg1, #arg2, #L);    \
        PRINT_ERROR("Function %s works bad with args %s and %s\n", #func, #arg1, #arg2);    \
        if (PRINT(__VA_ARGS__) != 0) PRINT("\n");    \
        N_ERRORS++; \
        EXECUTABLE_TEST_FAILED = 1;   \
    }

#define RUN_TEST(test_func) {\
    N_ERRORS = 0;   \
    int test_res = test_func(); \
    if (test_res == TEST_PASSED || test_res == TEST_ENDED_PREMATURELY) {   \
        PRINT_OK("Passed %s\n", #test_func);    \
    } else {    \
        PRINT_ERROR("Failed %s(%d)\n", #test_func, N_ERRORS);    \
    }   \
}

#define ASSERT_EQUAL_WITHOUT_DEATH(L, R) {  \
    pid_t child = fork(); \
    switch (child) {  \
        case -1: {  \
            return TEST_ENDED_PREMATURELY;  \
        }   \
        case 0: {   \
            if (L != R) {   \
                exit(TEST_FAILED);    \
            }   \
            exit(TEST_PASSED);  \
        } break;  \
        int status = 0; \
        waitpid(child, &status, 0);   \
        if (WIFSIGNALED(status)) {   \
            PRINT_SEGFAULT("ASSERTION EQUAL FAILED with segfault (%s == %s)\n", #L, #R);    \
            N_ERRORS++; \
        } else if (WEXITSTATUS(status) == TEST_FAILED) { \
            N_ERRORS++; \
            PRINT_ERROR("ASSERTION EQUAL FAILED %s != %s\n", #L, #R);   \
        }   \
    }   \
}
