#ifndef IS_PRINTABLE_FMT_HPP_INCLUDE_GUARD
#define IS_PRINTABLE_FMT_HPP_INCLUDE_GUARD

// This file defines a type trait, 'ttl::is_regular', based on the C++20
// 'regular' concept. It additionally provide related type traits used
// to implement 'ttl::is_regular', such as 'ttl::is_copyable' and
// 'ttl::is_equality_comparable'.
// It also provides a 'lib::printable' concept, modeled by semiregular types
// having value semantics, that can be printed to 'stdout' using 'lib::print'
// (defined below).
// Note that this is a simplified implementation, not fully conformant.

#include <type_traits>
#include <string>
#include <fmt/core.h>

namespace ttl { // type traits library

// Movable
template<class T>
using is_movable = std::conjunction < std::is_object<T>
                                    , std::is_move_constructible<T>
                                    , std::is_assignable<T&, T>
                                    , std::is_swappable<T> >;
// Copyable
template<class T>
using is_copyable = std::conjunction < ttl::is_movable<T>
                                     , std::is_copy_constructible<T>
                                     , std::is_assignable<T&, T const&>
                                     , std::is_assignable<T&, T&>
                                     , std::is_assignable<T&, T> >;
// Semiregular
template<class T>
using is_semiregular = std::conjunction < ttl::is_copyable<T>
                                        , std::is_default_constructible<T> >;

// Equality Comparable
template<class T>
struct is_equality_comparable; // Implemented below

// Regular
template<class T>
using is_regular = std::conjunction < ttl::is_semiregular<T>
                                    , ttl::is_equality_comparable<T> >;

// Helper types and variables
template<class T> using is_movable_t             = typename is_movable<T>::type;
template<class T> using is_copyable_t            = typename is_copyable<T>::type;
template<class T> using is_semiregular_t         = typename is_semiregular<T>::type;
template<class T> using is_equality_comparable_t = typename is_equality_comparable<T>::type;
template<class T> using is_regular_t             = typename is_regular<T>::type;

template<class T> constexpr bool is_movable_v             =  is_movable<T>::value;
template<class T> constexpr bool is_copyable_v            =  is_copyable<T>::value;
template<class T> constexpr bool is_semiregular_v         =  is_semiregular<T>::value;
template<class T> constexpr bool is_equality_comparable_v =  is_equality_comparable<T>::value;
template<class T> constexpr bool is_regular_v             =  is_regular<T>::value;

} // namespace ttl

namespace lib {

// Invokable class that prints objects of type 'T' to 'stdout'.
// This class is a customization point, clients are free to specialize it as needed.
template<typename T>
struct formatter // class template to support partial specialization
{
  // Print the specified 'obj' to 'stdout'.
  // Unless specialized, call 'fmt::format({}, obj)'.
  std::string operator () (T const& obj) {
    return fmt::format("{}", obj);
  }
};

// Return a representation of 'obj' as a formatted string.
template<typename T>
std::string format(T const& obj) {
  return lib::formatter<T>{}(obj);
}

// Print the specified 'obj' to 'stdout'.
template<typename T>
void print(T const& obj) {
  fmt::print("{}", lib::format(obj));
}

// 'lib::printable' concept
template<class T>
struct is_printable; // Implemented below

template<class T> using is_printable_t = typename is_printable<T>::type;
template<class T> constexpr bool is_printable_v =  is_printable<T>::value;

} // namespace lib

// Implementation /////////////////////////////////////////////////////////////

namespace {

// Detection idiom
template<template<class> class expression, class T, class = void>
struct detector : std::false_type {};

template<template<class> class expression, class T>
struct detector< expression, T, std::void_t< expression<T> > >
: std::true_type {};

template<template<class> class expression, class T>
using is_detected = detector<expression, T>;

template<template<class> class expression, class T, class R, class = void>
struct type_detector : std::false_type {};

template<template<class> class expression, class T, class R>
struct type_detector< expression, T, R, std::void_t< expression<T> > >
: std::is_same<expression<T>, R> {};

template<template<class> class expression, class T, class R>
using is_detected_type = type_detector<expression, T, R>;

// Expressions for equality comparison

template<class T>
using equality_compare_t   = decltype(std::declval<T const&>() == std::declval<T const&>());
template<class T>
using inequality_compare_t = decltype(std::declval<T const&>() != std::declval<T const&>());

template<class T>
using lib_print_t = decltype(lib::print(std::declval<T const&>()));

} // unnamed namespace

template<class T>
struct ttl::is_equality_comparable
: std::conjunction < ::is_detected_type<equality_compare_t, T, bool>
                   , ::is_detected_type<inequality_compare_t, T, bool> > {};

template<class T>
struct lib::is_printable
: std::conjunction < ttl::is_semiregular<T>
                   , ::is_detected_type<lib_print_t, T, void> > {};

#endif // IS_PRINTABLE_FMT_HPP_INCLUDE_GUARD
