#ifndef comp3400_2020w_utils_hpp_
#define comp3400_2020w_utils_hpp_

//===========================================================================

#include <bit>
#include <limits>
#include <concepts>
#include <stdexcept>

//===========================================================================

//
// isqrt(num)
// This function efficiently computes the integer square root of num using
// Newton's Method.
//
// References:
//   * Warren, Henry S. (2013). Hacker's Delight, Second Edition.
//     Addison-Wesley: Upper Saddle River, NJ.
//     ISBN-13: 978-0-321-84268-8; 2013
//       See Section 11-1 for discussion on integer square root 
//       computations and Figure 11-1 for similar 32-bit code.
//   * https://en.wikipedia.org/wiki/Newton%27s_method
//
template <typename Int>
requires
  std::numeric_limits<Int>::is_integer &&
  // i.e., Int must be an integer type
  (requires (Int i) { { countl_zero(i) } -> std::convertible_to<Int>; } ||
   requires (Int i) { { std::countl_zero(i) } -> std::convertible_to<Int>; })
  // i.e., there must be a definition of countl_zero(i) returning a
  //       type that can be implicitly converted to Int.
constexpr Int isqrt(Int const& n)
{
  using std::countl_zero;
  using std::numeric_limits;

  if constexpr(!numeric_limits<Int>::is_signed)
  {
    // n is unsigned...
    if (n <= 1) return n;
  } else
  {
    // n is signed, so if negative throw exception...
    if (n < 0) throw std::domain_error("isqrt() passed negative value");
    else if (n <= 1) return n;
  }

  // At this point the n is never negative.

  // Use the number of leading zeros to obtain first guess...
  auto const shift = numeric_limits<Int>::digits - countl_zero(n-1)/2;
  Int guess0 = Int(1) << shift;
  Int guess1 = (guess0 + (n >> shift)) >> 1;
  // NOTE: Since guess0 starts off with a power of two, a right shift instead
  //       of a division can be used to compute the initial value of guess1.

  while (guess1 < guess0)
  {
    guess0 = guess1;
    guess1 = (guess0 + n/guess0) >> 1; // i.e., ">> 1" is "/2"
  }
  return guess0;
}

//===========================================================================

#endif // #ifndef comp3400_2020w_utils_hpp_
