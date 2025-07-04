#include "include/UTL/json.hpp"

#include <unordered_map>
#include <list>
#include <set>

int main() {
    using namespace utl;
    
    json::Node json;
    
    // Ways to assign a JSON object
    json["object"]["key_1"] = 1;
    json["object"]["key_2"] = 2;
    json["object"]          =                                     { { "key_1", 1 }, { "key_2", 2 } };
    json["object"]          =                         json::Object{ { "key_1", 1 }, { "key_2", 2 } };
    json["object"]          =           std::map<std::string, int>{ { "key_1", 1 }, { "key_2", 2 } };
    json["object"]          = std::unordered_map<std::string, int>{ { "key_1", 1 }, { "key_2", 2 } };
    
    // Ways to assign a JSON array
    json["array"] =            { 1, 2, 3 };
    json["array"] = json::Array{ 1, 2, 3 };
    json["array"] = std::vector{ 1, 2, 3 };
    json["array"] =   std::list{ 1, 2, 3 };
    json["array"] =    std::set{ 1, 2, 3 };
    
    json["matrix"] = { { 1, 2 }, { 3, 4 } }; // matrices & tensors are fine too
    json["tensor"] = { { { 1, 2 }, { 3, 4 } }, { { 4, 5 }, { 6, 7 } } };
    
    // Ways to assign a JSON string
    json["string"] =                  "lorem ipsum" ;
    json["string"] =     json::String("lorem ipsum");
    json["string"] =      std::string("lorem ipsum");
    json["string"] = std::string_view("lorem ipsum");
    
    // ...and so on and so forth with other types, same thing with custom containers.
    // All classes can convert as long as they provide std-like API.
}