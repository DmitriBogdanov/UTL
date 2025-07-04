#include "include/UTL/json.hpp"

#include <iostream>

int main() {
    using namespace utl;
    
    json::Node json;
    
    json["string"]           = "lorem ipsum";
    json["array"]            = { 1, 2, 3 }; 
    json["object"]["key_1"]  = 3.14; 
    json["object"]["key_2"]  = 6.28;
    
    // Prettified/Minimized JSON
    std::cout
        << "--- Prettified JSON ---"
        << "\n\n"
        << json.to_string()
        << "\n\n"
        << "--- Minimized JSON ---"
        << "\n\n"
        << json.to_string(json::Format::MINIMIZED);
}