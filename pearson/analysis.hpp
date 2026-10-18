/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 avoids input copies with const references.
// V2 computes centered squares and normalized products without temporary vectors.
// V3 caches length in the first loop; V4 also uses it in the second loop.
// V5 reads existing storage through const pointers instead of operator[].
// V6 reuses one mean per dataset; magnitudes are still computed per pair.

#ifndef ANALYSIS_HPP
#define ANALYSIS_HPP

#include "vector.hpp"
#include <vector>

namespace Analysis {

// V1: Const references replace value parameters to avoid deep input copies.
// We only read these inputs, and their owners must outlive the call.
std::vector<double> correlation_coefficients(
    const std::vector<Vector>& datasets);

// V6: Previously pearson computed both means per call. The caller now
// supplies the matching precomputed means to reuse them across pairs.
// This function still calculates both magnitudes for every pair.
double pearson(
    const Vector& vec1,
    const Vector& vec2,
    double x_mean,
    double y_mean);

}

#endif
