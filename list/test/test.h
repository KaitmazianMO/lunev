#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#define TEST_N_TESTS            $__test_n_test
#define TEST_REPORT_ARR         $__reports
#define TEST_REPORT_ARR_COUNTER $__i
#define TEST_N_TESTS_VERIFYIER  $__verifyier

#define TEST_HEAD(n)                    \
    char *TEST_REPORT_ARR[n];           \
    int  TEST_REPORT_ARR_COUNTER = 0;   \
    const int TEST_N_TESTS = n;         \
    int TEST_N_TESTS_VERIFYIER = 0

#define __TEST_BEGIN()                   \
++TEST_N_TESTS_VERIFYIER;                \
do {                                     

#define __TEST_END()                     \
} while(0)

#define TEST_SUMMARIZE()                                                                    \
    if (TEST_N_TESTS_VERIFYIER != TEST_N_TESTS) {                                           \
        fprintf(stderr, "Tests verifyer failed in %s.\n"                                    \
            "There is %d test\n", __func__, TEST_N_TESTS_VERIFYIER);                        \
        return;                                                                             \
    }                                                                                       \
    int i = 0;                                                                              \
    for (i = 0; i < TEST_REPORT_ARR_COUNTER; ++i) {                                         \
        fprintf(stderr, "%s:%s\n", __func__, TEST_REPORT_ARR[i]);                           \
    }                                                                                       \
    if (i == 0) {                                                                           \
        fprintf(stderr, "\x1b[32m" "%s: passed all %d tests.\n" "\x1b[0m",                  \
            __func__, TEST_N_TESTS);                                                        \
    } else {                                                                                \
        fprintf(stderr, "\x1b[31m" "%s: failed %d of %d tests.\n" "\x1b[0m",                \
            __func__, TEST_REPORT_ARR_COUNTER, TEST_N_TESTS);                               \
    }                                                                                       \


#define __TEST_QUIT break
#define __TEST_WRITE_REPORT(test, str) TEST_REPORT_ARR[TEST_REPORT_ARR_COUNTER++] = test  ":"  str; __TEST_QUIT       
#define __TEST_ADD_ERROR(test, str)    __TEST_WRITE_REPORT(test, str)
#define __TEST_EXPECT_NOT(test, cond, str)                                                    \
    if(cond) {                                                                                \
        __TEST_WRITE_REPORT(test, "Unexpected condition(" #cond ") happen. " str);            \
    }
    
#define __ASSERT_NOT_DEATH(test, action) {                                          \
    pid_t child = fork();                                                           \
    __TEST_EXPECT_NOT(test, child == -1, "Fork failed.");                           \
    if (child == 0) {                                                               \
        action;                                                                     \
        exit(EXIT_SUCCESS);                                                         \
    }                                                                               \
    int status = 0;                                                                 \
    __TEST_EXPECT_NOT(test, waitpid(child, &status, 0) == -1, "Waitpid failed.");   \
    if (WIFSIGNALED(status)) {                                                      \
        __TEST_ADD_ERROR(test, #action "precedes the fall.");                       \
    } else {                                                                        \
        action;                                                                     \
    }                                                                               \
}

#define __ASSERT_EQ(test, l, r)                           \
    __ASSERT_NOT_DEATH(test, l)                           \
    __ASSERT_NOT_DEATH(test, r)                           \
    if (l != r) {                                         \
        __TEST_ADD_ERROR(test, "Failed " #l " == " #r);   \
    }

#define ASSERT_EQ(l, r)                     \
    __TEST_BEGIN()                          \
    __ASSERT_EQ("ASSERT_EQ", l, r);         \
    __TEST_END()

#define EXPECT_NOT(cond, com)                   \
    __TEST_BEGIN()                              \
    __TEST_EXPECT_NOT("EXPECT_NOT", cond, com); \
    __TEST_END()

#define ASSERT_NOT_DEATH(action)                    \
    __TEST_BEGIN()                                  \
    __ASSERT_NOT_DEATH("ASSERT_NOT_DEATH", action); \
    __TEST_END()
