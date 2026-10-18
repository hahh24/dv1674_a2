/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 avoids input copies with const references.
// V2 computes centered squares and normalized products without temporary vectors.
// V3 caches length in the first loop; V4 also uses it in the second loop.

#include "vector.hpp"
#include <vector>

#if !defined(ANALYSIS_HPP)
#define ANALYSIS_HPP

namespace Analysis {
// V1: Const references replace value parameters to avoid deep input copies.
// We only read these inputs, and their owners must outlive the call.
std::vector<double> correlation_coefficients(const std::vector<Vector>& datasets);
double pearson(const Vector& vec1, const Vector& vec2);
};

#endif
