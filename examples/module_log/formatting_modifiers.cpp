#include "include/UTL/log.hpp"

int main() {
    using namespace utl;
    
    log::note("Colored:        ", "text" | log::color::red                );
    log::note("Left-aligned:   ", "text" | log::align_left(10)            );
    log::note("Center-aligned: ", "text" | log::align_center(10)          );
    log::note("Right-aligned:  ", "text" | log::align_right(10)           );
    log::note("Fixed:          ", 2.3578 | log::fixed(2)                  );
    log::note("Scientific:     ", 2.3578 | log::scientific(2)             );
    log::note("Hex:            ", 2.3578 | log::hex(2)                    );
    log::note("Base-2:         ", 1024   | log::base(2)                   );
    log::note("Multiple:       ", 1024   | log::base(2) | log::color::blue);
}