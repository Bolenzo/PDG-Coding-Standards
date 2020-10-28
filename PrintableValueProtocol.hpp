#ifndef PRINTABLE_VALUE_PROTOCOL_HPP_INCLUDE_GUARD
#define PRINTABLE_VALUE_PROTOCOL_HPP_INCLUDE_GUARD

#include <memory> // unique_ptr

namespace lib {

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

} // namespace lib

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////

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

#endif // PRINTABLE_VALUE_PROTOCOL_HPP_INCLUDE_GUARD
