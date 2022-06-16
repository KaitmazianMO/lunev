#ifndef STATIC_THREAD_H_INCLUDED
#define STATIC_THREAD_H_INCLUDED

#include <pthread.h>

struct StaticThread {
        pthread_t thread;
};

int static_thread_create(struct StaticThread *thread, unsigned n_core,
                  void *(*start_routine) (void *), void *arg);

int static_thread_join(struct StaticThread *thread, void **ret);

#endif // THREAD_H_INCLUDED