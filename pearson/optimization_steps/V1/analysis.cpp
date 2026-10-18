/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 avoids input copies with const references.

#include "analysis.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>

namespace Analysis {

// V1: Previously this collection was copied. We now borrow it read-only,
// avoiding those deep copies; the caller must keep it alive during this call.
std::vector<double> correlation_coefficients(const std::vector<Vector>& datasets)
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
double pearson(const Vector& vec1,const Vector& vec2)
{
    auto x_mean { vec1.mean() };
    auto y_mean { vec2.mean() };

    // We center each input by subtracting its mean into a new Vector.
    auto x_mm { vec1 - x_mean };
    auto y_mm { vec2 - y_mean };

    // Magnitude is sqrt(sum of squared centered values), in index order.
    auto x_mag { x_mm.magnitude() };
    auto y_mag { y_mm.magnitude() };

    // We normalize each centered Vector by its own magnitude, then dot
    // their normalized values. Division creates two more temporary Vectors.
    auto x_mm_over_x_mag { x_mm / x_mag };
    auto y_mm_over_y_mag { y_mm / y_mag };

    auto r { x_mm_over_x_mag.dot(y_mm_over_y_mag) };

    return std::max(std::min(r, 1.0), -1.0);
}
};
