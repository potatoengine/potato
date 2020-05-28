// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "audio_engine.h"

#include <soloud.h>

namespace {
    class AudioEngineImpl final : public up::AudioEngine {
    public:
        AudioEngineImpl() = default;
    };
} // namespace

auto up::AudioEngine::create() -> box<AudioEngine> { return new_box<AudioEngineImpl>(); }
