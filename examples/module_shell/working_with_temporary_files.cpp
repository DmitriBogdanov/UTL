#include "include/UTL/shell.hpp"

#include <cassert>

int main() {
    const auto handle = utl::shell::TemporaryHandle::overwrite("temporary.txt");
    
    // Write to temporary file
    handle.ofstream() << "TEXT";
    
    // Read from temporary file
    std::string          text;
    handle.ifstream() >> text;
    
    assert(text == "TEXT");
    
    // Append some more text
    handle.ofstream(std::ios_base::app) << "MORE TEXT";
    
    // Temp. file is deleted once handle is destroyed
}