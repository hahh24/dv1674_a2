// Parallel overview: Runtime arguments select radius, file paths and workers
// for the two-pass blur based on optimized sequential V2.

#include "filters_par.hpp"
#include "ppm.hpp"

#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

// We validate decimal positive integers and unsigned range before starting
// work. The caller additionally checks the supported maximum blur radius.
unsigned parse_number(const char* text)
{
    const std::string value{text};

    if (value.empty() ||
        value.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("Expected a positive integer");
    }

    const unsigned long number = std::stoul(value);

    if (number == 0 || number > std::numeric_limits<unsigned>::max()) {
        throw std::invalid_argument("Integer is out of range");
    }

    return static_cast<unsigned>(number);
}

int main(int argc, char const* argv[])
{
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " radius infile outfile num_threads\n";
        return 1;
    }

    try {
        // argv[1] supplies radius; argv[2]/argv[3] are input/output paths.
        // argv[4] supplies the runtime thread count, independent of build flags.
        const unsigned radius = parse_number(argv[1]);
        const unsigned num_threads = parse_number(argv[4]);

        if (radius > 1000) {
            throw std::invalid_argument("Maximum radius is 1000");
        }

        PPM::Reader reader{};
        PPM::Writer writer{};

        auto m = reader(argv[2]);
        // blur_parallel joins both passes before returning. Input m stays
        // alive throughout; output is written only after computation finishes.
        auto blurred = Filter::blur_parallel(
            m, static_cast<int>(radius), num_threads);

        writer(blurred, argv[3]);
    }
    // Invalid arguments, file errors or failed thread starts report failure.
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}