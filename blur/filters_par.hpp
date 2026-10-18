// Parallel overview: We distribute output rows of optimized sequential V2.

#ifndef FILTERS_PAR_HPP
#define FILTERS_PAR_HPP

#include "matrix.hpp"

namespace Filter {
// We require a radius in 1..1000, a positive runtime thread count and an
// image fitting the 3000-wide scratch matrix. Two passes share read-only
// weights, write disjoint rows, and synchronize with join between passes.
// One thread runs directly; failed worker creation joins started workers
// before reporting an error. All borrowed task data stays alive until join.
Matrix blur_parallel(Matrix m, int radius, unsigned num_threads);
}

#endif