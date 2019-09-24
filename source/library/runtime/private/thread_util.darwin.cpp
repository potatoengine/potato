// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/runtime/thread_util.h"
#include <pthread.h>

void up::setCurrentThreadName(zstring_view name) {
    pthread_setname_np(name.c_str());
}
