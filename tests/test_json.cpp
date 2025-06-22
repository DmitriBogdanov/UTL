// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/json.hpp"

// _______________________ INCLUDES _______________________

#include <array>            // testing JSON array conversion
#include <deque>            // testing JSON array conversion
#include <filesystem>       // iteration over the test suite files
#include <forward_list>     // testing JSON array conversion
#include <initializer_list> // testing JSON array conversion
#include <list>             // testing JSON array conversion
#include <set>              // testing JSON array conversion
#include <unordered_map>    // testing JSON array conversion
#include <vector>           // testing JSON array conversion

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ==================================
// --- RFC-8259 conformance tests ---
// ==================================

TEST_CASE("Parser passes JSON validation test suite (accept valid)") {
    const fs::path test_suite_path = "tests/data/json_test_suite/should_accept/";

    std::cout << "Running test case from path: " << fs::current_path() << '\n' << std::flush;

    for (const auto& test_suite_entry : fs::directory_iterator(test_suite_path)) {
        std::cout << "Parsing file named: " << test_suite_entry.path() << '\n' << std::flush;

        const bool throws = check_if_throws([&]() { return json::from_file(test_suite_entry.path()); });

        CHECK(!throws);
    }
}

TEST_CASE("Parser passes JSON validation test suite (reject invalid)") {
    const fs::path test_suite_path = "tests/data/json_test_suite/should_reject/";

    std::cout << "Running test case from path: " << fs::current_path() << '\n' << std::flush;

    // JSON specification is rather strict when it comes to numeric format, not allowing a lot of the valid IEEE 754
    // floats that work fine with C++ formatting facilities. Since there is no real benefit in prohibiting the parsing
    // of such values, we explicitly mark them as "acceptable to parse".
    const std::unordered_set<std::string> acceptable_cases = {"n_number_0.e1.json",
                                                              "n_number_2.e3.json",
                                                              "n_number_2.e+3.json",
                                                              "n_number_2.e-3.json",
                                                              "n_number_-01.json",
                                                              "n_number_-2..json",
                                                              "n_number_-NaN.json",
                                                              "n_number_minus_infinity.json",
                                                              "n_number_neg_int_starting_with_zero.json",
                                                              "n_number_neg_real_without_int_part.json",
                                                              "n_number_real_without_fractional_part.json",
                                                              "n_number_with_leading_zero.json"};

    for (const auto& test_suite_entry : fs::directory_iterator(test_suite_path)) {
        std::cout << "Parsing file named: " << test_suite_entry.path() << '\n' << std::flush;

        const bool throws = check_if_throws([&]() { return json::from_file(test_suite_entry.path()); });

        if (!acceptable_cases.count(test_suite_entry.path().filename())) CHECK(throws);
    }
}

// =============================
// --- Type conversion tests ---
// =============================

TEST_CASE_TEMPLATE("To-JSON-array conversions are handled correctly", T, //
                   json::Array,                                          //
                   std::vector<int>,                                     //
                   std::list<int>,                                       //
                   std::initializer_list<int>,                           //
                   std::deque<int>,                                      //
                   std::forward_list<int>,                               //
                   std::set<int>                                         //
) {
    json::Node json;
    json["array"] = T{1, 2, 3};

    const auto& arr = json.at("array").get_array(); // will throw if node is not an array

    CHECK(arr.size() == 3);

    int expected_value = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) CHECK(it->get_number() == ++expected_value);
}

TEST_CASE("To-JSON-array conversions are handled correctly (for multidimensional arrays)") {
    json::Node json;
    json["array_1D"] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    json["array_2D"] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    json["array_3D"] = {
        {{1, 2},    {3, 4}},
        {{5, 6}, {7, 8, 9}}
    };

    CHECK(json.at("array_1D").to_string(json::Format::MINIMIZED) == "[1,2,3,4,5,6,7,8,9]");
    CHECK(json.at("array_2D").to_string(json::Format::MINIMIZED) == "[[1,2,3],[4,5,6],[7,8,9]]");
    CHECK(json.at("array_3D").to_string(json::Format::MINIMIZED) == "[[[1,2],[3,4]],[[5,6],[7,8,9]]]");
}

TEST_CASE_TEMPLATE("To-JSON-object conversions are handled correctly", T, //
                   json::Object,                                          //
                   std::map<std::string, int>,                            //
                   std::unordered_map<std::string, int>                   //
) {
    json::Node json;
    json["object"] = T{
        {"key_1", 1},
        {"key_2", 2}
    };

    const auto& obj = json.at("object").get_object(); // will throw if node is not an array

    CHECK(obj.size() == 2);
    CHECK(obj.at("key_1").get_number() == 1);
    CHECK(obj.at("key_2").get_number() == 2);
}

