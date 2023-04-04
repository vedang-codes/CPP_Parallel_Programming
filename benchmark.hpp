#ifndef comp3400_2020w_benchmark_hpp_
#define comp3400_2020w_benchmark_hpp_

//===========================================================================

#include <cstddef>
#include <chrono>
#include <list>
#include <type_traits>

#include "tally.hpp"

//===========================================================================

#if defined(__GNUC__)
inline void do_not_optimize_barrier()
{
  asm volatile("" ::: "memory"); // GCC specific
}
#else
#  error "Write a compiler-specific definition of do_not_optimize_barrier()!"
#endif

//===========================================================================

//
// tally benchmark(num_iters,op,args...)
// This function will call op(args...) num_iters times returning the tallied
// timing results.
//
template <typename Op, typename... Args>
constexpr tally benchmark(std::size_t const& num_iters, Op&& op, Args&&... args)
{
  using namespace std;
  using namespace std::chrono;

  tally t;
  for (size_t i{}; i != num_iters; ++i)
  {
    do_not_optimize_barrier();
    auto time0 = steady_clock::now();
    do_not_optimize_barrier();

    if constexpr(!is_void_v<decltype(op(args...))>)
      [[maybe_unused]] auto result = op(args...);
    else
      op(args...);

    do_not_optimize_barrier();
    auto time1 = steady_clock::now();
    do_not_optimize_barrier();
    t += duration<double>(time1-time0).count();
  }
  return t;
}

//===========================================================================

//
// struct benchmark_results<T, SeqContainer = std::list>
//
// This structure holds:
//   * the tallied benchmark results, and,
//   * the results returned from each function call being tallied
//
// If the tallied function call returns void, then no results are no stored
// in this structure.
//
template <
  typename T,
  template <typename> class SeqContainer = std::list
>
struct benchmark_results
{
  using cleaned_value_type = std::remove_cv_t<std::remove_reference_t<T>>;

  tally time;
  SeqContainer<cleaned_value_type> results;
};

// Partial specialization for void, i.e., to not store any results...
template <template <typename> class SeqContainer>
struct benchmark_results<void,SeqContainer>
{
  tally time;
};

//
// results_benchmark(num_iters,op,args...) ->
//  benchmark_results
// This function will call op(args...) num_iters times returning the results
// as a benchmark_results structure
//
template <typename Op, typename... Args>
constexpr auto results_benchmark(
  std::size_t const& num_iters, Op&& op, Args&&... args
) ->
  benchmark_results< std::invoke_result_t<Op, Args...> >
{
  using namespace std;
  using namespace std::chrono;

  benchmark_results< std::invoke_result_t<Op, Args...> > retval;
  for (size_t i{}; i != num_iters; ++i)
  {
    do_not_optimize_barrier();

    auto time0 = steady_clock::now();
    do_not_optimize_barrier();
    
    if constexpr(!is_void_v<decltype(op(args...))>)
      retval.results.push_back(op(args...));
    else
      op(args...);

    do_not_optimize_barrier();
    auto time1 = steady_clock::now();
    do_not_optimize_barrier();
    retval.time += duration<double>(time1-time0).count();
  }
  return retval;
}

//===========================================================================

template <typename T>
struct accumulated_benchmark_result
{
  using cleaned_value_type = std::remove_cv_t<std::remove_reference_t<T>>;

  tally time;
  cleaned_value_type result;
};

//
// accumulating_benchmark(num_iters,init,aop,op,args...) ->
//    accumulated_enchmark_results
// This function will call op(args...) num_iters times returning the results
// in an accumulated_benchmark_results structure where:
//   * init is the initial value of any accumulated results
//   * aop(a,b) is binary operator returning bool that accumulates the
//     previously accumulated value with the next to-be-accumulated value
//
template <typename T, typename AccumOp, typename Op, typename... Args>
constexpr auto accumulating_benchmark(
  std::size_t const& num_iters, T&& init, AccumOp aop, Op&& op, Args&&... args
) ->
  accumulated_benchmark_result< std::invoke_result_t<Op, Args...> >
{
  using namespace std;
  using namespace std::chrono;

  accumulated_benchmark_result< std::invoke_result_t<Op, Args...> > 
    retval{ {}, std::forward<T>(init) };

  for (size_t i{}; i != num_iters; ++i)
  {
    do_not_optimize_barrier();
    auto time0 = steady_clock::now();
    do_not_optimize_barrier();

    if constexpr(!is_void_v<decltype(op(args...))>)
      retval.result = aop(std::move(retval.result), op(args...));
    else
      op(args...);

    do_not_optimize_barrier();
    auto time1 = steady_clock::now();
    do_not_optimize_barrier();
    retval.time += duration<double>(time1-time0).count();
  }
  return retval;
}

template <typename T, typename Op, typename... Args>
constexpr auto accumulating_benchmark(
  size_t const& num_iters, T&& init, Op&& op, Args&&... args
)
{
  return accumulating_benchmark(
    num_iters,
    std::forward<T>(init), 
    [](auto&& lhs, auto&& rhs) 
    { 
      return lhs += std::forward<std::remove_reference_t<decltype(rhs)>>(rhs);
    },
    std::forward<Op>(op), std::forward<Args>(args)...
  );
}

//===========================================================================

#endif // #ifndef comp3400_2020w_benchmark_hpp_
