// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "audio_engine.h"
#include "sound_resource.h"

#include "potato/runtime/stream.h"
#include "potato/spud/vector.h"

#include <soloud.h>
#include <soloud_wav.h>

namespace {
    class AudioEngineImpl final : public up::AudioEngine {
    public:
        explicit AudioEngineImpl(up::FileSystem& fileSystem);
        ~AudioEngineImpl();

        auto loadSound(up::zstring_view path) -> up::rc<up::SoundResource> override;
        auto play(up::SoundResource const* sound) -> up::PlayHandle override;

    private:
        up::FileSystem& _fileSystem;
        SoLoud::Soloud _soloud;
    };

    class SoundResourceWav final : public up::SoundResource {
    public:
        mutable SoLoud::Wav _wav;
    };
} // namespace

AudioEngineImpl::AudioEngineImpl(up::FileSystem& fileSystem) : _fileSystem(fileSystem) {
    _soloud = SoLoud::Soloud();
    _soloud.init();
}

AudioEngineImpl::~AudioEngineImpl() { _soloud.deinit(); }

auto up::AudioEngine::create(FileSystem& fileSystem) -> box<AudioEngine> { return new_box<AudioEngineImpl>(fileSystem); }

auto AudioEngineImpl::loadSound(up::zstring_view path) -> up::rc<up::SoundResource> {
    up::vector<up::byte> contents;
    auto stream = _fileSystem.openRead(path);
    if (auto rs = readBinary(stream, contents); rs != up::IOResult::Success) {
        return nullptr;
    }
    stream.close();

    auto wav = up::new_shared<SoundResourceWav>();
    auto const result =
        wav->_wav.loadMem(reinterpret_cast<unsigned char const*>(contents.data()), static_cast<unsigned int>(contents.size()), true, false);
    if (result != 0) {
        return nullptr;
    }

    return wav;
}

auto AudioEngineImpl::play(up::SoundResource const* sound) -> up::PlayHandle {
    if (sound == nullptr) {
        return {};
    }

    auto handle = _soloud.play(static_cast<SoundResourceWav const*>(sound)->_wav);
    return static_cast<up::PlayHandle>(handle);
}
