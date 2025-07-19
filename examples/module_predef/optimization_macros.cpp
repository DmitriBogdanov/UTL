#include "include/UTL/predef.hpp"

#include <iostream>

enum class State { YES, NO };

UTL_PREDEF_FORCE_INLINE std::string to_string(State value) {
    switch (value) {
        case State::YES: return "YES";
        case State::NO : return "NO" ;
        default:         utl::predef::unreachable();
    }
}

int main() {
    assert( to_string(State::YES) == "YES" );
}