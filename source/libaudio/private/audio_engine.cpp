// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "audio_engine.h"
#include "sound_resource.h"

#include "potato/runtime/resource_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/vector.h"

#include <soloud.h>
#include <soloud_wav.h>

namespace up {
    namespace {
        class AudioEngineImpl final : public AudioEngine {
        public:
            explicit AudioEngineImpl(ResourceLoader& resourceLoader);
            ~AudioEngineImpl() override;

            auto createResourceBackend() -> box<ResourceLoaderBackend> override;
            auto play(SoundResource const* sound) -> PlayHandle override;

        private:
            ResourceLoader& _resourceLoader;
            SoLoud::Soloud _soloud;
        };

        class SoundResourceWav final : public SoundResource {
        public:
            mutable SoLoud::Wav _wav;
        };

        class SoundResourceLoaderBackend : public ResourceLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return SoundResource::resourceType; }
            rc<Resource> loadFromStream(Stream stream) override;
        };
    } // namespace
} // namespace up

up::AudioEngineImpl::AudioEngineImpl(up::ResourceLoader& resourceLoader) : _resourceLoader(resourceLoader) {
    _soloud = SoLoud::Soloud();
    _soloud.init();
}

up::AudioEngineImpl::~AudioEngineImpl() {
    _soloud.deinit();
}

auto up::AudioEngine::create(ResourceLoader& resourceLoader) -> box<AudioEngine> {
    return new_box<AudioEngineImpl>(resourceLoader);
}

auto up::AudioEngineImpl::createResourceBackend() -> box<ResourceLoaderBackend> {
    return new_box<SoundResourceLoaderBackend>();
}

auto up::AudioEngineImpl::play(SoundResource const* sound) -> PlayHandle {
    if (sound == nullptr) {
        return {};
    }

    auto handle = _soloud.play(static_cast<SoundResourceWav const*>(sound)->_wav);
    return static_cast<PlayHandle>(handle);
}

auto up::SoundResourceLoaderBackend::loadFromStream(Stream stream) -> rc<Resource> {
    vector<byte> contents;
    if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
        return nullptr;
    }

    auto wav = new_shared<SoundResourceWav>();
    auto const result = wav->_wav.loadMem(
        reinterpret_cast<unsigned char const*>(contents.data()),
        static_cast<unsigned int>(contents.size()),
        true,
        false);
    if (result != 0) {
        return nullptr;
    }

    return wav;
}
