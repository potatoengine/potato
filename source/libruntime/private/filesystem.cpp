// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "assertion.h"
#include "filesystem.h"
#include "stream.h"

#include <filesystem>
#include <fstream>

namespace up {
    namespace {
        struct NativeStreamBackend final : public Stream::Backend {
            explicit NativeStreamBackend(std::ifstream stream) : _stream(std::move(stream)) {}

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
            explicit NativeOutputBackend(std::ofstream stream) : _stream(std::move(stream)) {}

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

auto up::FileSystem::openRead(zstring_view path, FileOpenMode mode) -> Stream {
    std::ifstream nativeStream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : std::ios_base::openmode{});
    if (!nativeStream) {
        return nullptr;
    }
    return Stream(up::new_box<NativeStreamBackend>(std::move(nativeStream)));
}

auto up::FileSystem::openWrite(zstring_view path, FileOpenMode mode) -> Stream {
    std::ofstream nativeStream(path.c_str(),
        mode == FileOpenMode::Binary ? std::ios_base::out | std::ios_base::trunc | std::ios_base::binary : std::ios_base::trunc | std::ios_base::out);
    if (!nativeStream) {
        return nullptr;
    }
    return Stream(up::new_box<NativeOutputBackend>(std::move(nativeStream)));
}

static auto errorCodeToResult(std::error_code ec) noexcept -> up::IOResult {
    if (!ec) {
        return up::IOResult::Success;
    }

    if (ec.category() == std::system_category()) {
        // FIXME: translate error codes
        return up::IOResult::System;
    }
    return up::IOResult::Unknown;
}

bool up::FileSystem::fileExists(zstring_view path) noexcept {
    [[maybe_unused]] std::error_code ec;
    return std::filesystem::is_regular_file(std::string_view(path.c_str(), path.size()), ec);
}

bool up::FileSystem::directoryExists(zstring_view path) noexcept {
    [[maybe_unused]] std::error_code ec;
    return std::filesystem::is_directory(std::string_view(path.c_str(), path.size()), ec);
}

auto up::FileSystem::fileStat(zstring_view path, FileStat& outInfo) -> IOResult {
    std::error_code ec;
    outInfo.size = std::filesystem::file_size(std::string_view(path.c_str(), path.size()), ec);
    outInfo.mtime = std::chrono::duration_cast<std::chrono::microseconds>(
        std::filesystem::last_write_time(std::string_view(path.c_str(), path.size()), ec).time_since_epoch())
                        .count();
    auto const status = std::filesystem::status(std::string_view(path.c_str(), path.size()), ec);
    outInfo.type = status.type() == std::filesystem::file_type::regular ? FileType::Regular
                                                                        : status.type() == std::filesystem::file_type::directory
            ? FileType::Directory
            : status.type() == std::filesystem::file_type::symlink ? FileType::SymbolicLink : FileType::Other;
    return errorCodeToResult(ec);
}

auto up::FileSystem::enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts) -> EnumerateResult {
    UP_ASSERT(!path.empty());

    auto iter = std::filesystem::recursive_directory_iterator(path.c_str());
    auto end = std::filesystem::recursive_directory_iterator();

    while (iter != end) {
        std::string genPath =
            ((opts & EnumerateOptions::FullPath) == EnumerateOptions::FullPath ? iter->path() : std::filesystem::relative(iter->path(), path.c_str()))
                .generic_string();

        EnumerateItem item;
        item.info.path = genPath.c_str();
        item.info.type = iter->is_regular_file()
            ? FileType::Regular
            : iter->is_directory() ? FileType::Directory : iter->is_symlink() ? FileType::SymbolicLink : FileType::Other;
        item.info.size = item.info.type == FileType::Regular ? iter->file_size() : 0;
        item.depth = iter.depth();

        auto result = cb(item);
        if (result == EnumerateResult::Break) {
            return result;
        }

        if (iter->is_directory() && result == EnumerateResult::Continue) {
            iter.disable_recursion_pending();
        }

        ++iter;
    }

    return EnumerateResult::Continue;
}

auto up::FileSystem::createDirectories(zstring_view path) -> IOResult {
    std::error_code ec;
    std::filesystem::create_directories(path.c_str(), ec);
    return errorCodeToResult(ec);
}

auto up::FileSystem::remove(zstring_view path) -> IOResult {
    std::error_code ec;
    if (!std::filesystem::remove(path.c_str(), ec)) {
        return IOResult::FileNotFound;
    }
    return errorCodeToResult(ec);
}

auto up::FileSystem::removeRecursive(zstring_view path) -> IOResult {
    std::error_code ec;
    std::filesystem::remove_all(path.c_str(), ec);
    return errorCodeToResult(ec);
}

auto up::FileSystem::currentWorkingDirectory() noexcept -> string {
    std::error_code ec;
    if (auto path = std::filesystem::current_path(ec).generic_string(); !ec) {
        return {path.data(), path.size()};
    }
    return string();
}

bool up::FileSystem::currentWorkingDirectory(zstring_view path) {
    std::error_code ec;
    std::filesystem::current_path(std::filesystem::path(path.c_str()), ec);
    return !ec;
}

auto up::FileSystem::copyFileTo(zstring_view fromPath, zstring_view toPath) -> IOResult {
    std::error_code ec;
    std::filesystem::copy_file(fromPath.c_str(), toPath.c_str(), std::filesystem::copy_options::overwrite_existing, ec);
    return errorCodeToResult(ec);
}

auto up::FileSystem::moveFileTo(zstring_view fromPath, zstring_view toPath) -> IOResult {
    std::error_code ec;
    std::filesystem::rename(fromPath.c_str(), toPath.c_str(), ec);
    return errorCodeToResult(ec);
}
