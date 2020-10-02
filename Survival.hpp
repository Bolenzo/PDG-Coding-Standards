#ifndef SURVIVAL_HPP_INCLUDE_GUARD
#define SURVIVAL_HPP_INCLUDE_GUARD

namespace pdg {

// Convience aliases
using Probability = double;
using Time = double;

// Type of exception thrown.
struct computation_error {};

// This protocol class provides a pure abstract interface and contract which
// allows to query the survival probability of a system at a future time.
// The contract requires that the survival probability function 'S: [0,+\infty) -> [0,1]'
// is monotonically decreasing, with 'S(0) = 1'; moreover it must be right-continuous
// and have a finite number of points in which it is discountinuous. 
// A related functionality computing the conditional survival probability is available.
// An additional virtual interface is provided: the instantaneous hazard rate
// 'h: [0,+\infty) -> R' expresses the expected frequency (over a unit of time) of failure
// of the system at a future time; it is equal to the first order derivative of the survival
// probability function 'S', with opposite sign and rescaled by 'S' itself, or
// 'h(T) = - S'(T) / S(T)' in formulas; note that the the right-derivative of 'S',
// which is well defined, is used.
class Survival
{
  // No data members!
public:
  // Destroy this object.
  virtual ~Survival( ) noexcept = 0;
  // Suppress assignment through abstract protocol.
  Survival& operator = (Survival const&) = delete;
  
  // Return the probability 'S(T)' that this system will survive until at least
  // the specified 'T'; see the class description for further informations on the
  // analytic properties of the function. An exception of type 'pdg::computation_error'
  // is thrown if the computation fails, for example because a negative probability
  // would be returned. The behaviour is undefined unless '0 <= T'.   
  pdg::Probability survival_prob(pdg::Time const& T) const;

  // Return the probability 'S(T|t)' that this system will survive until at least
  // the specified 'T', conditional to it surviving until at least the specified 't'.
  // It is equivalent to calling 'survival_prob(T) / survival_prob(t)'.
  // An exception of type 'pdg::computation_error' is thrown if the computation fails;
  // in particular this happens if 'survival_prob(t) == 0'.
  // The behaviour is undefined unless '0 <= t <= T'.
  pdg::Probability conditional_survival_prob(pdg::Time const& T,
                                             pdg::Time const& t) const;

  // Return the instantaneous hazard rate 'h' of this system at the specified
  // future time 'T'; see the class description for further informations on
  // the analytic properties of the function. The result is guaranteed to be greater
  // than or equal to '0'. A 'pdg::computation_error' is thrown if the computation fails;
  // this includes the case where the result would be 'NaN': in particular this happens
  // if 'survival_prob(T) == 0'. If the survival probability has a cusp in 'T',
  // then '+infty' is returned. The behaviour is undefined unless 'T >= 0'.
  double hazard_rate(pdg::Time const& T) const;

private:
  // Implement the 'survival_prob' contract.
  virtual pdg::Probability survival_prob_impl(pdg::Time const& T) const = 0;
  // Implement the 'hazard_rate' contract.
  virtual double hazard_rate_impl(pdg::Time const& T) const = 0;
protected:
  // Implement the 'conditional_survival_prob' contract. A default implementation
  // calling 'S(T)/S(t)' is supplied, and classes implementing this protocol can
  // opt into it by calling 'Survival::conditional_survival_prob_impl(T, t)' if
  // they cannot provide a more efficient implementation.
  virtual pdg::Probability conditional_survival_prob_impl(
                             pdg::Time const& T, pdg::Time const& t) const = 0;
};

} // namespace pdg

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////
#include <cassert>

pdg::Survival::~Survival() noexcept = default;

pdg::Probability pdg::Survival::survival_prob(pdg::Time const& T) const
{
  assert( T >= 0 );
  return survival_prob_impl(T);
}

pdg::Probability pdg::Survival::conditional_survival_prob(
                                pdg::Time const& T, pdg::Time const& t) const
{
  assert( 0 <= t      );
  assert(      t <= T );
  return conditional_survival_prob_impl(T, t);
}

double pdg::Survival::hazard_rate(pdg::Time const& T) const
{
  assert( T >= 0 );
  return hazard_rate_impl(T);
}

pdg::Probability pdg::Survival::conditional_survival_prob_impl(
                                pdg::Time const& T, pdg::Time const& t) const
{
  assert( 0 <= t      );
  assert(      t <= T );
  auto const S_t = survival_prob(t);
  if (S_t == 0) throw computation_error{};
  return survival_prob(T) / S_t;
}

#endif // SURVIVAL_HPP_INCLUDE_GUARD
