/*
Author: David Holmqvist <daae19@student.bth.se>
*/

// Optimization overview: V1 reuses weights and saves image dimensions.
// V2 introduces direct channels and row-first traversal together, plus a
// radius + 1 weight buffer as a correctness change.

#include "filters.hpp"
#include "matrix.hpp"
#include "ppm.hpp"
#include <cmath>
#include <vector>

namespace Filter
{

namespace Gauss
{
    void get_weights(int n, double* weights_out)
    {
        for (int i{0}; i <= n; i++)
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
    // V2: Scratch is the intermediate image with row width 3000
    // (PPM::max_dimension); dst has the actual image width. Each array must
    // use its own stride so a narrow image does not mix scratch rows.
    const int scratch_width{
        static_cast<int>(scratch.get_x_size())
    };

    // get_weights writes radius + 1 entries.
    // V1: Previously every pixel recomputed exp() weights in both passes.
    // We compute them once: they depend on radius and distance, not position.
    // Reuse keeps the same weights while avoiding repeated Gaussian evaluation.
    // V2: The old fixed buffer could overflow at maximum radius. We now have
    // radius + 1 entries for inclusive indices 0..radius. This is a buffer
    // correctness change, not a separately measured speed optimization.
    std::vector<double> weights(radius + 1);
    Gauss::get_weights(radius, weights.data());
    const double* w{weights.data()};

    // V2: Previously every channel used r/g/b accessors. We now borrow the
    // existing channel arrays; no pixel copies are made by these getters.
    // Keep the matrices alive and preserve the image and neighbor bounds.
    unsigned char* dst_r{dst.get_R()};
    unsigned char* dst_g{dst.get_G()};
    unsigned char* dst_b{dst.get_B()};

    unsigned char* scratch_r{scratch.get_R()};
    unsigned char* scratch_g{scratch.get_G()};
    unsigned char* scratch_b{scratch.get_B()};

    // V2: Previously x was outermost. We now visit y then contiguous x,
    // aiming for better locality, and reuse row/neighbor indices for R, G, B.
    // Direct access and loop order were introduced together, not measured
    // separately; neither change guarantees fewer cache misses or speedup.
    // Horizontal pass: read dst and write scratch.
    for (int y{0}; y < height; y++)
    {
        const int dst_row{y * width};
        const int scratch_row{y * scratch_width};

        for (int x{0}; x < width; x++)
        {
            const int index{dst_row + x};

            auto r{w[0] * dst_r[index]};
            auto g{w[0] * dst_g[index]};
            auto b{w[0] * dst_b[index]};
            auto n{w[0]};

            for (int wi{1}; wi <= radius; wi++)
            {
                const double wc{w[wi]};
                int x2{x - wi};

                if (x2 >= 0)
                {
                    const int neighbor{dst_row + x2};
                    r += wc * dst_r[neighbor];
                    g += wc * dst_g[neighbor];
                    b += wc * dst_b[neighbor];
                    n += wc;
                }

                x2 = x + wi;

                if (x2 < width)
                {
                    const int neighbor{dst_row + x2};
                    r += wc * dst_r[neighbor];
                    g += wc * dst_g[neighbor];
                    b += wc * dst_b[neighbor];
                    n += wc;
                }
            }

            // V2 preserves conversion to unsigned char between passes.
            // Keeping scratch as floating point would change later results.
            const int output{scratch_row + x};
            scratch_r[output] = r / n;
            scratch_g[output] = g / n;
            scratch_b[output] = b / n;
        }
    }

    // The full first pass must finish: vertical neighbors can read other
    // scratch rows. Use scratch_width for reads and width for dst writes.
    // Per-pixel neighbor order and edge normalization remain unchanged.
    // Vertical pass: read scratch and write dst.
    for (int y{0}; y < height; y++)
    {
        const int scratch_row{y * scratch_width};
        const int dst_row{y * width};

        for (int x{0}; x < width; x++)
        {
            const int index{scratch_row + x};

            auto r{w[0] * scratch_r[index]};
            auto g{w[0] * scratch_g[index]};
            auto b{w[0] * scratch_b[index]};
            auto n{w[0]};

            for (int wi{1}; wi <= radius; wi++)
            {
                const double wc{w[wi]};
                int y2{y - wi};

                if (y2 >= 0)
                {
                    const int neighbor{y2 * scratch_width + x};
                    r += wc * scratch_r[neighbor];
                    g += wc * scratch_g[neighbor];
                    b += wc * scratch_b[neighbor];
                    n += wc;
                }

                y2 = y + wi;

                if (y2 < height)
                {
                    const int neighbor{y2 * scratch_width + x};
                    r += wc * scratch_r[neighbor];
                    g += wc * scratch_g[neighbor];
                    b += wc * scratch_b[neighbor];
                    n += wc;
                }
            }

            const int output{dst_row + x};
            dst_r[output] = r / n;
            dst_g[output] = g / n;
            dst_b[output] = b / n;
        }
    }

    return dst;
}

}