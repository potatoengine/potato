#include <cmath>

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
            for (int i = 0; i != rhs.value.component_length; ++i) {
                if (!(lhs[i] == ExpectFloat{rhs.value[i]})) {
                    return false;
                }
            }
            return true;
        }

        operator T() const { return value; }
    };
} // namespace
