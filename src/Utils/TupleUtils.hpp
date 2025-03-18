#pragma once

#include <optional>
#include <tuple>

namespace Utils {

// Helper classes for checking if tuple has type.
// Is true if Tuple contains type T, if Tuple contains type Derived : public T, OR if T is
// an std::optional.
// Inspired by: https://stackoverflow.com/a/25958302/6222104
namespace TupleContainsTypeInternal {
template <typename T, typename Tuple>
struct has_type;

// Empty tuple (we went through entire tuple with no matches)
template <typename T>
struct has_type<T, std::tuple<>> : std::false_type {};

// First type of tuple is T
template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

// If T is std::optional, be true
template <typename T, typename TupleT>
struct has_type<std::optional<T>, TupleT> : std::true_type {};

// Recursive case: check the rest of the tuple
// Essentially, this specialization is chosen if first
// type of tuple doesn't match; we check if the first type
// derives from the type we're looking for, otherwise we
// continue checking the rest of the tuple.
template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>>
    : std::conditional_t<std::is_base_of_v<T, U>, std::true_type,
                         has_type<T, std::tuple<Ts...>>> {};
} // namespace TupleContainsTypeInternal

template <typename T, typename TupleT>
using TupleContainsType = typename TupleContainsTypeInternal::has_type<T, TupleT>::type;

// Same as std::get<T>(tuple), but supports derived types.
// https://stackoverflow.com/a/34002368/6222104
namespace TupleGetDerived {
template <typename Base, typename Tuple, std::size_t I = 0>
struct tuple_ref_index;

template <typename Base, typename Head, typename... Tail, std::size_t I>
struct tuple_ref_index<Base, std::tuple<Head, Tail...>, I>
    : std::conditional<std::is_base_of<Base, Head>::value,
                       std::integral_constant<std::size_t, I>,
                       tuple_ref_index<Base, std::tuple<Tail...>, I + 1>>::type {};

template <typename Base, typename Tuple>
auto get(Tuple&& tuple)
    -> decltype(std::get<tuple_ref_index<Base, typename std::decay<Tuple>::type>::value>(
        std::forward<Tuple>(tuple))) {
    return std::get<tuple_ref_index<Base, typename std::decay<Tuple>::type>::value>(
        std::forward<Tuple>(tuple));
}
} // namespace TupleGetDerived

// Similar to std::get<T>() on a tuple, but if T is an std::optional<T>,
// we return a std::optional<std::reference_wrapper<T>>, whether T is
// present in the tuple or not.
// Supports inherited types.
namespace OptionalTupleGetterInternal {
// Getter for non-optional types
template <typename T>
struct Getter {
    template <typename TupleT>
    static T& get(TupleT& t) {
        return TupleGetDerived::get<T>(t);
    }

    template <typename TupleT>
    static const T& get(const TupleT& t) {
        return TupleGetDerived::get<T>(t);
    }
};

// Getter for optional types
template <typename T>
struct Getter<std::optional<T>> {
    template <typename TupleT>
        requires TupleContainsType<T, TupleT>::value
    static std::optional<std::reference_wrapper<T>> get(TupleT& t) {
        return TupleGetDerived::get<T>(t);
    }

    template <typename TupleT>
    static std::optional<std::reference_wrapper<T>> get(TupleT& t) {
        return std::nullopt;
    }

    template <typename TupleT>
        requires TupleContainsType<T, TupleT>::value
    static std::optional<std::reference_wrapper<const T>> get(const TupleT& t) {
        return TupleGetDerived::get<T>(t);
    }

    template <typename TupleT>
    static std::optional<std::reference_wrapper<const T>> get(const TupleT& t) {
        return std::nullopt;
    }
};

} // namespace OptionalTupleGetterInternal

template <typename T>
using OptionalTupleGetter = OptionalTupleGetterInternal::Getter<T>;

} // namespace Utils
