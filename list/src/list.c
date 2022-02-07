#include "list.h"
#include "list_impl.h"

#include <stdlib.h>

#define SIZE(list)  (list->size)
#define HEAD(list)  (list->ghost.next)
#define TAIL(list)  (list->ghost.prev)

static list_allocator ALLOCATOR = calloc;

static struct list_node *node_create(const void *data);

void node_set_data(struct list_node *node, const void *data) {
    if (node) {
        node->data = data;
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

struct list_node *node_next(struct list_node *node) {
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
    
    this->ghost.next = 
    this->ghost.prev =
    &this->ghost;
    this->size = 0;
    return this;
}

void list_destroy(struct list *this) {
    if (this && SIZE(this)) {
        const struct list_node *tail = TAIL(this);
        struct list_node *node = HEAD(this),
                         *tmp = NULL;
        while (node != tail) {
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
    if (this) {
        return HEAD(this);  
    }
    return NULL;
}

struct list_node *list_tail(const struct list *this) {
    if (this) {
        return HEAD(this);  
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
    return new_node;
}

struct list_node *list_erase(struct list *this, struct list_node *begin, struct list_node *end) {
    if (!this || !SIZE(this) || !begin || !end) {
        return NULL;
    }

    begin->prev->next = end;
    end->prev = begin->prev;
    
    struct list_node *tmp = NULL;
    struct list_node *node = begin;
    while (node != end) {
        tmp = node->next;
        free(node);
        node = tmp;
    } 

    return end;    
}

int list_for_each(const struct list *this, list_call_back cb, void *cb_context) {
    if (!this || !cb) {
        return 0;
    }

    const struct list_node *tail = TAIL(this);
    struct list_node *node = HEAD(this),
                     *tmp = NULL;
    while (node != tail) {
        if (cb(node->data, cb_context) == 0) {
            return 0;
        }
    }

    return 1;
}

static struct list_node *node_create(const void *data) {
    struct list_node *node = ALLOCATOR (1, sizeof(*node));
    if (node) {
        node->data = data;
    }
    return node;
}

list_allocator *list_set_allocator(list_allocator *new_allocator) {
    list_allocator *old_alloc = ALLOCATOR;
    if (new_allocator) {
        ALLOCATOR = new_allocator;
    }

    return old_alloc;
}