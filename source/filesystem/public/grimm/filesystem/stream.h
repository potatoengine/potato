// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/int_types.h"
#include "grimm/foundation/span.h"
#include "common.h"

namespace gm::fs {
    class Stream {
    public:
        using size_type = uint64;
        using difference_type = int64;

        class Backend {
        public:
            Backend() = default;
            virtual ~Backend() = default;

            Backend(Backend const&) = delete;
            Backend& operator=(Backend const&) = delete;

            virtual bool isOpen() const noexcept = 0;
            virtual bool isEof() const noexcept = 0;
            virtual bool canRead() const noexcept = 0;
            virtual bool canWrite() const noexcept = 0;
            virtual bool canSeek() const noexcept = 0;

            virtual Result seek(SeekPosition position, difference_type offset) = 0;
            virtual difference_type tell() const = 0;
            virtual difference_type remaining() const = 0;

            virtual Result read(span<byte>& buffer) = 0;
            virtual Result write(span<byte const> buffer) = 0;
            virtual Result flush() = 0;
        };

        Stream() = default;
        Stream(box<Backend> impl) noexcept : _impl(std::move(impl)) {}
        Stream(std::nullptr_t) noexcept {}

        Stream& operator=(box<Backend> impl) noexcept {
            _impl = std::move(impl);
            return *this;
        }
        Stream& operator=(std::nullptr_t) noexcept {
            _impl.reset();
            return *this;
        }

        bool isOpen() const noexcept { return _impl != nullptr && _impl->isOpen(); }
        bool isEof() const noexcept { return _impl == nullptr || _impl->isEof(); }
        bool canRead() const noexcept { return _impl != nullptr && _impl->canRead(); }
        bool canWrite() const noexcept { return _impl != nullptr && _impl->canWrite(); }
        bool canSeek() const noexcept { return _impl != nullptr && _impl->canSeek(); }

        explicit operator bool() const noexcept { return isOpen(); }

        Result seek(SeekPosition position, difference_type offset) { return _impl->seek(position, offset); }
        difference_type tell() const { return _impl->tell(); }
        difference_type remaining() const { return _impl->remaining(); }

        Result read(span<byte>& buffer) { return _impl->read(buffer); }
        Result write(span<byte const> buffer) { return _impl->write(buffer); }
        Result flush() { return _impl->flush(); }

        void close() { _impl.reset(); }

    private:
        box<Backend> _impl;
    };
} // namespace gm::fs
