#define _GNU_SOURCE
#include <sched.h>

#include "debug.h"
#include "static_thread.h"

int static_thread_create(struct StaticThread *thread, unsigned n,
                  void *(*start_routine)(void *), void *arg)
{
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(n, &set);
        pthread_attr_t attr;
        if (pthread_attr_init(&attr) != 0 || 
            pthread_attr_setaffinity_np(&attr, sizeof(set), &set) != 0) {
                INFO("Failed to make attr.");
                return -1;
        }
        int ret = pthread_create(&thread->thread, &attr, start_routine, arg);
        if (pthread_attr_destroy(&attr) != 0) {
                INFO("Failed to destroy attr.");
                return -1;
        }
        return ret;
}

int static_thread_join(struct StaticThread *thread, void **ret)
{
        return pthread_join(thread->thread, ret);
}
