#ifndef LIST_IMPL_H_INCLUDED
#define LIST_IMPL_H_INCLUDED

struct list_node {
    void *data;
    struct list_node *prev, *next;
};

struct list {
    struct list_node ghost;
    size_t size;
};


#endif