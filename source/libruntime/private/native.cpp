// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "native.h"
#include "stream.h"

#include <fstream>

namespace up {
    namespace {
        struct NativeInputBackend final : public Stream::Backend {
            NativeInputBackend(std::ifstream stream) : _stream(std::move(stream)) {}

            bool isOpen() const noexcept override { return _stream.is_open(); }
            bool isEof() const noexcept override { return _stream.eof(); }
            bool canRead() const noexcept override { return true; }
            bool canWrite() const noexcept override { return false; }
            bool canSeek() const noexcept override { return true; }

            IOResult seek(SeekPosition position, Stream::difference_type offset) override {
                _stream.seekg(offset,
                    position == SeekPosition::Begin ? std::ios::beg : position == SeekPosition::End ? std::ios::end : std::ios::cur);
                return IOResult::Success;
            }
            Stream::difference_type tell() const override { return _stream.tellg(); }
            Stream::difference_type remaining() const override {
                auto pos = _stream.tellg();
                _stream.seekg(0, std::ios::end);
                auto end = _stream.tellg();
                _stream.seekg(pos, std::ios::beg);
                return end - pos;
            }

            IOResult read(span<byte>& buffer) override {
                if (!_stream.is_open()) {
                    return IOResult::InvalidArgument;
                }

                _stream.read(buffer.as_chars().data(), buffer.size());
                buffer = buffer.first(_stream.gcount());
                if (_stream.eof()) {
                    _stream.clear();
                }

                return IOResult::Success;
            }

            IOResult write([[maybe_unused]] span<byte const> ignore) override { return IOResult::UnsupportedOperation; }

            IOResult flush() override { return IOResult::UnsupportedOperation; }

            mutable std::ifstream _stream;
        };

        struct NativeOutputBackend final : public Stream::Backend {
            NativeOutputBackend(std::ofstream stream) : _stream(std::move(stream)) {}

            bool isOpen() const noexcept override { return _stream.is_open(); }
            bool isEof() const noexcept override { return _stream.eof(); }
            bool canRead() const noexcept override { return false; }
            bool canWrite() const noexcept override { return true; }
            bool canSeek() const noexcept override { return false; }

            IOResult seek(SeekPosition position, Stream::difference_type offset) override { return IOResult::UnsupportedOperation; }
            Stream::difference_type tell() const noexcept override { return 0; }
            Stream::difference_type remaining() const noexcept override { return 0; }

            IOResult write(span<byte const> buffer) override {
                _stream.write(buffer.as_chars().data(), buffer.size());
                return IOResult::Success;
            }

            IOResult flush() override {
                _stream.flush();
                return IOResult::Success;
            }

            IOResult read(span<byte>&) override { return IOResult::UnsupportedOperation; }

            std::ofstream _stream;
        };
    } // namespace
} // namespace up

auto up::NativeFileSystem::openRead(zstring_view path, FileOpenMode mode) const -> Stream {
    return Stream(up::new_box<NativeInputBackend>(
        std::ifstream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : std::ios_base::openmode{})));
}

auto up::NativeFileSystem::openWrite(zstring_view path, FileOpenMode mode) -> Stream {
    return Stream(up::new_box<NativeOutputBackend>(
        std::ofstream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : std::ios_base::openmode{})));
}
