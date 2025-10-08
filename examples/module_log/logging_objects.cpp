#include "include/UTL/log.hpp"

#include <complex>

int main() {
    using namespace utl;
    
    const auto start = std::chrono::steady_clock::now();
    
    log::info("Starting the solver, x0 = ", std::vector{2e-3, 3e-3, 4e-3}           );
    log::warn("Solution diverges, err = " , std::complex<double>{2e14, 3e28}        );
    log::err ("Terminated after "         , std::chrono::steady_clock::now() - start);
}