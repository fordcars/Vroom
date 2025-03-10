#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string>

namespace Utils {

template <std::size_t N>
struct StringLiteral {
    std::array<char, N> value;

    constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value.begin()); }
    constexpr auto operator<=>(const StringLiteral&) const = default; // For NTTP

    template <std::size_t M>
    constexpr bool operator==(const StringLiteral<M>& other) const {
        return std::equal(value.cbegin(), value.cend(), other.value.cbegin(),
                          other.value.cend());
    }
};

template <StringLiteral... Keys>
class StringIndexor {
public:
    static consteval std::size_t size() { return sizeof...(Keys); }

    // Get name from index
    template <std::size_t Index>
    static consteval const char* get() {
        static_assert(Index < sizeof...(Keys), "Index out of bounds");

        std::size_t currIndex = 0;
        const char* out = "";
        (..., (currIndex++ == Index ? out = Keys.value : ""));
        return out;
    }

    // Get index from name
    template <StringLiteral Key>
    static consteval std::size_t get() {
        constexpr std::size_t index = [] {
            std::size_t foundIndex = sizeof...(Keys);
            std::size_t currIndex = 0;
            (..., (Key == Keys ? (foundIndex = currIndex++) : currIndex++));
            return foundIndex;
        }();

        static_assert(index < sizeof...(Keys), "Key not found");
        return index;
    }

    // Get name from index
    // Only use this if the index is only known at runtime
    static std::optional<const char*> runtimeGet(std::size_t index) {
        std::size_t currIndex = 0;
        std::optional<const char*> out;
        (..., (currIndex++ == index ? out = &Keys.value[0] : ""));
        return out;
    }

    // Get index from name
    // Only use this if the key is only known at runtime
    static std::optional<std::size_t> runtimeGet(const std::string& key) {
        std::optional<std::size_t> index;
        std::size_t currIndex = 0;
        (..., (key == std::string(&Keys.value[0]) ? index = currIndex++ : currIndex++));
        return index;
    }
};

} // namespace Utils
