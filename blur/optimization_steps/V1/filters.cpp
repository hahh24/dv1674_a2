/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 reuses weights and saves image dimensions.

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>

namespace Filter
{

    namespace Gauss
    {
        void get_weights(int n, double *weights_out)
        {
            for (auto i{0}; i <= n; i++)
            {
                double x{static_cast<double>(i) * max_x / n};
                weights_out[i] = exp(-x * x * pi);
            }
        }
    }

    // We keep two separable passes: horizontal into scratch, then vertical into
    // dst. Only in-bounds neighbors contribute, including to the weight sum n.
    // The caller must supply a positive radius and an image fitting scratch.
    Matrix blur(Matrix m, const int radius)
    {
        Matrix scratch{PPM::max_dimension};
        auto dst{m};

        // V1: Previously loop tests and edge checks repeatedly queried dimensions.
        // We save the unchanged width and height to reduce those accessor calls.
        const int width{static_cast<int>(dst.get_x_size())};
        const int height{static_cast<int>(dst.get_y_size())};

        // V1: Previously every pixel recomputed exp() weights in both passes.
        // We compute them once: they depend on radius and distance, not position.
        // Reuse keeps the same weights while avoiding repeated Gaussian evaluation.
        // This fixed buffer still requires radius < max_radius (inclusive writes).
        double w[Gauss::max_radius]{};
        Gauss::get_weights(radius, w);

        // Horizontal pass
        for (int x{0}; x < width; x++)
        {
            for (int y{0}; y < height; y++)
            {
                auto r{w[0] * dst.r(x, y)};
                auto g{w[0] * dst.g(x, y)};
                auto b{w[0] * dst.b(x, y)};
                auto n{w[0]};

                for (int wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto x2{x - wi};

                    if (x2 >= 0)
                    {
                        r += wc * dst.r(x2, y);
                        g += wc * dst.g(x2, y);
                        b += wc * dst.b(x2, y);
                        n += wc;
                    }

                    x2 = x + wi;

                    if (x2 < width)
                    {
                        r += wc * dst.r(x2, y);
                        g += wc * dst.g(x2, y);
                        b += wc * dst.b(x2, y);
                        n += wc;
                    }
                }

                scratch.r(x, y) = r / n;
                scratch.g(x, y) = g / n;
                scratch.b(x, y) = b / n;
            }
        }

        // First finish all scratch rows: the vertical pass reads neighboring rows.
        // Assignments above convert to unsigned char, as in the baseline.
        // Vertical pass
        for (int x{0}; x < width; x++)
        {
            for (int y{0}; y < height; y++)
            {
                auto r{w[0] * scratch.r(x, y)};
                auto g{w[0] * scratch.g(x, y)};
                auto b{w[0] * scratch.b(x, y)};
                auto n{w[0]};

                for (int wi{1}; wi <= radius; wi++)
                {
                    auto wc{w[wi]};
                    auto y2{y - wi};

                    if (y2 >= 0)
                    {
                        r += wc * scratch.r(x, y2);
                        g += wc * scratch.g(x, y2);
                        b += wc * scratch.b(x, y2);
                        n += wc;
                    }

                    y2 = y + wi;

                    if (y2 < height)
                    {
                        r += wc * scratch.r(x, y2);
                        g += wc * scratch.g(x, y2);
                        b += wc * scratch.b(x, y2);
                        n += wc;
                    }
                }

                dst.r(x, y) = r / n;
                dst.g(x, y) = g / n;
                dst.b(x, y) = b / n;
            }
        }

        return dst;
    }

}