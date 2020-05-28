// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "audio_engine.h"

#include <soloud.h>

namespace {
    class AudioEngineImpl final : public up::AudioEngine {
    public:
        AudioEngineImpl();
        ~AudioEngineImpl();

    private:
        SoLoud::Soloud _soloud;
    };
} // namespace

AudioEngineImpl::AudioEngineImpl() {
    _soloud = SoLoud::Soloud();
    _soloud.init();
}

AudioEngineImpl::~AudioEngineImpl() { _soloud.deinit(); }

auto up::AudioEngine::create() -> box<AudioEngine> { return new_box<AudioEngineImpl>(); }
