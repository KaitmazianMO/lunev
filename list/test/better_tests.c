#include "better_tests.h"
#include "../list/list.h"
#include "../list/list_impl.h"

void run_tests();
void null_argument_test();
void initialized_empty_list_test();
void head_tail_inserting_test();
int inserting_test();

int main() {
    run_tests();

    return 0;
}

void run_tests() {
    null_argument_test();
    initialized_empty_list_test();
    head_tail_inserting_test();
}

void initialized_empty_list_test() {
    TEST_HEAD(11);

    struct list *list = list_create();
    EXPECT_NOT(list == NULL, "List allocation failed.");
    {
        ASSERT_EQ(list->size, 0);
        ASSERT_EQ(list->ghost.next, &list->ghost);
        ASSERT_EQ(list->ghost.prev, &list->ghost);
    }
    {
        ASSERT_EQ(list->size,   list_size(list));
        ASSERT_EQ(&list->ghost, list_head(list));
        ASSERT_EQ(&list->ghost, list_tail(list));
    }
    {
        ASSERT_EQ(&list->ghost,     node_next(&list->ghost));
        ASSERT_EQ(&list->ghost,     node_prev(&list->ghost));
        ASSERT_EQ(list->ghost.data, node_data(&list->ghost));
        void *data = (void *)7;
        node_set_data(&list->ghost, data);
        ASSERT_EQ(data, node_data(&list->ghost));
    }

    list_destroy(list);
    
    TEST_SUMMARIZE();
}

void null_argument_test() {
    TEST_HEAD(20);

    {
        ASSERT_NOT_DEATH(node_set_data(NULL, NULL));
        ASSERT_EQ(node_data(NULL), NULL);
        ASSERT_EQ(node_next(NULL), NULL);
        ASSERT_EQ(node_prev(NULL), NULL);
    }
    {
        ASSERT_NOT_DEATH(list_destroy(NULL));
        ASSERT_EQ(list_size(NULL), 0);
        ASSERT_EQ(list_head(NULL), NULL);
        ASSERT_EQ(list_tail(NULL), NULL);
    }
    {
        struct list *list = list_create();
        ASSERT_NOT_DEATH(list_insert_front(list, NULL));
        ASSERT_NOT_DEATH(list_insert_back(list, NULL));
        ASSERT_NOT_DEATH(list_insert_after(list, NULL, NULL));
        ASSERT_NOT_DEATH(list_insert_before(list, NULL, NULL));
        ASSERT_NOT_DEATH(list_destroy(list));

        ASSERT_EQ(list_insert_front(NULL, NULL),        NULL);
        ASSERT_EQ(list_insert_back(NULL, NULL),         NULL);
        ASSERT_EQ(list_insert_after(NULL, NULL, NULL),  NULL);
        ASSERT_EQ(list_insert_before(NULL, NULL, NULL), NULL);
    }
    {
        ASSERT_EQ(list_erase(NULL, NULL, NULL), NULL);
        ASSERT_EQ(list_for_each(NULL, NULL, NULL), 0);
        ASSERT_EQ(list_set_allocator(NULL), calloc);
    }

    TEST_SUMMARIZE();
}

void head_tail_inserting_test() {

    TEST_HEAD(11*4);

    struct list *list_front_inserting = list_create();
    struct list *list_back_inserting  = list_create();

    void *arr[] = { (void *)0, (void *)1, (void *)2, (void *)3,
                    (void *)4, (void *)5, (void *)6, (void *)7,
                    (void *)8, (void *)9, (void *)10 };
    size_t arr_sz = sizeof(arr) / sizeof(arr[0]);

    if (list_front_inserting) {
        for (size_t i = 0; i < arr_sz; ++i) {
            EXPECT_NOT(list_insert_back(list_front_inserting, arr[i]) == NULL, "List inserting failed.");
        }

        struct list_node *node = list_head(list_front_inserting);
        for (size_t i = 0; i != arr_sz; ++i) {
            ASSERT_EQ(node->data, arr[i]); 
            node = node_next(node);          
        }
    }

    if (list_back_inserting) {
        for (size_t i = 0; i < arr_sz; ++i) {
            EXPECT_NOT(list_insert_back(list_back_inserting, arr[i]) == NULL, "List inserting failed.");
        }

        struct list_node *node = list_head(list_back_inserting);
        for (size_t i = arr_sz; i != 0; --i) {
            ASSERT_EQ(node->data, arr[i-1]);           
            node = node_next(node);
        }
    }

    list_destroy(list_front_inserting);
    list_destroy(list_back_inserting);
    TEST_SUMMARIZE();
}