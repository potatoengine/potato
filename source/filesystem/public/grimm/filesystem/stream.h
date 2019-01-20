// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/rc.h"
#include "grimm/foundation/types.h"
#include "common.h"

namespace gm::fs {
    class StreamBackend : public shared<StreamBackend> {
    public:
        using size_type = uint64;
        using difference_type = int64;

        virtual ~StreamBackend() = default;

        virtual bool isOpen() const noexcept = 0;
        virtual bool canRead() const noexcept = 0;
        virtual bool canWrite() const noexcept = 0;
        virtual bool canSeek() const noexcept = 0;

        virtual Result seek(SeekPosition position, difference_type offset) = 0;

        virtual Result read(span<byte>& buffer) = 0;
        virtual Result write(span<byte const>& buffer) = 0;
    };

    class Stream {
    public:
        using size_type = StreamBackend::size_type;
        using difference_type = StreamBackend::difference_type;

        Stream(rc<StreamBackend> impl) : _impl(std::move(impl)) {}

        bool isOpen() const noexcept { return _impl != nullptr && _impl->isOpen(); }
        bool canRead() const noexcept { return _impl != nullptr && _impl->canRead(); }
        bool canWrite() const noexcept { return _impl != nullptr && _impl->canWrite(); }
        bool canSeek() const noexcept { return _impl != nullptr && _impl->canSeek(); }

        Result seek(SeekPosition position, difference_type offset) { return _impl->seek(position, offset); }

        Result read(span<byte>& buffer) { return _impl->read(buffer); }
        Result write(span<byte const>& buffer) { return _impl->write(buffer); }

        void close() { _impl.reset(); }

    private:
        rc<StreamBackend> _impl;
    };
} // namespace gm::fs
