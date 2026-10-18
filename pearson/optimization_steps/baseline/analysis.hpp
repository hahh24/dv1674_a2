/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Baseline: We pass vectors by value and use temporary vectors for Pearson.

#include "vector.hpp"
#include <vector>

#if !defined(ANALYSIS_HPP)
#define ANALYSIS_HPP

namespace Analysis {
// Baseline: The collection and pair arguments are passed by value.
std::vector<double> correlation_coefficients(std::vector<Vector> datasets);
double pearson(Vector vec1, Vector vec2);
};

#endif
