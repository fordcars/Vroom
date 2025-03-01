#include <optional>
#include <tuple>

// Equivalent to std::get<T>() on a tuple, but if T is an std::optional<T>,
// we return a std::optional<std::reference_wrapper<T>>, whether T is
// present in the tuple or not.

namespace Utils {

// Helper classes for checking if tuple has type.
// Is true if Tuple contains type T, OR if T is an std::optional.
// Inspired by: https://stackoverflow.com/a/25958302/6222104
namespace TupleContainsOptionalTypeInternal {
template <typename T, typename Tuple>
struct has_type;

// If T is std::optional, be true
template <typename T, typename... Ts>
struct has_type<std::optional<T>, std::tuple<Ts...>> : std::true_type {};

// Empty tuple (we went through entire tuple with no matches)
template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

// Recursive case: check the rest of the tuple
template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

// First type of tuple is T
template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};
}  // namespace TupleContainsOptionalTypeInternal

template <typename T, typename TupleT>
using TupleContainsOptionalType =
    typename TupleContainsOptionalTypeInternal::has_type<T, TupleT>::type;

namespace OptionalTupleGetterInternal {

// Getter for non-optional types
template <typename T>
struct Getter {
    template <typename TupleT>
    static T& get(TupleT& t) {
        return std::get<T>(t);
    }

    template <typename TupleT>
    static const T& get(const TupleT& t) {
        return std::get<T>(t);
    }
};

// Getter for optional types
template <typename T>
struct Getter<std::optional<T>> {
    template <typename TupleT>
        requires TupleContainsOptionalType<T, TupleT>::value
    static std::optional<std::reference_wrapper<T>> get(TupleT& t) {
        return std::get<T>(t);
    }

    template <typename TupleT>
    static std::optional<std::reference_wrapper<T>> get(TupleT& t) {
        return std::nullopt;
    }

    template <typename TupleT>
        requires TupleContainsOptionalType<T, TupleT>::value
    static std::optional<std::reference_wrapper<const T>> get(const TupleT& t) {
        return std::get<T>(t);
    }

    template <typename TupleT>
    static std::optional<std::reference_wrapper<const T>> get(const TupleT& t) {
        return std::nullopt;
    }
};

}  // namespace OptionalTupleGetterInternal

template <typename T>
using OptionalTupleGetter = OptionalTupleGetterInternal::Getter<T>;

}  // namespace Utils
