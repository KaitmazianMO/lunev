#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include <stddef.h>

struct list;
struct list_node;
typedef int (*list_call_back)(void *data, void *args);
typedef void *(*list_allocator)(size_t n, size_t sz); 

void  node_set_data(struct list_node *node, const void *data);
void *node_data(struct list_node *node);
struct list_node *node_next(struct list_node *node);
struct list_node *node_prev(struct list_node *node);

struct list *list_create();
void  list_destroy(struct list *this);

size_t list_size(struct list *this);
struct list_node *list_head(const struct list *this);
struct list_node *list_tail(const struct list *this);

struct list_node *list_insert_front(struct list *this, const void *data);
struct list_node *list_insert_back(struct list *this, const void *data);
struct list_node *list_insert_after(struct list *this, struct list_node *node, const void *data);
struct list_node *list_insert_before(struct list *this, struct list_node *node, const void *data);

// safe - checks if the node belong to the list
void list_erase(struct list *this, struct list_node *node, _Bool safe);

// cb returns 0 to stop iterations
int list_for_each(const struct list *this, list_call_back cb, void *cb_context);

list_allocator list_set_allocator(list_allocator allocate);

#endif // LIST_H_INCLUDED