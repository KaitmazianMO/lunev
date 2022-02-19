#include "test.h"
#include "../list/list.h"
#include "../list/list_impl.h"

void run_tests();
int initialized_empty_list_test();
int head_tail_inserting_test();
int null_argument_test();

int main() {
    run_tests();

    return 0;
}

void run_tests() {
    RUN_TEST(null_argument_test);
    RUN_TEST(initialized_empty_list_test);
    RUN_TEST(head_tail_inserting_test);
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

int null_argument_test() {
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

int head_tail_inserting_test() {
    struct list *list_front_inserting = list_create();
    struct list *list_back_inserting  = list_create();

    void *arr[] = { (void *)0, (void *)1, (void *)2, (void *)3,
                    (void *)4, (void *)5, (void *)6, (void *)7,
                    (void *)8, (void *)9, (void *)10 };
    size_t arr_sz = sizeof(arr) / sizeof(arr[0]);

    if (list_front_inserting) {
        for (size_t i = 0; i < arr_sz; ++i) {
            if (list_insert_back(list_front_inserting, arr[i]) == NULL) {
                list_destroy(list_front_inserting);
                return TEST_ENDED_PREMATURELY;
            }
        }

        struct list_node *node = list_head(list_front_inserting);
        for (size_t i = 0; i != arr_sz; ++i) {
            ASSERT_EQUAL(node_data(node), arr[i], "list_insert_back doesn't work"); 
            node = node_next(node);          
        }
    }

    if (list_back_inserting) {
        for (size_t i = 0; i < arr_sz; ++i) {
            if (list_insert_front(list_back_inserting, arr[i]) == NULL) {
                list_destroy(list_back_inserting);
                return TEST_ENDED_PREMATURELY;
            }
        }

        struct list_node *node = list_head(list_back_inserting);
        for (size_t i = arr_sz; i != 0; --i) {
            ASSERT_EQUAL(node_data(node), arr[i-1], "list_insert_front doesn't work");           
            node = node_next(node);
        }
    }

    list_destroy(list_front_inserting);
    list_destroy(list_back_inserting);
    return EXECUTABLE_TEST_FAILED ? TEST_FAILED : TEST_PASSED;
}