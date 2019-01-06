// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <iterator>

namespace gm {
    template <typename IteratorT, typename SentinelT = IteratorT>
    class iterator_range;

    template <typename IteratorT, typename SentinelT>
    iterator_range(std::pair<IteratorT, SentinelT> const&)->iterator_range<std::remove_cv_t<IteratorT>, std::remove_cv_t<SentinelT>>;

    template <typename IteratorT, typename SentinelT>
    iterator_range(std::pair<IteratorT, SentinelT>&&)->iterator_range<std::remove_cv_t<IteratorT>, std::remove_cv_t<SentinelT>>;

    template <typename ContainerT>
    iterator_range(ContainerT const& c)->iterator_range<decltype(c.begin()), decltype(c.end())>;
} // namespace gm

/// This is a range over two iterators (or an iterator and a sentinel).
///
/// A range is very similar to a std::pair of iterators, except it also supports methods .empty(), .front(), and .pop_front().
/// The limits of the range are accessed via .begin() and .end(), which means they also work with range-based for.
template <typename IteratorT, typename SentinelT>
class gm::iterator_range {
    IteratorT _begin;
    SentinelT _end;

public:
    using iterator = IteratorT;
    using sentinel = SentinelT;
    using value_type = typename std::iterator_traits<IteratorT>::value_type;
    using reference = typename std::iterator_traits<IteratorT>::reference;
    using size_type = typename std::iterator_traits<IteratorT>::difference_type;

    iterator_range() = default;

    template <typename BeginT, typename EndT>
    iterator_range(BeginT&& begin, EndT&& end)
        : _begin(std::forward<BeginT>(begin)), _end(std::forward<EndT>(end)) {}

    template <typename BeginT, typename EndT>
    /*implicit*/ iterator_range(std::pair<BeginT, EndT> const& range)
        : _begin(range.first), _end(range.second) {}

    template <typename BeginT, typename EndT>
    /*implicit*/ iterator_range(std::pair<BeginT, EndT>&& range)
        : _begin(std::move(range.first)), _end(std::move(range.second)) {}

    template <typename ContainerT>
    /*implicit*/ iterator_range(ContainerT const& cont)
        : _begin(cont.begin()), _end(cont.end()) {}

    template <typename = decltype(_begin[0])>
    decltype(auto) operator[](size_type index) const { return _begin[index]; }

    bool empty() const { return !(_begin != _end); }
    explicit operator bool() const { return _begin != _end; }

    template <typename = decltype(_end - _begin)>
    size_type size() const { return _end - _begin; }

    decltype(auto) front() const { return *_begin; }
    template <typename = decltype(_end[-1])>
    decltype(auto) back() const { return *(_end - 1); }

    void pop_front() { ++_begin; }
    template <typename = decltype(--_end)>
    void pop_back() { --_end; }

    iterator begin() const { return _begin; }
    sentinel end() const { return _end; }
};

template <typename IteratorT, typename SentinelT>
struct gm::is_range<gm::iterator_range<IteratorT, SentinelT>> : std::true_type {};
