#include "include/UTL/json.hpp"

#include <cassert>

int main() {
    using namespace utl;
    using namespace json::literals;

    // Create JSON from literal
    auto json = R"(
        {
            "string": "lorem_ipsum",
            "array": [ 1, 2, 3 ],
            "object": {
                "key_1": 3.14,
                "key_2": 6.28
            }
        }
    )"_utl_json;

    // Check that node exists
    assert( json.contains("string") );

    // Check the type of a JSON node
    assert( json["string"].is_string() );

    // Get typed value from a JSON node
    const auto str = json.at("string").get_string(); // '.at(key)' and '[key]' are both valid

    // Iterate over a JSON object node
    for (const auto &[key, value] : json.at("object").get_object())
        assert( key.front() == 'k' && value.get_number() > 0 );

    // Iterate over a JSON array node
    for (const auto &element : json.at("array").get_array())
        assert( element.get_number() > 0 );
}