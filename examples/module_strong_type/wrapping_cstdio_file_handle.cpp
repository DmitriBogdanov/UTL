#include "include/UTL/strong_type.hpp"

#include <cstdio>

int main() {
    using namespace utl;
    
    // Create strongly typed wrapper around <cstdio> file handle
    // (FILE*) with move-only semantics and RAII cleanup
    using FileHandle = strong_type::Unique<FILE*, class FileTag, strong_type::Bind<&std::fclose>>;
    
    FileHandle file = std::fopen("temp.txt", "w");
    
    // calls 'fclose()' on internal pointer upon destruction
}
