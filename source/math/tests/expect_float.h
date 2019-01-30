#pragma once

#include <cmath>
#include "stream_vector.h"
#include "grimm/math/common.h"

namespace {
    template <typename T>
    struct ExpectFloat {
        T value;

        ExpectFloat(T v) : value(v) {}

        friend bool operator==(T lhs, ExpectFloat rhs) {
            // FIXME: calculate appropriate epsilon scale vs a minimum tolerance
            // https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
            return std::fabs(lhs - rhs.value) < (T)0.001;
        }

        operator T() const { return value; }
    };

    template <typename T>
    struct ExpectVector {
        T value;

        ExpectVector(T v) : value(v) {}

        friend bool operator==(T lhs, ExpectVector rhs) {
            for (int i = 0; i != gm::component_length_v<decltype(rhs.value)>; ++i) {
                if (!(lhs[i] == ExpectFloat{rhs.value[i]})) {
                    return false;
                }
            }
            return true;
        }

        operator T() const { return value; }

        friend std::ostream& operator<<(std::ostream& os, ExpectVector const& val) {
            return os << val.value;
        }
    };
} // namespace
