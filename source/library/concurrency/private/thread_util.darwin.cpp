// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/concurrency/thread_util.h"
#include <pthread.h>

void up::concurrency::setCurrentThreadName(zstring_view name) {
    pthread_setname_np(name.c_str());
}
