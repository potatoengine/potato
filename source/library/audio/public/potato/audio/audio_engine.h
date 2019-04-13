// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace up {

    /// API for controlling the low-level audio engine
    class AudioEngine {
    public:
        AudioEngine(AudioEngine&&) = delete;
        AudioEngine& operator=(AudioEngine&&) = delete;

        UP_AUDIO_API AudioEngine();
        UP_AUDIO_API ~AudioEngine();

        UP_AUDIO_API bool initialize();
    };
}
