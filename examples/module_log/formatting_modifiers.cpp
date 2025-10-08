#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    log::info("Colored:        ", "text" | log::color::red                );
    log::info("Left-aligned:   ", "text" | log::align_left(10)            );
    log::info("Center-aligned: ", "text" | log::align_center(10)          );
    log::info("Right-aligned:  ", "text" | log::align_right(10)           );
    log::info("Fixed:          ", 2.3578 | log::fixed(2)                  );
    log::info("Scientific:     ", 2.3578 | log::scientific(2)             );
    log::info("Hex:            ", 2.3578 | log::hex(2)                    );
    log::info("Base-2:         ", 1024   | log::base(2)                   );
    log::info("Multiple:       ", 1024   | log::base(2) | log::color::blue);
}