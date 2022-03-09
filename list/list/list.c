#include "list.h"
#include "list_impl.h"

#include <stdlib.h>

#define GHOST(list) (&list->ghost)
#define SIZE(list)  (list->size)
#define HEAD(list)  (list->ghost.next)
#define TAIL(list)  (list->ghost.prev)

static list_allocator ALLOCATOR = calloc;

static struct list_node *node_create(const void *data);

void node_set_data(struct list_node *node, const void *data) {
    if (node) {
        node->data = (void *)data;
    }
}

void *node_data(struct list_node *node) {
    if (node) {
        return node->data; 
    }
    return NULL;
}

struct list_node *node_next(struct list_node *node) {
    if (node) {
        return node->next;
    }
    return NULL; 
}

struct list_node *node_prev(struct list_node *node) {
    if (node) {
        return node->prev;
    }
    return NULL; 
}

struct list *list_create() {
    struct list *this = ALLOCATOR(1, sizeof(*this));
    if (!this) {
        return NULL;
    }

    this->ghost.next = &this->ghost; 
    this->ghost.prev = &this->ghost;
    this->size = 0;
    return this;
}

void list_destroy(struct list *this) {
    if (this && SIZE(this)) {
        struct list_node *node = HEAD(this),
                         *tmp = NULL;
        while (node != GHOST(this)) {
            tmp = node->next;
            free(node);
            node = tmp;
        }
    }
    free(this);
}

size_t list_size(struct list *this) {
    if (this) {
        return SIZE(this);
    }
    return 0;
}

struct list_node *list_head(const struct list *this) {
    if (this && SIZE(this)) {
        return HEAD(this);  
    }
    return NULL;
}

struct list_node *list_tail(const struct list *this) {
    if (this && SIZE(this)) {
        return TAIL(this);
    }
    return NULL;
}

struct list_node *list_insert_front(struct list *this, const void *data) {
    if (this) {
        return list_insert_before(this, HEAD(this), data);
    }
    return NULL;
}

struct list_node *list_insert_back(struct list *this, const void *data) {
    if (this) {
        return list_insert_after(this, TAIL(this), data);
    }
    return NULL;
}


struct list_node *list_insert_after(struct list *this, struct list_node *node, const void *data) {
    if (!this || !node) {
        return NULL;
    }

    struct list_node *new_node = node_create(data); 
    if (!new_node) {
        return NULL;
    }
    
    new_node->prev   = node;
    new_node->next   = node->next;
    node->next->prev = new_node;
    node->next       = new_node;
    ++SIZE(this);
    return new_node;
}

struct list_node *list_insert_before(struct list *this, struct list_node *node, const void *data) {
    if (!this || !node) {
        return NULL;
    }

    struct list_node *new_node = node_create(data); 
    if (!new_node) {
        return NULL;
    }
    
    new_node->next   = node;
    new_node->prev   = node->prev;
    node->prev->next = new_node;
    node->prev       = new_node;
    ++SIZE(this);
    return new_node;
}

 void list_erase(struct list *this, struct list_node *node, _Bool safe) {
    if (!this || !SIZE(this) || !node || node == &this->ghost) {
        return;
    }

    if (safe) {
        _Bool found = 0;
        struct list_node *list_node = list_head(this);
        while (list_node != GHOST(this)) {
            if (list_node == node) { 
                found = 1;
                break;
            }
            list_node = list_node->next;
        }
        if (!found) {
            return;
        }
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
    --SIZE(this);
}

int list_for_each(const struct list *this, list_call_back cb, void *cb_context) {
    if (!this || !SIZE(this) || !cb) {
        return 0;
    }

    struct list_node *node = HEAD(this);
    while (node != GHOST(this)) {
        if (cb(node->data, cb_context) == 0) {
            return 0;
        }
        node = node->next;
    }

    return 1;
}

static struct list_node *node_create(const void *data) {
    struct list_node *node = ALLOCATOR (1, sizeof(*node));
    if (node) {
        node->data = (void *)data;
    }
    return node;
}

list_allocator list_set_allocator(list_allocator new_allocator) {
    list_allocator old_alloc = ALLOCATOR;
    if (new_allocator) {
        ALLOCATOR = new_allocator;
    }
    return old_alloc;
}