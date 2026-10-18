/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 reuses weights and saves image dimensions.

#include "matrix.hpp"

#if !defined(FILTERS_HPP)
#define FILTERS_HPP

namespace Filter
{

    namespace Gauss
    {
        constexpr unsigned max_radius{1000};
        constexpr float max_x{1.33};
        constexpr float pi{3.14159};

        // We write n + 1 weights (indices 0..n), so the caller must provide
        // that many entries and a positive n. Values depend on n and distance.
        void get_weights(int n, double *weights_out);
    }

    Matrix blur(Matrix m, const int radius);

}

#endif