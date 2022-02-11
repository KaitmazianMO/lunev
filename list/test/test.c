#include "test.h"
#include "../src/list.h"
#include "../src/list_impl.h"

void run_tests();
int initialized_empty_list_test();
int null_argument_test();

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

#define ASSERT_NOT_DEATH(call) {  \
    pid_t child = fork(); \
    switch (child) {  \
        case -1: {  \
            return TEST_ENDED_PREMATURELY;  \
        }   \
        case 0: {    \
            call; \
            exit(TEST_PASSED);  \
        } break;    \
        int status = 0;    \
        waitpid(child, &status, 0);   \
        if (WIFSIGNALED(status)) {   \
            PRINT_SEGFAULT("ASSERTION NOT DEATH FAILED %s (%d)\n", #call, WEXITSTATUS(status));    \
            N_ERRORS++; \
        }   \
    }   \
}

int null_argument_test() {
        ASSERT_NOT_DEATH(kill(getpid(), SIGSEGV));
    {
        ASSERT_NOT_DEATH(node_set_data(NULL, NULL));
        ASSERT_EQUAL_WITHOUT_DEATH(node_data(NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(node_next(NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(node_prev(NULL), NULL);
    }
    {
        ASSERT_NOT_DEATH(list_destroy(NULL));
        ASSERT_EQUAL_WITHOUT_DEATH(list_size(NULL), 0);
        ASSERT_EQUAL_WITHOUT_DEATH(list_head(NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_tail(NULL), NULL);
    }
    {
        struct list *list = list_create();
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_front(list, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_back(list, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_after(list, NULL, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_before(list, NULL, NULL), NULL);
        ASSERT_NOT_DEATH(list_destroy(list));

        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_front(NULL, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_back(NULL, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_after(NULL, NULL, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_insert_before(NULL, NULL, NULL), NULL);
    }
    {
        ASSERT_EQUAL_WITHOUT_DEATH(list_erase(NULL, NULL, NULL), NULL);
        ASSERT_EQUAL_WITHOUT_DEATH(list_for_each(NULL, NULL, NULL), 0);
        ASSERT_EQUAL_WITHOUT_DEATH(list_set_allocator(NULL), calloc);
    }
    return EXECUTABLE_TEST_FAILED ? TEST_FAILED : TEST_PASSED;
}