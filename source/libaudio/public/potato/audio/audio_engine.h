// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/filesystem.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"

namespace up {
    enum class PlayHandle : uint32 { None = 0 };

    class SoundResource;

    class AudioEngine {
    public:
        virtual ~AudioEngine() = default;

        UP_AUDIO_API static auto create(FileSystem& fileSystem) -> box<AudioEngine>;

        virtual auto loadSound(zstring_view path) -> rc<SoundResource> = 0;
        virtual auto play(SoundResource const* sound) -> PlayHandle = 0;

    protected:
        AudioEngine() = default;
    };
} // namespace up
