// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/audio/audio_engine.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

#include <AK/IBytes.h>

#if !defined(AK_OPTIMIZED)
#   include <AK/Comm/AkCommunication.h>
#endif

up::AudioEngine::AudioEngine() = default;
up::AudioEngine::~AudioEngine() = default;

bool up::AudioEngine::initialize() {
    AkMemSettings memSettings;
    memSettings.uMaxNumPools = 20;

    if (AK::MemoryMgr::Init(&memSettings) != AK_Success) {
        return false;
    }

    AkStreamMgrSettings stmSettings;
    AK::StreamMgr::GetDefaultSettings(stmSettings);

    if (!AK::StreamMgr::Create(stmSettings)) {
        return false;
    }

    AkDeviceSettings deviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

    AkInitSettings initSettings;
    AkPlatformInitSettings platformInitSettings;
    AK::SoundEngine::GetDefaultInitSettings(initSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

    AKRESULT result = AK::SoundEngine::Init(&initSettings, &platformInitSettings);
    if (result != AK_Success) {
        return false;
    }

    return true;
}
