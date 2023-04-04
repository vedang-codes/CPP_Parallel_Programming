#include <algorithm>
#include <execution>
#include <limits>
#include <random>
#include <vector>
#include <sstream>
#include <iostream>
#include "benchmark.hpp"
#include "utils.hpp"

using namespace std;

// a function given by the professor to calculate if the given argument is prime or not
constexpr bool is_prime_trial_division(unsigned const &n)
{
    using std::sqrt;

    if (n < 2)
        return false;

    auto const sqrt_n = isqrt(n);
    bool retval = true;
    for (unsigned i = 2; i <= sqrt_n && retval; ++i)
        retval &= (n % i != 0);
    return retval;
}

template <typename ExecPolicy, typename Container>
auto do_work(ExecPolicy &ep, Container const &v)
{
    // this results stroring the result from the function results_benchmark
    auto results = results_benchmark(3, [&]()
                                     { return std::count_if(ep, begin(v), end(v), is_prime_trial_division); });
    //  printing out the time taken
    std::cout << "\ttime: " << results.time << " seconds\n"
              << "\tresults: ";
    //   printing out the results
    std::copy(begin(results.results), end(results.results), ostream_iterator<typename decltype(results.results)::value_type>(std::cout, " "));
    std::cout << '\n';

    // returning the results
    return results;
}

int main(int argc, char const *argv[])
{
    unsigned N = 30;
    random_device::result_type SEED;

    // if there are not 3 arguments perform the below task
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <number-of-integers> <random_seed>\n";
        return 1;
    }

    // if there are 3 arguments then perform the below task
    if (argc == 3)
    {
        istringstream buf(argv[1]);
        if (unsigned tmp; buf >> tmp)
            N = tmp;
        else
        {
            std::cerr << "Invalid integer value.\n";
            return 2;
        }

        istringstream buf2(argv[2]);
        if (random_device::result_type tmp; buf2 >> tmp)
            SEED = tmp;
        else
        {
            std::cerr << "Invalid random seed.\n";
            return 3;
        }
    }

    std::default_random_engine re(SEED);
    std::uniform_int_distribution<unsigned> ud(2, std::numeric_limits<unsigned>::max());

    // initializing the vector to store the random numbers
    std::vector<unsigned> v;

    v.reserve(N);

    // generating the 30 random numbers
    generate_n(
        back_inserter(v),
        N,
        [&]()
        { return ud(re); });

    // printing out the sequential results and time
    std::cout << "sequential results:\n";
    auto sequential = do_work(std::execution::seq, v);

    // printing out the parallel results and time
    std::cout << "parallel results:\n";
    auto parallel = do_work(std::execution::par, v);

    std::cout << "conclusion: ";
    if ((sequential.time.mean() + 3 * sequential.time.stddev()) < parallel.time.mean())
        cout << "compute the results sequentially\n";

    else
        cout << "compute the results parallel\n";

    return 0;
}