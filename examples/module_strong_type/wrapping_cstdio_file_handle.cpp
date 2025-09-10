#include "include/UTL/strong_type.hpp"

// Disable MSVC warning about using 'fopen' instead of a windows-specific 'fopen_s()',
// this warning is both dubious and not particularly relevant for this example
#ifdef  _MSC_VER
#pragma warning (disable: 4996, justification: "Non-conformant behavior of MSVC.")
#endif

#include <cstdio>

int main() {
    using namespace utl;
    
    // Create strongly typed wrapper around <cstdio> file handle
    // (FILE*) with move-only semantics and RAII cleanup
    using FileHandle = strong_type::Unique<FILE*, class FileTag, strong_type::Bind<&std::fclose>>;
    
    FileHandle file = std::fopen("temp.txt", "w");
    
    // calls 'fclose()' on internal pointer upon destruction
}
