// Parallel overview: We keep sequential V2 weights, direct channel access,
// row-first traversal and unsigned char intermediate storage. We distribute
// output rows per pass and join all workers between the two passes.

#include "filters_par.hpp"
#include "filters.hpp"
#include "ppm.hpp"

#include <pthread.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace {


// Each task writes its own row range; inputs and weights are read-only.
// Parallel: A task borrows three input and three output channel arrays.
// Inputs and weights are shared for reading; [first_row, last_row) is owned
// exclusively for output. Overlapping input reads are safe, including vertical
// neighbors outside a worker's output range; disjoint writes need no mutex.
// Strides are physical row widths, not the number of active image columns.
struct Task {
    const unsigned char* input[3];
    unsigned char* output[3];
    const double* weights;
    int width;
    int height;
    int input_stride;
    int output_stride;
    int radius;
    int first_row;
    int last_row;
    bool horizontal;
};

void* process_rows(void* argument)
{
    const Task& task = *static_cast<Task*>(argument);

    // Parallel: Previously one sequential loop visited all rows. Each worker
    // now visits only its output rows, retaining the V2 inner x loop and
    // channel/index reuse. Empty row intervals naturally perform no work.
    for (int y = task.first_row; y < task.last_row; ++y) {
        for (int x = 0; x < task.width; ++x) {
            const int index = y * task.input_stride + x;

            double r = task.weights[0] * task.input[0][index];
            double g = task.weights[0] * task.input[1][index];
            double b = task.weights[0] * task.input[2][index];
            double n = task.weights[0];

            for (int wi = 1; wi <= task.radius; ++wi) {
                const double wc = task.weights[wi];
                // horizontal selects x neighbors for pass one and y neighbors
                // for pass two. Input stride always belongs to the input array.
                const int coordinate = task.horizontal ? x : y;
                const int limit = task.horizontal
                    ? task.width : task.height;

                // Preserve the original order: negative neighbor first.
                const int negative = coordinate - wi;
                if (negative >= 0) {
                    const int neighbor = task.horizontal
                        ? y * task.input_stride + negative
                        : negative * task.input_stride + x;

                    r += wc * task.input[0][neighbor];
                    g += wc * task.input[1][neighbor];
                    b += wc * task.input[2][neighbor];
                    n += wc;
                }

                const int positive = coordinate + wi;
                if (positive < limit) {
                    const int neighbor = task.horizontal
                        ? y * task.input_stride + positive
                        : positive * task.input_stride + x;

                    r += wc * task.input[0][neighbor];
                    g += wc * task.input[1][neighbor];
                    b += wc * task.input[2][neighbor];
                    n += wc;
                }
            }

            // Preserve V2 division and conversion to unsigned char in each
            // pass, including scratch; floating-point scratch would change results.
            const int output = y * task.output_stride + x;
            task.output[0][output] = r / n;
            task.output[1][output] = g / n;
            task.output[2][output] = b / n;
        }
    }

    return nullptr;
}

void run_pass(const Task& base, unsigned num_threads)
{
   
    // One-thread execution avoids worker creation.
    if (num_threads == 1) {
        Task task = base;
        task.first_row = 0;
        task.last_row = base.height;
        process_rows(&task);
        return;
    }

    // Parallel: Allocate stable Task storage before starting workers. Tasks,
    // matrices and weights must remain alive until every worker is joined.
    std::vector<Task> tasks(num_threads, base);
    std::vector<pthread_t> threads(num_threads);

    unsigned created = 0;
    int create_error = 0;

    for (unsigned i = 0; i < num_threads; ++i) {
        // Contiguous row ranges cover the image without overlapping writes.
        // Boundaries floor(height*i/threads) partition contiguous rows.
        // Ranges cover the image exactly once and differ by at most one row.
        // We use a wide product before division; excess workers get empty ranges.
        tasks[i].first_row = static_cast<int>(
            static_cast<unsigned long long>(base.height) * i
            / num_threads);
        tasks[i].last_row = static_cast<int>(
            static_cast<unsigned long long>(base.height) * (i + 1)
            / num_threads);

        // pthread_create starts process_rows concurrently using this Task.
        create_error = pthread_create(
            &threads[i], nullptr, process_rows, &tasks[i]);

        // On failure stop creating workers, then join those already running
        // before throwing; the caller must not use a partially completed pass.
        if (create_error != 0) {
            break;
        }

        ++created;
    }

    
    // pthread_join waits for each worker and makes its writes visible.
    // run_pass returns only after all started workers have completed; join
    // failure terminates rather than releasing storage a worker may still use.
    for (unsigned i = 0; i < created; ++i) {
        const int error = pthread_join(threads[i], nullptr);
        if (error != 0) {
            // Continuing would risk destroying data still used by a worker.
            std::terminate();
        }
    }

    if (create_error != 0) {
        throw std::runtime_error(
            "pthread_create failed: " + std::to_string(create_error));
    }
}

}

namespace Filter {

Matrix blur_parallel(Matrix m, int radius, unsigned num_threads)
{
    if (num_threads == 0) {
        throw std::invalid_argument("Thread count must be positive");
    }

    if (radius < 1 || radius > static_cast<int>(Gauss::max_radius)) {
        throw std::invalid_argument("Radius must be between 1 and 1000");
    }

    // V2 storage: Scratch is still the 3000-wide intermediate image.
    // The active image must fit this allocation, just as in sequential blur.
    Matrix scratch{PPM::max_dimension};
    auto dst{m};

    const int width = static_cast<int>(dst.get_x_size());
    const int height = static_cast<int>(dst.get_y_size());
    const int scratch_stride =
        static_cast<int>(scratch.get_x_size());

    // V1 reuse: We compute radius-dependent weights once for both passes.
    // V2 buffer correctness: Inclusive indices 0..radius need radius + 1
    // entries; this buffer change was not a separately measured speed gain.
    std::vector<double> weights(radius + 1);
    Gauss::get_weights(radius, weights.data());

    // Parallel pass one reads dst R/G/B and writes scratch R/G/B.
    // Its input stride is image width; its output stride is scratch width.
    Task horizontal{};
    horizontal.input[0] = dst.get_R();
    horizontal.input[1] = dst.get_G();
    horizontal.input[2] = dst.get_B();
    horizontal.output[0] = scratch.get_R();
    horizontal.output[1] = scratch.get_G();
    horizontal.output[2] = scratch.get_B();
    horizontal.weights = weights.data();
    horizontal.width = width;
    horizontal.height = height;
    horizontal.input_stride = width;
    horizontal.output_stride = scratch_stride;
    horizontal.radius = radius;
    horizontal.horizontal = true;

    run_pass(horizontal, num_threads);

    // All horizontal workers have joined: scratch is complete for vertical reads.
    // Parallel: This join boundary replaces the sequential pass boundary.
    // Vertical workers read neighboring scratch rows, which other workers
    // may have produced. All horizontal writes must finish before those reads.
    // Synchronization uses pthread_join, not pthread_barrier. Pass two reads
    // scratch with stride 3000 and writes dst with the actual image width.
    Task vertical = horizontal;
    vertical.input[0] = scratch.get_R();
    vertical.input[1] = scratch.get_G();
    vertical.input[2] = scratch.get_B();
    vertical.output[0] = dst.get_R();
    vertical.output[1] = dst.get_G();
    vertical.output[2] = dst.get_B();
    vertical.input_stride = scratch_stride;
    vertical.output_stride = width;
    vertical.horizontal = false;

    // A second run_pass creates new workers (unless num_threads is one).
    // It joins them before returning dst and destroying weights or scratch.
    run_pass(vertical, num_threads);

    return dst;
}

}