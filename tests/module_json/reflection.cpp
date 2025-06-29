#include "tests/common.hpp"

#include "include/UTL/json.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

// =========================
// --- Simple reflection ---
// =========================

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

TEST_CASE("Reflection / Flat struct") {
    const auto cfg = test_simple_cfg;

    // Test 'struct -> JSON' reflection
    const auto reflected_json = json::from_struct(cfg);
    check_json_against_struct(reflected_json, cfg);

    // Test 'JSON -> struct' reflection
    const auto reflected_cfg = reflected_json.to_struct<SimpleConfig>();
    CHECK(reflected_cfg == cfg);
}

// ================================
// --- Nested struct reflection ---
// ================================

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

TEST_CASE("Reflection / Nested reflected structs") {
    const auto cfg = test_nested_cfg;

    // Test 'struct -> JSON' reflection
    const auto reflected_json = json::from_struct(cfg);
    check_json_against_struct(reflected_json, cfg);

    // Test 'JSON -> struct' reflection
    const auto reflected_cfg = reflected_json.to_struct<NestedConfig>();
    CHECK(reflected_cfg == cfg);
}

// =========================================================
// --- Nested containers of reflected structs reflection ---
// =========================================================

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

TEST_CASE("Reflection / Nested containers of reflected structs") {
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