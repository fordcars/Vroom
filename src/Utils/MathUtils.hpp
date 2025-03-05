#pragma once

#include <cmath>

namespace Utils {
inline bool floatsEqualish(float a, float b, float margin) {
    return fabs(a - b) < margin;
}
} // namespace Utils
