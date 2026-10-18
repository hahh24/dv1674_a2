// Parallel overview: Independent pairs use the optimized sequential V6 kernel.

#ifndef ANALYSIS_PAR_HPP
#define ANALYSIS_PAR_HPP

#include "vector.hpp"
#include <vector>

namespace Analysis {

// We require a positive runtime thread count and borrow unchanged datasets
// until return. We precompute means, assign disjoint result intervals, and
// join all workers before returning the N*(N-1)/2 results in sequential order.
// One thread runs directly; zero threads or a failed start report an error.
std::vector<double> correlation_coefficients_parallel(
    const std::vector<Vector>& datasets,
    unsigned num_threads);

}

#endif