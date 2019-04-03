// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/concurrency/thread_util.h"
#include <pthread.h>

void gm::concurrency::setCurrentThreadName(zstring_view name) {
    pthread_setname_np(name.c_str());
}
