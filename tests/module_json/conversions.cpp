#include "tests/common.hpp"

#include "include/UTL/json.hpp"

// _______________________ INCLUDES _______________________

#include <deque>         // deque<>
#include <forward_list>  // forward_list<>
#include <list>          // list<>
#include <set>           // set<>
#include <unordered_map> // unordered_map<>

// ____________________ IMPLEMENTATION ____________________

TEST_CASE_TEMPLATE("Conversions / To-JSON-array", T, //
                   json::Array,                      //
                   std::vector<int>,                 //
                   std::list<int>,                   //
                   std::initializer_list<int>,       //
                   std::deque<int>,                  //
                   std::forward_list<int>,           //
                   std::set<int>                     //
) {
    json::Node json;
    json["array"] = T{1, 2, 3};

    const auto& arr = json.at("array").get_array(); // will throw if node is not an array

    CHECK(arr.size() == 3);

    int expected_value = 0;
    for (auto it = arr.begin(); it != arr.end(); ++it) CHECK(it->get_number() == ++expected_value);
}

TEST_CASE("Conversions /  To-JSON-array (multidimensional case)") {
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

TEST_CASE_TEMPLATE("Conversions /  To-JSON-object", T,  //
                   json::Object,                        //
                   std::map<std::string, int>,          //
                   std::unordered_map<std::string, int> //
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

TEST_CASE_TEMPLATE("Conversions /  To-JSON-string", T, //
                   json::String,                       //
                   std::string,                        //
                   std::string_view                    //
) {
    json::Node json;
    json["string"] = T{"lorem ipsum"};
    CHECK(json.at("string").get_string() == "lorem ipsum");
}

TEST_CASE("Conversions / To-JSON-string (literal case)") {
    json::Node json;
    json["string"] = "lorem ipsum";
    CHECK(json.at("string").get_string() == "lorem ipsum");
}

TEST_CASE_TEMPLATE("Conversions / To-JSON-number", T, //
                   json::Number,                      //
                   float,                             //
                   double,                            //
                   int,                               //
                   unsigned int,                      //
                   std::size_t,                       //
                   long double,                       //
                   char                               //
) {
    json::Node json;
    json["number"] = T(2);
    CHECK(json.at("number").get_number() == 2);
}

TEST_CASE_TEMPLATE("Conversions / To-JSON-bool", T, //
                   json::Bool,                      //
                   bool                             //
) {
    json::Node json;
    json["bool"] = T(true);
    CHECK(json.at("bool").get_bool() == true);
}

TEST_CASE_TEMPLATE("Conversions / To-JSON-null", T, //
                   json::Null                       //
) {
    json::Node json;
    json["null"] = T();
    CHECK(json.at("null").get_null() == json::Null());
    // nothing else converts to 'Null' so the only thing we really test is that 'Null == Null' works
}