#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "../src/list.h"
#include "../src/list_impl.h"

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

int N_ERRORS = 0;
int EXECUTABLE_TEST_FAILED = 0;


#pragma GCC diagnostic ignored "-Wformat-zero-length"
#define ASSERT_EQUAL(L, R, ...) \
    if (L != R) {   \
        fprintf(stderr, "%s: ASSERTION EQUAL FAILED %s != %s\n", __FUNCTION__, #L, #R);    \
        if (fprintf(stderr, __VA_ARGS__) != 0) fprintf(stderr, "\n");  \
        N_ERRORS++; \
        EXECUTABLE_TEST_FAILED = 1;   \
    }

#define ONE_ARG_FUNCTION_ASSERT_EQUAL(L, func, arg1)  \
    if (L != func(arg1)) {   \
        fprintf(stderr, "%s: ASSERTION EQUAL FAILED %s(%s) != %s\n", __FUNCTION__, #func, #arg1, #L);    \
        fprintf(stderr, "Function %s works bad with arg %s\n", #func, #arg1);    \
        N_ERRORS++; \
        EXECUTABLE_TEST_FAILED = 1;   \
    }

#define TWO_ARG_FUNCTION_ASSERT_EQUAL(L, func, arg1, arg2, ...)  \
    if (L != func(arg1, arg2)) {   \
        fprintf(stderr, "%s: ASSERTION EQUAL FAILED %s(%s, %s) != %s\n", __FUNCTION__, #func, #arg1, #arg2, #L);    \
        fprintf(stderr, "Function %s works bad with args %s and %s\n", #func, #arg1, #arg2);    \
        if (fprintf(stderr, __VA_ARGS__) != 0) fprintf(stderr, "\n");    \
        N_ERRORS++; \
        EXECUTABLE_TEST_FAILED = 1;   \
    }

void run_tests();
int initialized_empty_list_test();
int null_argument_test();

#define RUN_TEST(test_func) {\
    N_ERRORS = 0;   \
    int test_res = test_func(); \
    if (test_res == TEST_PASSED || test_res == TEST_ENDED_PREMATURELY) {   \
        fprintf(stderr, COLOR_GREEN "Passed %s\n" COLOR_RESET, #test_func);    \
    } else {    \
        fprintf(stderr, COLOR_RED "Failed %s(%d)\n" COLOR_RESET, #test_func, N_ERRORS);    \
    }   \
}

int main() {
    run_tests();

}

void run_tests() {
    RUN_TEST(initialized_empty_list_test);
    RUN_TEST(null_argument_test);
}


int initialized_empty_list_test() {
    EXECUTABLE_TEST_FAILED = 0;

    struct list *list = list_create();
    if (!list) {
        return TEST_ENDED_PREMATURELY;
    }
    
    {
        ASSERT_EQUAL(list->size, 0, "Initial size is not zero");
        ASSERT_EQUAL(list->ghost.next, &list->ghost, "Ghost element format is not respected");
        ASSERT_EQUAL(list->ghost.prev, &list->ghost, "Ghost element format is not respected");
    }
    {
        ONE_ARG_FUNCTION_ASSERT_EQUAL(list->size,   list_size, list);
        ONE_ARG_FUNCTION_ASSERT_EQUAL(&list->ghost, list_head, list);
        ONE_ARG_FUNCTION_ASSERT_EQUAL(&list->ghost, list_tail, list);
    }
    {
        ONE_ARG_FUNCTION_ASSERT_EQUAL(&list->ghost,     node_next, &list->ghost);
        ONE_ARG_FUNCTION_ASSERT_EQUAL(&list->ghost,     node_prev, &list->ghost);
        ONE_ARG_FUNCTION_ASSERT_EQUAL(list->ghost.data, node_data, &list->ghost);
        void *data = (void *)7;
        node_set_data(&list->ghost, data);
        ASSERT_EQUAL(data, node_data(&list->ghost), "Function node_set works bad");
    }

    list_destroy(list);
    return EXECUTABLE_TEST_FAILED ? TEST_FAILED : TEST_PASSED;
}

#define INCORRECT_ASSERT_EQUAL_TEST(L, R)   \
    switch (fork()) {  \
        case -1: {  \
            return TEST_ENDED_PREMATURELY;  \
        }   \
        case 0: {   \
                \
        }   \
    }

int null_argument_test() {
    //node_set_data(NULL, NULL);
    
    ASSERT_EQUAL(node_data(NULL), NULL, "");
    ASSERT_EQUAL(node_next(NULL), NULL, "");
    ASSERT_EQUAL(node_prev(NULL), NULL, "");

    // list_destroy(NULL);

    ASSERT_EQUAL(list_size(NULL), 0, "");
    ASSERT_EQUAL(list_head(NULL), NULL, "");
    ASSERT_EQUAL(list_tail(NULL), NULL, "");

    // @todo: not NULL list tests
    ASSERT_EQUAL(list_insert_front(NULL, NULL), NULL, "");
    ASSERT_EQUAL(list_insert_back(NULL, NULL), NULL, "");
    ASSERT_EQUAL(list_insert_after(NULL, NULL, NULL), NULL, "");
    ASSERT_EQUAL(list_insert_before(NULL, NULL, NULL), NULL, "");

    ASSERT_EQUAL(list_erase(NULL, NULL, NULL), NULL, "");

    ASSERT_EQUAL(list_for_each(NULL, NULL, NULL), 0, "");

    ASSERT_EQUAL(list_set_allocator(NULL), calloc, "");

    return EXECUTABLE_TEST_FAILED ? TEST_FAILED : TEST_PASSED;
}