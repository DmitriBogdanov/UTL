#include "include/UTL/integral.hpp"

#include <iostream>

int main() {
    try {
        constexpr int N = -14;

        // for (std::size_t i = 0; i < N; ++i) std::cout << i;
        // compiler warns about signed/unsigned comparison, doesn't compile with -Werror

        // for (std::size_t i = 0; i < static_cast<std::size_t>(N); ++i) std::cout << i;
        // casts 'N' to '18446744073709551602' since we forgot to check for negative 'N'

        for (std::size_t i = 0; i < utl::integral::to_unsigned(N); ++i) std::cout << i;
        // this is good, comparison is unsigned/unsigned and incorrect 'N' will cause an exception

    } catch (std::domain_error &e) {
        std::cerr << "ERROR: Caught exception:\n\n" << e.what();
    }
}