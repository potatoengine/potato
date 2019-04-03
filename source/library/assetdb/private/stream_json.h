// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/stream.h"

namespace up {
    class RapidJsonStreamWrapper {
    public:
        using Ch = char;

        RapidJsonStreamWrapper(fs::Stream& stream) : _stream(stream) {}

        void Put(char c) {
            byte const b = reinterpret_cast<byte&>(c);
            _stream.write(span{&b, 1});
        }

        void Flush() {
            _stream.flush();
        }

        char Peek() {
            if (_peeked) {
                return _peekChar;
            }

            span<byte> read(reinterpret_cast<byte*>(&_peekChar), 1);
            _stream.read(read);
            _peeked = !read.empty();
            return _peeked ? _peekChar : 0;
        }

        char Take() {
            Peek();

            if (_peeked) {
                _peeked = false;
                return _peekChar;
            }
            else {
                return 0;
            }
        }

        fs::Stream::difference_type Tell() const {
            return _stream.tell();
        }

        char* PutBegin() { return nullptr; }
        fs::Stream::difference_type PutEnd(char*) { return 0; }

    private:
        fs::Stream& _stream;
        char _peekChar = 0;
        bool _peeked = false;
    };
} // namespace up
