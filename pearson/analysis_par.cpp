// Parallel overview: We build on sequential V6 (shared, precomputed means).
// We distribute independent pairs; each coefficient uses sequential pearson.

#include "analysis_par.hpp"
#include "analysis.hpp"

#include <pthread.h>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Shared inputs are read-only; each task owns a disjoint result interval.
// Parallel: Unlike sequential push_back, workers write fixed result slots.
// Datasets, means and offsets are shared for reading only. Each worker owns
// [begin, end), so distinct double elements can be written without a mutex.
// These pointers borrow storage that must outlive every worker.
struct Task {
    const std::vector<Vector>* datasets;
    const std::vector<double>* means;
    const std::vector<std::size_t>* offsets;
    std::vector<double>* result;
    std::size_t begin;
    std::size_t end;
};

void* process_pairs(void* argument)
{
    const Task& task = *static_cast<Task*>(argument);

    // Parallel: More workers than pairs can give empty intervals. Return
    // before mapping an index that does not identify any pair.
    if (task.begin == task.end) {
        return nullptr;
    }

    const auto& datasets = *task.datasets;
    const auto& means = *task.means;
    const auto& offsets = *task.offsets;
    auto& result = *task.result;

    
    // Map the first flat result index to the original dataset pair.
    // Parallel: offsets[i] is the flat index of the first pair (i, i+1).
    // upper_bound finds the first offset greater than begin; the previous
    // entry identifies sample1. The distance from that offset gives sample2.
    // We then advance in the same pair order as the sequential nested loops.
    const auto position = std::upper_bound(
        offsets.begin(), offsets.end(), task.begin);

    std::size_t sample1 =
        static_cast<std::size_t>(position - offsets.begin() - 1);
    std::size_t sample2 =
        sample1 + 1 + (task.begin - offsets[sample1]);

    for (std::size_t index = task.begin; index < task.end; ++index) {
        // Parallel: We call the sequential V6 function for this whole pair.
        // Its square sums and product sum stay on this worker, with their
        // original arithmetic order; no coefficient uses a parallel reduction.
        result[index] = Analysis::pearson(
            datasets[sample1],
            datasets[sample2],
            means[sample1],
            means[sample2]);

        ++sample2;

        if (sample2 == datasets.size()) {
            ++sample1;
            sample2 = sample1 + 1;
        }
    }

    return nullptr;
}

}

namespace Analysis {

std::vector<double> correlation_coefficients_parallel(
    const std::vector<Vector>& datasets,
    unsigned num_threads)
{
    if (num_threads == 0) {
        throw std::invalid_argument("Thread count must be positive");
    }

    if (datasets.size() < 2) {
        return {};
    }

    // V6 reuse: Compute each mean once before workers start. The original
    // datasets stay unchanged; magnitudes are still computed for every pair.
    std::vector<double> means;
    means.reserve(datasets.size());

    for (const auto& dataset : datasets) {
        means.push_back(dataset.mean());
    }

    // Parallel: Row i in the upper triangle has N-i-1 pairs. Prefix sums
    // flatten all N*(N-1)/2 unique pairs (i < j), with no self-pairs.
    std::vector<std::size_t> offsets(datasets.size(), 0);

    for (std::size_t i = 0; i + 1 < datasets.size(); ++i) {
        offsets[i + 1] =
            offsets[i] + (datasets.size() - i - 1);
    }

    const std::size_t pair_count = offsets.back();

    // Preallocation avoids concurrent vector resizing or push_back.
    // Parallel: Previously sequential code appended each result. We allocate
    // the full size before starting threads so workers never push_back or
    // resize shared storage, which would race and could invalidate references.
    std::vector<double> result(pair_count);

    Task base{
        &datasets, &means, &offsets, &result, 0, pair_count
    };

    // Parallel: One thread calls the same worker directly on the caller,
    // avoiding pthread creation and join overhead for this path.
    if (num_threads == 1) {
        process_pairs(&base);
        return result;
    }

    // Parallel: Allocate all Task objects before handing their addresses to
    // pthreads. Neither this vector nor any shared storage may move or be
    // destroyed until all workers have joined.
    std::vector<Task> tasks(num_threads, base);
    std::vector<pthread_t> threads(num_threads);

    // Parallel: The old nested loops ran all pairs on one thread. We give
    // each worker chunk pairs and the first remainder workers one extra.
    // Contiguous intervals cover each pair once, differing by at most one.
    // This balances pair counts; it does not guarantee equal execution times.
    const std::size_t chunk = pair_count / num_threads;
    const std::size_t remainder = pair_count % num_threads;

    unsigned created = 0;
    int create_error = 0;

    for (unsigned i = 0; i < num_threads; ++i) {

        // Equal pair counts, with the remainder shared among early workers.
        tasks[i].begin =
            chunk * i + std::min<std::size_t>(i, remainder);
        tasks[i].end =
            tasks[i].begin + chunk + (i < remainder ? 1 : 0);

        // pthread_create starts process_pairs concurrently with this Task.
        create_error = pthread_create(
            &threads[i], nullptr, process_pairs, &tasks[i]);

        // Stop starting threads on failure; join all successfully started
        // workers below before reporting failure, not a partial result.
        if (create_error != 0) {
            break;
        }

        ++created;
    }

    // Join before reading output or releasing shared task storage.
    // pthread_join waits for completion and makes result writes available
    // to the caller. If join fails, terminate: worker lifetime is uncertain
    // and releasing its borrowed data would be unsafe.
    for (unsigned i = 0; i < created; ++i) {
        if (pthread_join(threads[i], nullptr) != 0) {
            std::terminate();
        }
    }

    if (create_error != 0) {
        throw std::runtime_error(
            "pthread_create failed: " + std::to_string(create_error));
    }

    return result;
}

}