TEST_CASE_TEMPLATE("To-JSON-string conversions are handled correctly", T, //
                   json::String,                                         //
                   std::string,                                          //
                   std::string_view                                      //
) {
    json::Node json;
    json["string"] = T{"lorem ipsum"};
    CHECK(json.at("string").get_string() == "lorem ipsum");
}

TEST_CASE("To-JSON-string conversions are handled correctly (for literals)") {
    json::Node json;
    json["string"] = "lorem ipsum";
    CHECK(json.at("string").get_string() == "lorem ipsum");
}

TEST_CASE_TEMPLATE("To-JSON-number conversions are handled correctly", T, //
                   json::Number,                                          //
                   float,                                                 //
                   double,                                                //
                   int,                                                   //
                   unsigned int,                                          //
                   std::size_t,                                           //
                   long double,                                           //
                   char                                                   //
) {
    json::Node json;
    json["number"] = T(2);
    CHECK(json.at("number").get_number() == 2);
}

TEST_CASE_TEMPLATE("To-JSON-bool conversions are handled correctly", T, //
                   json::Bool,                                          //
                   bool                                                 //
) {
    json::Node json;
    json["bool"] = T(true);
    CHECK(json.at("bool").get_bool() == true);
}

TEST_CASE_TEMPLATE("To-JSON-null conversions are handled correctly", T, //
                   json::Null                                           //
) {
    json::Node json;
    json["null"] = T();
    CHECK(json.at("null").get_null() == json::Null());
    // nothing else converts to 'Null' so the only thing we really test is that 'Null == Null' works
}

// =============================
// --- Object node API tests ---
// =============================

TEST_CASE_TEMPLATE("JSON node can be implicitly converted to object only for 'Null'", T, //
                   json::Object, json::Array, json::String, json::Number, json::Bool, json::Null) {
    json::Node json = T();

    const bool throws = check_if_throws([&]() { json["key"] = "value"; });

    // Only 'Null' converts to 'Object' implicitly after 'operator[]'
    if (json.is_null() || json.is_object()) {
        CHECK(!throws);
        CHECK(json.is_object());
    }
    // other types should throw on conversion
    else {
        CHECK(throws);
    }
}

TEST_CASE("JSON object node API basics work as intended") {
    auto json = json::from_string(R"(
        {
            "string": "lorem ipsum",
            "number": 17,
            "null": null
        }
    )");

    CHECK(check_if_throws([&]() { auto val = json.at("non_existent_key"); }));
    CHECK(json.contains("string"));
    CHECK(json.at("string").get_string() == "lorem ipsum");
    CHECK(json.value_or("number", -5.) == 17.);
    CHECK(json.value_or("non_existent_key", -5.) == -5.);
}

// ========================
// --- Reflection tests ---
// ========================

// --- Simple reflection ---
// -------------------------

// Set up reusable "flat struct" with all the JSON datatypes that we can reuse for other tests
struct SimpleConfig {
    std::unordered_map<std::string, int> object;
    std::vector<int>                     array;
    std::string                          string;
    double                               number;
    bool                                 boolean;
    json::Null                           null;

    bool operator==(const SimpleConfig& other) const {
        return (this->object == other.object) && (this->array == other.array) && (this->string == other.string) &&
               (this->number == other.number) && (this->boolean == other.boolean) && (this->null == other.null);
    } // in C++20 we could just 'default' it for the same effect
};

void check_json_against_struct(const json::Node& json, const SimpleConfig& cfg) {
    CHECK(json.get_object().size() == 6);
    CHECK(json.at("object").get_object().size() == cfg.object.size());
    CHECK(json.at("object").at("key_1").get_number() == cfg.object.at("key_1"));
    CHECK(json.at("object").at("key_2").get_number() == cfg.object.at("key_2"));
    CHECK(json.at("array").get_array().size() == cfg.array.size());
    CHECK(json.at("array").get_array().at(0).get_number() == cfg.array[0]);
    CHECK(json.at("array").get_array().at(1).get_number() == cfg.array[1]);
    CHECK(json.at("array").get_array().at(2).get_number() == cfg.array[2]);
    CHECK(json.at("string").get_string() == cfg.string);
    CHECK(json.at("number").get_number() == cfg.number);
    CHECK(json.at("boolean").get_bool() == cfg.boolean);
    CHECK(json.at("null").get_null() == cfg.null);
}

