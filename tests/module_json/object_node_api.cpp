#include "tests/common.hpp"

#include "include/UTL/json.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("Object node API / Basics") {
    const auto json = json::from_string(R"(
        {
            "string": "lorem ipsum",
            "number": 17,
            "null": null
        }
    )");

    CHECK_THROWS(json.at("non_existent_key"));
    
    CHECK(json.contains("string"));
    CHECK(json.at("string").get_string() == "lorem ipsum");
    CHECK(json.value_or("number", -5.) == 17.);
    CHECK(json.value_or("non_existent_key", -5.) == -5.);
}

TEST_CASE_TEMPLATE("Object node API / Only 'Null' converts automatically converts to 'Object'", T, //
                   json::Object,                                                                   //
                   json::Array,                                                                    //
                   json::String,                                                                   //
                   json::Number,                                                                   //
                   json::Bool,                                                                     //
                   json::Null                                                                      //
) {
    json::Node json = T();
    
    const auto insert_value_at_key = [&]() { json["key"] = "value"; };

    // Only 'Null' converts to 'Object' implicitly after 'operator[]'
    if (json.is_null() || json.is_object()) {
        CHECK_NOTHROW(insert_value_at_key());
        CHECK(json.is_object());
    }
    // other types should throw on conversion
    else {
        CHECK_THROWS(insert_value_at_key());
    }
}
