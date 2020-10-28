#ifndef PRINTABLE_VALUE_ADAPTER_HPP_INCLUDE_GUARD
#define PRINTABLE_VALUE_ADAPTER_HPP_INCLUDE_GUARD

#include <memory> // unique_ptr
#include <type_traits>

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

} // unnamed namespace

template<class T>
struct ttl::is_equality_comparable
: std::conjunction < ::is_detected_type<equality_compare_t, T, bool>
                   , ::is_detected_type<inequality_compare_t, T, bool> > {};
                   
namespace lib {

/******************************************************************************
* class lib::PrintableValueProtocol
******************************************************************************/
// This protocol class provides a pure abstract interface and contract which
// models objects having a value that can printed to 'stdout'.
// It provides a member function 'print' that prints the value of the object
// to 'stdout'. Two 'PrintableValueProtocol' objects have the same value if
// their respective output to 'stdout' is the same. Value semantics are mimicked 
// using 'unique_ptr': in particular copies are generated using the virtual
// copy idiom, and default construction is mimicked having a global instance
// 'PrintableValueProtocol::Default' whose value is "null" (it prints nothing).
// For ease of implementation, the type is (morally) only semiregular.
class PrintableValueProtocol
{
public:
  // Destroy this object.
  virtual ~PrintableValueProtocol() noexcept = 0;
  // Suppress assignment through abstract protocol.
  PrintableValueProtocol& operator = (PrintableValueProtocol const&) = delete;

  // Print this object to 'stdout'.
  void print() const;

  // Return a pointer having unique ownership of a 'PrintableValueProtocol'
  // object having the same value of this object.
  std::unique_ptr<PrintableValueProtocol> clone() const;

  // A 'PrintableValueProtocol' object whose value is "null". That is,
  // 'PrintableValueProtocol::Default.print()' prints nothing to 'stdout'.
  static PrintableValueProtocol const& Default;

  // Return a pointer having unique ownership of a 'PrintableValueProtocol'
  // object having "null" value.
  // Note this is equivalent to calling 'PrintableValueProtocol::Default.clone()'.
  static std::unique_ptr<PrintableValueProtocol> getDefault();

private: /* virtual implementation */
  // Implement the 'print' contract.
  virtual void print_impl() const = 0;
  // Implement the 'clone' contract.
  virtual std::unique_ptr<PrintableValueProtocol> clone_impl() const = 0;
};

// Return 'true' if the specified 'lhs' and 'rhs' have the same value,
// and 'false' otherwise. Two 'PrintableValueProtocol' objects have the same
// value if their respective outputs to 'stdout' are equal.
bool operator == (PrintableValueProtocol const& lhs,
                  PrintableValueProtocol const& rhs) = delete;

// Return 'true' if the specified 'lhs' and 'rhs' do not have the same value,
// and 'false' otherwise. Two 'PrintableValueProtocol' objects do not have the
// same value if their respective outputs to 'stdout' are not equal.
bool operator != (PrintableValueProtocol const& lhs,
                  PrintableValueProtocol const& rhs) = delete;

/******************************************************************************
* class lib::PrintableValueAdapter
******************************************************************************/
// This class provides an "adapter" from the specified 'T' to the
// 'lib::PrintableValueProtocol' abstract protocol. The adapter reroutes the
// implementation through a specific customization point, 'print_cp'.
// 'T' must be semiregular and have value-semantics.
template <class T>
class PrintableValueAdapter : public lib::PrintableValueProtocol
{
  static_assert(ttl::is_semiregular_v<T>, "T must be semiregular");

  T impl_; // concrete implementation to be adapted.
public:
  // Create an adapter to the 'lib::PrintableValueProtocol' protocol using
  // the specified 'obj'.
  /*implicit*/ PrintableValueAdapter(T obj);

private:
  // Implement the 'lib::PrintableValueProtocol' protocol by forwarding the
  // call to the customization point 'lib::PrintableValueAdapter_print_cp'.
  void print_impl() const override;
  // Implement the 'lib::PrintableValueProtocol' protocol by copying the
  // adapted object.
  std::unique_ptr<PrintableValueProtocol> clone_impl() const override;

private: // Customization point for 'lib::PrintableValueAdapter'
  // Clients are free to specialize the following function template for
  // their types, to match the behaviour of their respective contracts.

  // Print the specified 'obj' to 'stdout'.
  // Note that unless specialized, 'fmt::print("{}", obj)' is returned.
  static void print_cp(T const& obj);
};

} // namespace lib

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////
#include <utility> // move
#include <fmt/core.h>

lib::PrintableValueProtocol::~PrintableValueProtocol() noexcept = default;

void lib::PrintableValueProtocol::print() const {
  return print_impl();
}

auto lib::PrintableValueProtocol::clone() const
-> std::unique_ptr<lib::PrintableValueProtocol> {
  return clone_impl();
}

namespace {

// This class provides an implementation of the 'lib::PrintableValueProtocol'
// protocol. It models a "null" value, that prints nothing to 'stdout'.
class NullPrintableValue : public lib::PrintableValueProtocol
{
  // Do nothing.
  void print_impl() const final;
  // Return a pointer having unique ownership of a 'NullPrintableValue' object.
  std::unique_ptr<lib::PrintableValueProtocol> clone_impl() const final;
};

void NullPrintableValue::print_impl() const {
 // Intentionally blank.
}

auto NullPrintableValue::clone_impl() const
-> std::unique_ptr<lib::PrintableValueProtocol> {
  return std::make_unique<NullPrintableValue>();
}

} // unnamed namespace

lib::PrintableValueProtocol const& lib::PrintableValueProtocol::Default
  = ::NullPrintableValue{};

 auto lib::PrintableValueProtocol::getDefault()
 -> std::unique_ptr<lib::PrintableValueProtocol> {
  return lib::PrintableValueProtocol::Default.clone();
}

template<class T>
lib::PrintableValueAdapter<T>::PrintableValueAdapter(T obj)
: impl_(std::move(obj))
{ }

template<class T>
void lib::PrintableValueAdapter<T>::print_impl() const
{
  print_cp(impl_);
}

template<class T>
auto lib::PrintableValueAdapter<T>::clone_impl() const
-> std::unique_ptr<PrintableValueProtocol>
{
  return std::make_unique<lib::PrintableValueAdapter<T>>(impl_);
}

template<class T>
void lib::PrintableValueAdapter<T>::print_cp(T const& obj)
{
  fmt::print("{}", obj);
}

#endif // PRINTABLE_VALUE_ADAPTER_HPP_INCLUDE_GUARD
