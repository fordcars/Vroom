#pragma once

#include <cmath>

namespace Utils {
inline bool floatsEqualish(float a, float b, float margin = 0.0001) {
    return fabs(a - b) < margin;
}
} // namespace Utils