const SimpleConfig test_simple_cfg = {
    {{"key_1", 1}, {"key_2", 2}}, //
    {4, 5, 6}, //
    "lorem ipsum", //
    0.5, //
    true, //
    json::Null{}  //
};

UTL_JSON_REFLECT(SimpleConfig, object, array, string, number, boolean, null);

TEST_CASE("JSON struct reflection works for simple flat structures") {
    const auto cfg = test_simple_cfg;

    // Test 'struct -> JSON' reflection
    const auto reflected_json = json::from_struct(cfg);
    check_json_against_struct(reflected_json, cfg);

    // Test 'JSON -> struct' reflection
    const auto reflected_cfg = reflected_json.to_struct<SimpleConfig>();
    CHECK(reflected_cfg == cfg);
}

// --- Nested struct reflection ---
// --------------------------------

struct NestedConfig {
    SimpleConfig substruct;
    bool         flag;

    bool operator==(const NestedConfig& other) const {
        return (this->substruct == other.substruct) && (this->flag == other.flag);
    }
};

void check_json_against_struct(const json::Node& json, const NestedConfig& cfg) {
    CHECK(json.get_object().size() == 2);
    check_json_against_struct(json.at("substruct"), cfg.substruct);
    CHECK(json.at("flag").get_bool() == cfg.flag);
}

const NestedConfig test_nested_cfg = {test_simple_cfg, false};

UTL_JSON_REFLECT(NestedConfig, substruct, flag);

TEST_CASE("JSON struct reflection works for nested structures") {
    const auto cfg = test_nested_cfg;

    // Test 'struct -> JSON' reflection
    const auto reflected_json = json::from_struct(cfg);
    check_json_against_struct(reflected_json, cfg);

    // Test 'JSON -> struct' reflection
    const auto reflected_cfg = reflected_json.to_struct<NestedConfig>();
    CHECK(reflected_cfg == cfg);
}

// --- Nested containers of reflected structs reflection ---
// ---------------------------------------------------------

struct NestedContainerConfig {
    std::map<std::string, std::vector<SimpleConfig>> map_of_subconfig_arrays;

    std::vector<std::vector<std::vector<SimpleConfig>>> subconfig_tensor;

    bool operator==(const NestedContainerConfig& other) const {
        return (this->map_of_subconfig_arrays == other.map_of_subconfig_arrays) &&
               (this->subconfig_tensor == other.subconfig_tensor);
    }
};

void check_json_against_struct(const json::Node& json, const NestedContainerConfig& cfg) {
    CHECK(json.get_object().size() == 2);

    const auto& object = json.at("map_of_subconfig_arrays").get_object();
    for (const auto& [key, val] : object) {

        const auto& array = val.get_array();
        for (std::size_t i = 0; i < array.size(); ++i) {

            const auto& reflected_json_of_simple_cfg = array.at(i);
            check_json_against_struct(reflected_json_of_simple_cfg, cfg.map_of_subconfig_arrays.at(key).at(i));
        }
    }

    const auto& tensor_ext_1 = json.at("subconfig_tensor").get_array();
    for (std::size_t i = 0; i < tensor_ext_1.size(); ++i) {

        const auto& tensor_ext_2 = tensor_ext_1.at(i).get_array();
        for (std::size_t j = 0; j < tensor_ext_2.size(); ++j) {

            const auto& tensor_ext_3 = tensor_ext_2.at(j).get_array();
            for (std::size_t k = 0; k < tensor_ext_3.size(); ++k) {

                const auto& tensor_element = tensor_ext_3.at(k);
                check_json_against_struct(tensor_element, cfg.subconfig_tensor.at(i).at(j).at(k));
            }
        }
    }
}

const NestedContainerConfig test_nested_container_cfg = {
    {{"subconfig_1", {test_simple_cfg, test_simple_cfg}},
     {"subconfig_2", {test_simple_cfg, test_simple_cfg, test_simple_cfg}}},
    {{{test_simple_cfg, test_simple_cfg}}}
};

UTL_JSON_REFLECT(NestedContainerConfig, map_of_subconfig_arrays, subconfig_tensor);

TEST_CASE("JSON struct reflection works for nested containers of reflected structures") {
    const auto cfg = test_nested_container_cfg;

    // Test 'struct -> JSON' reflection
    const auto reflected_json = json::from_struct(cfg);
    check_json_against_struct(reflected_json, cfg);

    // Test 'JSON -> struct' reflection
    const auto reflected_cfg = reflected_json.to_struct<NestedContainerConfig>();
    CHECK(reflected_cfg == cfg);
}
// if map-of-arrays of structs and 3D tensor of reflected structs are properly reflected in
// another struct then it seems pretty safe to assume that everything else should be possible too