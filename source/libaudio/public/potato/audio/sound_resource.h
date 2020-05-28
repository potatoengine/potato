// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/rc.h"

namespace up {
    class SoundResource : public shared<SoundResource> {
    public:
        virtual ~SoundResource() = default;
    };
} // namespace up
