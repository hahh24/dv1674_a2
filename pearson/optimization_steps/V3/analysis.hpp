/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 avoids input copies with const references.
// V2 computes centered squares and normalized products without temporary vectors.
// V3 caches length only in the first loop: this is a partial intermediate step.
// This snapshot was reconstructed from V4; see PROVENANCE.txt for its origin.

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
