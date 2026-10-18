// Parallel overview: The runtime thread count controls independent pair workers
// over the optimized sequential V6 Pearson calculation.

#include "analysis_par.hpp"
#include "dataset.hpp"

#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

// We accept a positive decimal count that fits unsigned; invalid input
// throws before any worker starts. No thread count is fixed at build time.
unsigned parse_threads(const char* text)
{
    const std::string value{text};

    if (value.empty() ||
        value.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("Expected a positive thread count");
    }

    const unsigned long number = std::stoul(value);

    if (number == 0 || number > std::numeric_limits<unsigned>::max()) {
        throw std::invalid_argument("Thread count is out of range");
    }

    return static_cast<unsigned>(number);
}

int main(int argc, char const* argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " infile outfile num_threads\n";
        return 1;
    }

    try {
        // Arguments: argv[1] is the input path, argv[2] the output path,
        // and argv[3] sets the runtime thread count. The API joins workers
        // before returning, so datasets remain alive throughout their reads.
        const unsigned num_threads = parse_threads(argv[3]);

        auto datasets = Dataset::read(argv[1]);
        auto correlations = Analysis::correlation_coefficients_parallel(
            datasets, num_threads);

        Dataset::write(correlations, argv[2]);
    }
    // Parsing, input, or worker-start errors become a nonzero exit status.
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}