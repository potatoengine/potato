// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/concurrency/thread_util.h"

#if !defined(_GNU_SOURCE)
#    define _GNU_SOURCE // for glibc
#endif

#include <pthread.h>

void up::setCurrentThreadName(zstring_view name) {
    pthread_setname_np(pthread_self(), name.c_str());
}
