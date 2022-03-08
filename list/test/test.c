#include "test.h"
#include "../list/list.h"
#include "../list/list_impl.h"

void run_tests();
void null_argument_test();
void initialized_empty_list_test();
void head_tail_inserting_test();
void inserting_test();
void erase_test();
void for_each_test();
void memory_allocation_failed_test();

int main() {
    run_tests();

    return 0;
}

void run_tests() {
    null_argument_test();
    initialized_empty_list_test();
    head_tail_inserting_test();
    inserting_test();
    erase_test();
    for_each_test();
    memory_allocation_failed_test();
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
    TEST_HEAD(22);

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
        struct list *list = NULL;
        ASSERT_NOT_DEATH(list = list_create());
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
        ASSERT_NOT_DEATH(list_erase(NULL, NULL, 0));
        ASSERT_NOT_DEATH(list_erase(NULL, NULL, 1));
        ASSERT_EQ(list_for_each(NULL, NULL, NULL), 0);
        ASSERT_EQ(list_set_allocator(NULL), calloc);
    }

    TEST_SUMMARIZE();
}

void head_tail_inserting_test() {

    TEST_HEAD(11*4);

    struct list *list_front_inserting = list_create();
    struct list *list_back_inserting  = list_create();

    void * const arr[] = { (void *)0, (void *)1, (void *)2, (void *)3,
                           (void *)4, (void *)5, (void *)6, (void *)7,
                           (void *)8, (void *)9, (void *)10 };
    const size_t arr_sz = sizeof(arr) / sizeof(arr[0]);

    if (list_front_inserting) {
        for (size_t i = 0; i < arr_sz; ++i) {
            EXPECT_NOT(list_insert_front(list_front_inserting, arr[i]) == NULL, "List inserting failed.");
        }

        struct list_node *node = list_head(list_front_inserting);
        for (size_t i = 0; i != arr_sz; ++i) {
            ASSERT_EQ(node->data, arr[arr_sz - i - 1]); 
            node = node->next;          
        }
    }

    if (list_back_inserting) {
        for (size_t i = 0; i < arr_sz; ++i) {
            EXPECT_NOT(list_insert_back(list_back_inserting, arr[i]) == NULL, "List inserting failed.");
        }

        struct list_node *node = list_head(list_back_inserting);
        for (size_t i = 0; i < arr_sz; ++i) {
            if (node->data != arr[i]) fprintf(stderr, "%zu:%p %p\n", i, node->data, arr[i]);
            ASSERT_EQ(node->data, arr[i]);           
            node = node->next;
        }
    }

    list_destroy(list_front_inserting);
    list_destroy(list_back_inserting);
    TEST_SUMMARIZE();
}

void inserting_test() {
    TEST_HEAD(8);

    struct list *list = list_create();
    EXPECT_NOT(list == NULL, "List allocation failed.");
    
    struct list_node *head = list_insert_front(list, NULL);
    EXPECT_NOT(head == NULL, "List insert front failed.");
    struct list_node *head_next = list_insert_after(list, head, NULL);
    EXPECT_NOT(head_next == NULL, "List insert after failed.");
    ASSERT_EQ(head->next, head_next);
    ASSERT_EQ(head, head_next->prev);
    
    struct list_node *head_next_prev = list_insert_before(list, head_next, NULL);
    EXPECT_NOT(head_next_prev == NULL, "List isnert before failed.");
    ASSERT_EQ(head_next_prev->next, head_next);
    ASSERT_EQ(head_next_prev, head_next->prev);

    list_destroy(list);
    TEST_SUMMARIZE();
}

void erase_test() {
   TEST_HEAD(16);

    struct list *list = list_create();
    EXPECT_NOT(list == NULL, "List allocation failed.");
    
    ASSERT_NOT_DEATH(list_erase(list, NULL, 0));

    struct list_node *node1 = list_insert_front(list, NULL);
    EXPECT_NOT(node1 == NULL, "List insert front failed.");
    struct list_node *node2 = list_insert_after(list, node1, NULL);
    EXPECT_NOT(node2 == NULL, "List insert after failed.");
    struct list_node *node3 = list_insert_before(list, node2, NULL);
    EXPECT_NOT(node3 == NULL, "List isnert before failed.");

    struct list_node another_node = {};
    list_erase(list, &another_node, 1);
    ASSERT_EQ(list_size(list), 3u);

    list_erase(list, node2, 0u);
    ASSERT_EQ(list_size(list), 2u);
    ASSERT_EQ(node1->next, node3);
    ASSERT_EQ(node1, node3->prev);
    list_erase(list, node2, 1);
    ASSERT_EQ(list_size(list), 2u);

    list_erase(list, node3, 1);
    ASSERT_EQ(list_size(list), 1u);
    ASSERT_EQ(node1, list_head(list));
    ASSERT_EQ(node1, list_tail(list));
    list_erase(list, node3, 1);
    ASSERT_EQ(list_size(list), 1u);

    list_erase(list, node1, 0);
    ASSERT_EQ(list_size(list), 0u);
    list_erase(list, node1, 1);
    ASSERT_EQ(list_size(list), 0u);

    list_destroy(list);
    TEST_SUMMARIZE();
}

int action(void *data, void *sum) {
    *(int *)sum += *(int *)data;

    return 1;
}

int premature_ending_action(void *data, void *cntx) {
    (void)data;
    (void)cntx;
    return 0;    
}

void for_each_test() {
    TEST_HEAD(11 + 2);

    struct list *list = list_create();
    EXPECT_NOT(list == NULL, "List allocation failed.");

    int arr[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    const size_t size = sizeof arr / sizeof (arr[0]);
    for (size_t i = 0; i < size; ++i) {
        EXPECT_NOT(list_insert_back(list, arr + i) == NULL, "List inserting failed.");
    }

    int sum = 0;
    list_for_each(list, action, &sum);
    ASSERT_EQ(sum, 55);

    list_for_each(list, premature_ending_action, &sum);

    TEST_SUMMARIZE();
}

void *rofl_allocator(size_t n, size_t sz) {
    return NULL;
}

void memory_allocation_failed_test() {
    TEST_HEAD(5);

    struct list *list = list_create();
    EXPECT_NOT(list == NULL, "List allocation failed.");
    
    list_allocator old_allocator = list_set_allocator(rofl_allocator);
    ASSERT_EQ(list_create(), NULL);
    ASSERT_EQ(list_insert_front(list, NULL), NULL);
    ASSERT_EQ(list_insert_back(list, NULL), NULL);
    ASSERT_EQ(list_size(list), 0u);

    list_set_allocator(old_allocator);
    list_destroy(list);
    TEST_SUMMARIZE();
}