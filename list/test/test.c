#include <stdio.h>
#include <stdlib.h>

#include "../src/list.h"
#include "../src/list_impl.h"

int N_ERRORS = 0;

#define TEST(test_call) \
    if (test_call == 0) \
        ++N_ERRORS;

#define ASSERT_EQUAL(L, R, ...) \
    if (L != R) {   \
        fprintf(stderr, "%s: ASSERTION EQUAL FAILED %s != %s", __FUNCTION__, #L, #R);    \
        fprintf(stderr, __VA_ARGS__);    \
        return 0;   \
    }

int create_test() {
    struct list *list = list_create();
    if (list) {
        ASSERT_EQUAL(list->size, 0, "Initial size is not zero");
        ASSERT_EQUAL(list->ghost.next, &list->ghost, "Ghost element format is not respected");
        ASSERT_EQUAL(list->ghost.prev, &list->ghost, "Ghost element format is not respected");
    }

    list_destroy(list);
}

void run_tests();

int main() {

}

void run_tests() {
    N_ERRORS = 0;
    TEST(create_test());
    
}