// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/native_backend.h"
#include "potato/filesystem/stream.h"
#include <fstream>

auto up::NativeBackend::create() -> FileSystem {
    return FileSystem(rc<NativeBackend>(new NativeBackend));
}

namespace up {
    namespace {
        struct NativeInputBackend : public Stream::Backend {
            NativeInputBackend(std::ifstream stream) : _stream(std::move(stream)) {}

            bool isOpen() const noexcept override { return _stream.is_open(); }
            bool isEof() const noexcept override { return _stream.eof(); }
            bool canRead() const noexcept override { return true; }
            bool canWrite() const noexcept override { return false; }
            bool canSeek() const noexcept override { return true; }

            Result seek(SeekPosition position, Stream::difference_type offset) override {
                _stream.seekg(offset, position == SeekPosition::Begin ? std::ios::beg : position == SeekPosition::End ? std::ios::end : std::ios::cur);
                return Result::Success;
            }
            Stream::difference_type tell() const override {
                return _stream.tellg();
            }
            Stream::difference_type remaining() const override {
                auto pos = _stream.tellg();
                _stream.seekg(0, std::ios::end);
                auto end = _stream.tellg();
                _stream.seekg(pos, std::ios::beg);
                return end - pos;
            }

            Result read(span<byte>& buffer) override {
                if (!_stream.is_open()) {
                    return Result::InvalidArgument;
                }

                _stream.read(buffer.as_chars().data(), buffer.size());
                buffer = buffer.first(_stream.gcount());
                if (_stream.eof()) {
                    _stream.clear();
                }

                return Result::Success;
            }

            Result write(span<byte const>) override {
                return Result::UnsupportedOperation;
            }

            Result flush() override {
                return Result::UnsupportedOperation;
            }

            mutable std::ifstream _stream;
        };

        struct NativeOutputBackend : public Stream::Backend {
            NativeOutputBackend(std::ofstream stream) : _stream(std::move(stream)) {}

            bool isOpen() const noexcept override { return _stream.is_open(); }
            bool isEof() const noexcept override { return _stream.eof(); }
            bool canRead() const noexcept override { return false; }
            bool canWrite() const noexcept override { return true; }
            bool canSeek() const noexcept override { return false; }

            Result seek(SeekPosition position, Stream::difference_type offset) override {
                return Result::UnsupportedOperation;
            }
            Stream::difference_type tell() const override {
                return 0;
            }
            Stream::difference_type remaining() const override {
                return 0;
            }

            Result write(span<byte const> buffer) override {
                _stream.write(buffer.as_chars().data(), buffer.size());
                return Result::Success;
            }

            Result flush() override {
                _stream.flush();
                return Result::Success;
            }

            Result read(span<byte>&) override {
                return Result::UnsupportedOperation;
            }

            std::ofstream _stream;
        };
    } // namespace
} // namespace up

auto up::NativeBackend::openRead(zstring_view path, FileOpenMode mode) const -> Stream {
    return Stream(up::new_box<NativeInputBackend>(std::ifstream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : std::ios_base::openmode{})));
}

auto up::NativeBackend::openWrite(zstring_view path, FileOpenMode mode) -> Stream {
    return Stream(up::new_box<NativeOutputBackend>(std::ofstream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : std::ios_base::openmode{})));
}
