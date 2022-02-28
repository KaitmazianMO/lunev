#include <stdio.h>
#include <unistd.h>

#ifndef TEST_N_INTER
    #define TEST_N_INTER 1
#endif


#define TEST_N_ITER             $__test_n_iter
#define TEST_N_TESTS            $__test_n_test
#define TEST_REPORT_ARR         $__reports
#define TEST_REPORT_ARR_COUNTER $__i

#define TEST_HEAD(n)                    \
    char TEST_REPORT_ARR[n];            \
    int  TEST_REPORT_ARR_COUNTER = 0;   \
    const int TEST_N_TESTS = n;         \

#define __TEST_BEGIN(n_iters)            \
{                                        \
    { int TEST_N_ITER = n_iters;         \

#define __TEST_END(test)                                    \
    } while (--TEST_N_ITER > 0);                            \
    int i = 0;                                              \
    for (int i = 0; TEST_REPORT_ARR[i]; ++i) {              \
        fprintf (stderr, "%s\n", TEST_REPORT_ARR[i]);       \
    }                                                       \
    if (i == 0) {                                           \
        fprintf (stderr, "%s:%s: passed all %d tests.",     \
            __FUNCTION__, #test, TESTS_N_TESTS);            \
    } else {                                                \
        fprintf (stderr, "%s:%s: failed %d of %d tests.",   \
            __FUNCTION__, #test, TESTS_N_TESTS);            \
    }                                                       \
}

#define __TEST_QUIT break
#define __TEST_WRITE_REPORT(test, str) TEST_REPORT_ARR[TEST_REPORT_ARR_COUNTER++] = __FUNCTION__ ":" #test  ": "  str; TEST_QUIT       
#define __TEST_ADD_ERROR(test, str)    __TEST_WRITE_REPORT(str, test)
#define __TEST_EXPECT(test, cond, str)                                                      \
    if(!cond) {                                                                             \
        __TEST_WRITE_REPORT(test, "Expected condition(" #cond ") failed. " str);            \
        TEST_QUIT;                                                                          \
    }

#define __ASSERT_NOT_DEATH(test, action)                                            \
    pid_t child = fork();                                                           \
    __TEST_EXPECT(test, child != -1, "Fork failed");                                \
    if (child == 0) {                                                               \
        action;                                                                     \
        exit(TEST_PASSED);                                                          \
    }                                                                               \
    int status = 0;                                                                 \
    __TEST_EXPEXT(test, waitpid(child, &status, 0) != -1), "Waitpid failed.");      \
    if (WIFSIGNALED(status)) {                                                      \
        __TEST_ADD_ERROR(test, #action "precedes the fall");                        \
    }                                                                               \

#define __ASSERT_EQ(test, l, r)                 \
    __ASSERT_NOT_DEATH(test, l);                \
    __ASSERT_NOT_DEATH(test, r);                \
    if (l != r) {                               \
        __TEST_ADD_ERROR(test, #l "!= " #r);    \
    }

#define ASSERT_EQ(l, r)                     \
    __TEST_BEGIN(TEST_N_INTER);             \
    /*__ASSERT_EQ(ASSERT_EQ, l, r);*/           \
    __TEST_END(ASSERT_EQ);