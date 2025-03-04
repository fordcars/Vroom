#pragma once

#include <atomic>

namespace Utils {
template <typename Derived>
class Identifiable {
public:
    static inline std::atomic<std::size_t> nextId{0};
    const std::size_t id;

    Identifiable() : id(nextId.fetch_add(1, std::memory_order_relaxed)) {}
};
} // namespace Utils
