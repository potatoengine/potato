// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "traits.h"
#include <iterator>

namespace gm {
    template <typename IteratorT, typename SentinelT = IteratorT>
    class iterator_range;
    template <typename IteratorT, typename SentinelT>
    iterator_range<IteratorT, SentinelT> const& make_range(iterator_range<IteratorT, SentinelT> const& range);
    template <typename IteratorT, typename SentinelT>
    iterator_range<IteratorT, SentinelT> make_range(std::pair<IteratorT, SentinelT> const& range);
    template <typename IteratorT, typename SentinelT>
    iterator_range<IteratorT, SentinelT> make_range(std::pair<IteratorT, SentinelT>&& range);
    template <typename IteratorT, typename SentinelT>
    iterator_range<IteratorT, SentinelT> make_range(IteratorT iterator, SentinelT sentinel);
    template <typename ContainerT, typename IteratorT = decltype(std::declval<ContainerT>().begin()), typename SentinelT = decltype(std::declval<ContainerT>().end())>
    gm::iterator_range<IteratorT, SentinelT> make_range(ContainerT&& container);
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

/// Helper function for make_range to be an identity function over iterator_ranges.
template <typename IteratorT, typename SentinelT>
gm::iterator_range<IteratorT, SentinelT> const& gm::make_range(iterator_range<IteratorT, SentinelT> const& range) {
    return range;
}

template <typename IteratorT, typename SentinelT>
gm::iterator_range<IteratorT, SentinelT> gm::make_range(std::pair<IteratorT, SentinelT> const& range) {
    return iterator_range<IteratorT, SentinelT>(range);
}

template <typename IteratorT, typename SentinelT>
gm::iterator_range<IteratorT, SentinelT> gm::make_range(std::pair<IteratorT, SentinelT>&& range) {
    return iterator_range<IteratorT, SentinelT>(std::move(range));
}

template <typename IteratorT, typename SentinelT>
gm::iterator_range<IteratorT, SentinelT> gm::make_range(IteratorT iterator, SentinelT sentinel) {
    return iterator_range<IteratorT, SentinelT>(iterator, sentinel);
}

template <typename ContainerT, typename IteratorT, typename SentinelT>
gm::iterator_range<IteratorT, SentinelT> gm::make_range(ContainerT&& container) {
    return iterator_range<IteratorT, SentinelT>(container.begin(), container.end());
}
