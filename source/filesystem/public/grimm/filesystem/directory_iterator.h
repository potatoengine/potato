// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/zstring_view.h"
#include <iterator>

namespace gm::fs {
    class DirectoryIteratorBackend {
    public:
        DirectoryIteratorBackend(DirectoryIteratorBackend const&) = delete;
        DirectoryIteratorBackend& operator=(DirectoryIteratorBackend const&) = delete;

        virtual ~DirectoryIteratorBackend() = default;

        virtual bool next() = 0;
        virtual bool done() const noexcept = 0;
        virtual zstring_view current() const noexcept = 0;

    protected:
        DirectoryIteratorBackend() = default;
    };

    class DirectoryIterator {
    public:
        class sentinel {
        };

        class iterator {
        public:
            using value_type = zstring_view;
            using iterator_category = std::input_iterator_tag;

            iterator() = delete;
            iterator(DirectoryIterator& iter) : _iter(&iter) {}

            iterator& operator++() {
                _iter->next();
                return *this;
            }

            zstring_view operator*() const noexcept {
                return _iter->current();
            }

            bool operator==(sentinel rhs) const noexcept {
                return _iter->done();
            }

        private:
            DirectoryIterator* _iter = nullptr;
        };

        explicit DirectoryIterator(box<DirectoryIteratorBackend> backend) : _backend(std::move(backend)) {}

        DirectoryIterator& operator++() {
            _backend->next();
        }

        iterator begin() noexcept { return iterator(*this); }
        sentinel end() noexcept { return sentinel(); }

        bool next() {
            if (!_backend->next()) {
                _backend.reset();
                return false;
            }
            return true;
        }

        bool done() const noexcept { return _backend != nullptr; }

        zstring_view current() const noexcept { return _backend->current(); }

    private:
        box<DirectoryIteratorBackend> _backend;
    };
} // namespace gm::fs
