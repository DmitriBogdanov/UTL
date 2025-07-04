#include "include/UTL/log.hpp"

#include <cassert>

// A custom printable type
struct SomeCustomType {};

std::ostream& operator<<(std::ostream& os, SomeCustomType) {
    return os << "<custom type string>";
}

int main() {

    using namespace utl;
    
    // Printing
    log::println("Print any objects you want, for example: ", std::tuple{ "lorem", 0.25, "ipsum" });
    log::println("This is almost like Python!");
    log::println("Except compiled...");
    
    // Stringification
    assert( log::stringify("int is ", 5)          == "int is 5"             );
    assert( log::stringify(std::array{ 4, 5, 6 }) == "{ 4, 5, 6 }"          );
    assert( log::stringify(std::pair{ -1, 1 })    == "< -1, 1 >"            );
    assert( log::stringify(SomeCustomType{})      == "<custom type string>" );
    // ...and so on for any reasonable type including nested containers,
    // if you append values to an existing string 'log::append_stringified(str, ...)'
    // can be used instead of ' += log::stringify(...)' for even better performance
}