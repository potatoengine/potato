// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/box.h"

namespace up {
    class AudioEngine {
    public:
        virtual ~AudioEngine() = default;

        UP_AUDIO_API static box<AudioEngine> create();

    protected:
        AudioEngine() = default;
    };
} // namespace up
