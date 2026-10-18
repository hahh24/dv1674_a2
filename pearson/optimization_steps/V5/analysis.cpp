/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 avoids input copies with const references.
// V2 computes centered squares and normalized products without temporary vectors.
// V3 caches length in the first loop; V4 also uses it in the second loop.
// V5 reads existing storage through const pointers instead of operator[].

#include "analysis.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

namespace Analysis {

// V1: Previously this collection was copied. We now borrow it read-only,
// avoiding those deep copies; the caller must keep it alive during this call.
std::vector<double> correlation_coefficients(
    const std::vector<Vector>& datasets)
{
    std::vector<double> result {};

    for (auto sample1 { 0 }; sample1 < datasets.size() - 1; sample1++) {
        for (auto sample2 { sample1 + 1 }; sample2 < datasets.size(); sample2++) {
            auto corr { pearson(datasets[sample1], datasets[sample2]) };
            result.push_back(corr);
        }
    }

    return result;
}

// V1: We borrow both input vectors instead of copying them per pair.
// Neither input may change during the calculation.
// We require equal, nonempty lengths and nonzero centered magnitudes for
// a defined Pearson coefficient; these conditions are not checked here.
double pearson(const Vector& vec1, const Vector& vec2)
{
    auto x_mean { vec1.mean() };
    auto y_mean { vec2.mean() };

    // V2: Previously we built two centered and two normalized Vectors.
    // We now compute values directly, avoiding their allocation and copying.
    // Subtracting the mean centers each value; summing its square gives the
    // squared magnitude. Each individual sum still visits indices in order.
    double x_squared_sum { 0.0 };
    double y_squared_sum { 0.0 };

    // V3: Previously the first loop queried get_size() on every test. We save
    // the unchanged length once to reduce repeated calls; both inputs must
    // have this length and remain unchanged for the whole calculation.
    const unsigned size { vec1.get_size() };
    // V5: Previously each read used Vector::operator[]. These const pointers
    // refer to existing storage, with no data copies, and can reduce accessor
    // call overhead. Keep both owners alive and every index below their size.
    const double* x_data { vec1.get_data() };
    const double* y_data { vec2.get_data() };

    for (unsigned i { 0 }; i < size; i++) {
        double x { x_data[i] - x_mean };
        double y { y_data[i] - y_mean };

        x_squared_sum += x * x;
        y_squared_sum += y * y;
    }

    // V2: Taking the square root gives the same centered vector magnitude.
    auto x_mag { std::sqrt(x_squared_sum) };
    auto y_mag { std::sqrt(y_squared_sum) };

    // V2: We center again and divide each value by its own magnitude, then
    // accumulate normalized products in index order, as the old dot() did.
    // We keep the divisions before multiplication and the final clamp.
    // V4: The second loop now also uses the saved length, unlike V3.
    // This can remove its repeated size queries without changing index order.
    double r { 0.0 };

    for (unsigned i { 0 }; i < size; i++) {
        double x_normalized { (x_data[i] - x_mean) / x_mag };
        double y_normalized { (y_data[i] - y_mean) / y_mag };

        r += x_normalized * y_normalized;
    }

    return std::max(std::min(r, 1.0), -1.0);
}

};
