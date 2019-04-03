// Copyright (C) 2014,2015,2019 Sean Middleditch, all rights reserverd.

#pragma once

namespace up {

    template <typename T>
    inline T max(T const& lhs, T const& rhs) {
        return lhs < rhs ? rhs : lhs;
    }

    template <typename T>
    inline T min(T const& lhs, T const& rhs) {
        return lhs < rhs ? lhs : rhs;
    }

    template <typename IteratorT, typename SentinelT, typename FunctionT, typename ResultT, typename ProjT>
    ResultT accumulate(IteratorT first, SentinelT last, ResultT initial, FunctionT const& op, ProjT const& proj) {
        while (first != last) {
            initial = op(initial, proj(*first));
            ++first;
        }
        return initial;
    }

    template <typename IteratorT, typename SentinelT, typename FunctionT, typename ResultT>
    ResultT accumulate(IteratorT first, SentinelT last, ResultT initial, FunctionT const& op) {
        while (first != last) {
            initial = op(initial, *first);
            ++first;
        }
        return initial;
    }

} // namespace up
