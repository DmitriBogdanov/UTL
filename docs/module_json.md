[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::json

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/json.hpp)

**utl::json** module aims to provide an intuitive JSON manipulation API similar to [nlohmann_json](https://github.com/nlohmann/json) while being a bit more lightweight and providing better performance. Key features are:

- Simple integration (single header, less than `1k` lines of code)
- Intuitive API
- [Decent performance](#benchmarks)
- [Nice error messages](#error-handling)
- [Recursive class reflection](#structure-reflection)
- Doesn't introduce any invasive macros or operators
- Objects support transparent comparators (which means `std::string_view`, `const char*` and etc. can be used for lookup)
- All JSON types map to standard library containers (`std::map`, `std::vector`, etc.), no need to learn custom APIs
- Assignment & reflection support custom container types

> [!Note]
> Despite rather competitive performance, considerably faster parsing can be achieved with custom formatters, SIMD and unordered key optimizations (see [simdjson](https://github.com/simdjson/simdjson), [Glaze](https://github.com/stephenberry/glaze), [RapidJSON](https://github.com/Tencent/rapidjson)  and [yyjson](https://github.com/ibireme/yyjson)), this, however often comes at the expense of user convenience (like with *RapidJSON*) or features (*simdjson* is parser-only, *yyjson* is `C` and doesn't include reflection, *Glaze* has it all, but requires [C++23](https://en.cppreference.com/w/cpp/23)).

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Feature Support

| Feature                            | Implementation | Notes                                                                                        |
| ---------------------------------- | -------------- | -------------------------------------------------------------------------------------------- |
| Parsing                            | ✔              |                                                                                              |
| Serialization                      | ✔              |                                                                                              |
| JSON Formatting                    | ✔              |                                                                                              |
| JSON Validation                    | ✔              | Almost complete[¹](#tests) validation with proper error messages through exceptions          |
| Unicode Support                    | ✔              | Supports UTF-8                                                                               |
| Control Character Escape Sequences | ✔              |                                                                                              |
| Unicode Escape Sequences           | ✔              | Full support including UTF-16 surrogate pairs                                                |
| Trait-based Type Conversions       | ✔              |                                                                                              |
| Structure Reflection               | ✔              | Recursive reflection including nested types and containers[²](#complex-structure-reflection) |
| Compile-time JSON Schema           | ✘              | Outside the project scope, can be emulated with reflection                                   |
| Lazy Node Loading                  | ✘              | Outside the project scope                                                                    |

## Definitions

```cpp
// JSON Node
enum class Format { PRETTY, MINIMIZED };

class Node {
    // - Member Types -
    using object_type = std::map<std::string, Node, std::less<>>;
    using array_type  = std::vector<Node>;
    using string_type = std::string;
    using number_type = double;
    using bool_type   = bool;
    using null_type   = class{};
    
    // - Getters -
    template <class T>       T& get();
    template <class T> const T& get() const;
    
    object_type& get_object();
    array_type & get_array();
    string_type& get_string();
    number_type& get_number();
    bool_type  & get_bool();
    null_type  & get_null();

    const object_type& get_object() const;
    const array_type & get_array()  const;
    const string_type& get_string() const;
    const number_type& get_number() const;
    const bool_type  & get_bool()   const;
    const null_type  & get_null()   const;
    
    template <class T> bool is() const;
    
    bool is_object() const;
    bool is_array()  const;
    bool is_string() const;
    bool is_number() const;
    bool is_bool()   const;
    bool is_null()   const;
    
    template <class T>       T* get_if();
    template <class T> const T* get_if() const;
    
    // - Object methods -
    Node      & operator[](std::string_view key);
    const Node& operator[](std::string_view key) const;
    Node      &         at(std::string_view key);
    const Node&         at(std::string_view key) const;
    
    bool              contains(std::string_view key) const;
    template<class T> value_or(std::string_view key, const T &else_value);
    
    // - Array methods -
    Node      & operator[](std::size_t pos);
    const Node& operator[](std::size_t pos) const;
    Node      &         at(std::size_t pos);
    const Node&         at(std::size_t pos) const;
    
    void push_back(const Node&  node);
    void push_back(      Node&& node);
    
    // - Assignment -
    Node& operator=(const Node&) = default;
    Node& operator=(Node&&)      = default;
    
    template <class T> Node& operator=(const T& value); // type-trait based conversion
    
    // - Constructors -
    Node()            = default;
    Node(const Node&) = default;
    Node(Node&&)      = default;
    
    template <class T> Node(const T& value); // type-trait based conversion
    
    // Serializing
    std::string          to_string(                           Format format = Format::PRETTY) const;
    void                 to_file(const std::string& filepath, Format format = Format::PRETTY) const;
    template <class T> T to_struct()                                                          const;
};

// Typedefs
using Object = Node::object_type;
using Array  = Node::array_type;
using String = Node::string_type;
using Number = Node::number_type;
using Bool   = Node::bool_type;
using Null   = Node::null_type;

// Parsing
Node                    from_string(const std::string& chars   , unsigned int recursion_limit = 100);
Node                    from_file  (const std::string& filepath, unsigned int recursion_limit = 100);
template <class T> Node from_struct(const           T& value   );

Node literals::operator""_utl_json(const char* c_str, std::size_t c_str_size);

// Reflection
#define UTL_JSON_REFLECT(struct_name, ...)

template <class T> constexpr bool is_reflected_struct;
```

## Methods

### `Node` Class

#### Member types

> ```cpp
> using object_type = std::map<std::string, Node, std::less<>>;
> using array_type  = std::vector<Node>;
> using string_type = std::string;
> using number_type = double;
> using bool_type   = bool;
> using null_type   = class{};
> ```

Definitions of the types, corresponding to all possible JSON values according to [ECMA-404 specification](https://ecma-international.org/wp-content/uploads/ECMA-404.pdf): objects, arrays, strings, numbers, booleans and null.

#### Getters

> ```cpp
> template <class T>       T& get();
> template <class T> const T& get() const;
> ```

If JSON node holds the value of a type `T`, returns a reference to the value, otherwise, throws [std::bad_variant_access](https://en.cppreference.com/w/cpp/utility/variant/bad_variant_access).

**Note:** Similar to [std::get](https://en.cppreference.com/w/cpp/utility/variant/get).

> ```cpp
> object_type& get_object();
> array_type & get_array();
> string_type& get_string();
> number_type& get_number();
> bool_type  & get_bool();
> null_type  & get_null();
> 
> const object_type& get_object() const;
> const array_type & get_array()  const;
> const string_type& get_string() const;
> const number_type& get_number() const;
> const bool_type  & get_bool()   const;
> const null_type  & get_null()   const;
> ```

Shortcut versions of `T& get<T>()` for all possible value types.

> ```cpp
> template <class T> bool is() const;
> ```

Returns whether JSON node contains a value of a type `T`.

**Note:** Similar to [std::holds_alternative](https://en.cppreference.com/w/cpp/utility/variant/holds_alternative).

> ```cpp
> bool is_object() const;
> bool is_array()  const;
> bool is_string() const;
> bool is_number() const;
> bool is_bool()   const;
> bool is_null()   const;
> ```

Shortcut versions of `T& is<T>()` for all possible value types.

> ```cpp
> template <class T>       T* get_if();
> template <class T> const T* get_if() const;
> ```

Returns a `T*` pointer to the value stored at the JSON node, if stored value has a different type than `T`, returns [nullptr](https://en.cppreference.com/w/cpp/language/nullptr).

**Note:** Similar to [std::get_if](https://en.cppreference.com/w/cpp/utility/variant/get_if).

#### Object methods

> [!Important]
> Object methods can only be called for nodes that contain an object, incorrect node type will cause methods below to throw an exception.

> ```cpp
> Node      & operator[](std::string_view key);
> const Node& operator[](std::string_view key) const;
> ```

Returns a reference to the node corresponding to a given `key` in the JSON object, performs an insertion if such key does not already exist.

**Note:** If current node is `null_type` overload **(1)** will convert it to `object_type` and perform an insertion. This allows for a more natural syntax.

> ```cpp
> Node      & at(std::string_view key);
> const Node& at(std::string_view key) const;
> ```

Returns a reference to the node corresponding to a given `key` in the JSON object, throws an exception if such key does not exist. 

> ```cpp
> bool contains(std::string_view key) const;
> ```

Returns whether JSON object node contains an entry with given `key`.

> ```cpp
> template<class T> value_or(std::string_view key, const T &else_value);
> ```

Returns value stored at given `key` in the JSON object, if no such key can be found returns `else_value`.

**Note:** Logically equivalent to `object.contains(key) ? object.at(key).get<T>() : else_value`, but faster.

#### Array methods

> [!Important]
> Array methods can only be called for nodes that contain an array, incorrect node type will cause methods below to throw an exception.

> ```cpp
> Node      & operator[](std::size_t pos);
> const Node& operator[](std::size_t pos) const;
> ```

Returns a reference to the node at given `pos`. 

> ```cpp
> Node      &         at(std::size_t pos);
> const Node&         at(std::size_t pos) const;
> ```

Returns a reference to the node at given `pos`, throws an exception if index is out of bounds.

> ```cpp
> void push_back(const Node&  node);
> void push_back(      Node&& node);
> ```

Inserts a new node at the end of JSON array.

**Note:** If current node is `null_type` the method will convert it to `array_type` and perform an insertion. This allows for a more natural syntax.

#### Assignment & Constructors

> ```cpp
> template <class T> Node& operator=(const T& value);
> template <class T> Node(const T& value);
> ```

Converting assignment & constructors. Tries to convert `T` to one of the possible JSON types based on `T` traits, conversions and provided methods. If no such conversion is possible, SFINAE rejects the overload.

#### Serializing

> ```cpp
> std::string to_string(Format format = Format::PRETTY) const;
> ```

Serializes JSON node to a string using a given `format`.

> ```cpp
> void to_file(const std::string& filepath, Format format = Format::PRETTY) const;
> ```

Serializes JSON node to the file at `filepath` using a given `format`.

**Note:** Missing directories from `filepath` will be created automatically.

> ```cpp
> template <class T> T to_struct() const;
> ```

Serializes JSON node to the structure / class object of type `T`.

Type `T` must be reflected with `UTL_JSON_REFLECT()` macro, otherwise compilation fails with a proper assertion.

### Parsing

> ```cpp
> Node from_string(const std::string& buffer, unsigned int recursion_limit = 100);
> ```

Parses JSON from a given string `buffer`.

**Note:** JSON parsers need recursion depth limit to prevent malicious inputs (such as 100'000+ nested object opening braces) from causing stack overflows, instead we get a controllable `std::runtime_error`.

> ```cpp
> Node from_file(const std::string& filepath, unsigned int recursion_limit = 100);
> ```

Parses JSON from the file at `filepath`.

> ```cpp
> template <class T> Node from_struct(const T& value);
> ```

Parses JSON from structure / class object `value`.

Type `T` must be reflected with `UTL_JSON_REFLECT()` macro, otherwise compilation fails with a proper assertion.

> ```cpp
> Node literals::operator""_utl_json(const char* c_str, std::size_t c_str_size);
> ```

`json::Node` custom literals.

### Typedefs

> ```cpp
> using Object = Node::object_type;
> using Array  = Node::array_type;
> using String = Node::string_type;
> using Number = Node::number_type;
> using Bool   = Node::bool_type;
> using Null   = Node::null_type;
> ```

Shorter typedefs for all existing JSON value types.

### Reflection

> ```cpp
> #define UTL_JSON_REFLECT(struct_name, ...)
> ```

Reflects structure / class `struct_name` with member variables `...`.

Declaring this macro defines methods `Node::to_struct<struct_name>()` and `from_struct(const struct_name&)` for parsing and serialization.

**Note 1:** Reflection supports nested classes, each class should be reflected with a macro and `to_struct()` / `from_struct()` will call each other recursively whenever appropriate. Containers of reflected classes are also supported with any level of nesting. See [examples](#structure-reflection).

**Note 2:** Reflection does not impose any strict limitations on member variable types, it uses the same set of type traits as other methods to deduce appropriate conversions. It is expected however, that array-like member variables should support `.resize()` ([std::vector](https://en.cppreference.com/w/cpp/container/vector) and [std::list](https://en.cppreference.com/w/cpp/container/list) satisfy that) or provide an API similar to [std::array](https://en.cppreference.com/w/cpp/container/array). For object-like types it is expected that new elements can be inserted with `operator[]` ([std::map](https://en.cppreference.com/w/cpp/container/map) and [std::unordered_map](https://en.cppreference.com/w/cpp/container/unordered_map) satisfy that).

> ```cpp
> template <class T> constexpr bool is_reflected_struct;
> ```

Evaluates to `true` if `T` was reflected with `UTL_JSON_REFLECT()`, `false` otherwise.

## Examples 

### Parse/serialize JSON

[ [Run this code](https://godbolt.org/z/aEja9Gf1E) ]

```cpp
using namespace utl;

// Serialize JSON
json::Node config;

config["auxiliary_info"]       = true;
config["date"]                 = "2024.04.02";
config["options"]["grid_size"] = 120;
config["options"]["phi_order"] = 5;
config["scaling_functions"]    = { "identity", "log10" };
config["time_steps"]           = 500;
config["time_period"]          = 1.24709e+2;

config.to_file("config.json");

// Parse JSON
config = json::from_file("config.json");

std::cout << config.to_string();
```

Output:
```
{
    "auxiliary_info": true,
    "date": "2024.04.02",
    "options": {
        "grid_size": 120,
        "phi_order": 5
    },
    "scaling_functions": [
        "identity",
        "log10"
    ],
    "time_period": 124.709,
    "time_steps": 500
}
```

### Setters & type conversions

[ [Run this code](https://godbolt.org/z/6qM5cMTG8) ]

```cpp
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
```

### Getters

[ [Run this code](https://godbolt.org/z/oohca1aaP) ]

```cpp
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
```

### Formatting

[ [Run this code](https://godbolt.org/z/b1MnbcaE6) ]

```cpp
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
```

Output:
```
--- Prettified JSON ---

{
    "array": [
        1,
        2,
        3
    ],
    "object": {
        "key_1": 3.14,
        "key_2": 6.28
    },
    "string": "lorem ipsum"
}

--- Minimized JSON ---

{"array":[1,2,3],"object":{"key_1":3.14,"key_2":6.28},"string":"lorem ipsum"}
```

### Error handling

[ [Run this code](https://godbolt.org/z/Ysfj3fYxb) ]

```cpp
using namespace utl;

const auto invalid_json = R"(
    {
        "key_1": "value_1",
        "key_2":  value_2",
        "key_3": "value_3"
    }
)";

try {
    [[maybe_unused]] auto res = json::from_string(invalid_json);
}
catch (std::runtime_error &e) {
    std::cerr << "ERROR: Caught exception:\n\n" << e.what();
}
```

Output:
```
ERROR: Caught exception:

JSON node selector encountered unexpected marker symbol {v} at pos 48 (should be one of {0123456789{["tfn}).
Line 4:         "key_2":  value_2",
        ------------------^-------- [!]
```

### Structure reflection

> [!Important]
> When compiling with [MSVC](https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B) use [`/Zc:preprocessor`](https://learn.microsoft.com/en-us/cpp/build/reference/zc-preprocessor) to enable standard-compliant preprocessor. Default MSVC preprocessor is notoriously non-compliant due to legacy reasons and might not handle macro expansion properly.

[ [Run this code](https://godbolt.org/z/arv1sb46r) ]

```cpp
struct Config {
    bool        auxiliary_info = true;
    std::string date           = "2024.04.02";

    struct Options {
        int grid_size = 120;
        int phi_order = 5;
    } options;

    std::vector<std::string> scaling_functions = {"identity", "log10"};
    std::size_t              time_steps        = 500;
    double                   time_period       = 1.24709e+2;
};

UTL_JSON_REFLECT(Config, auxiliary_info, date, options, scaling_functions, time_steps, time_period);
UTL_JSON_REFLECT(Config::Options, grid_size, phi_order);

// ...

using namespace utl;

// Parse JSON from struct
auto       config = Config{};
json::Node json   = json::from_struct(config);

// Test the result
std::cout << "--- Struct to JSON ---\n" << json.to_string();

// Serialize JSON to struct
auto serialized_config = json.to_struct<Config>();

// Test the result
assert( config.auxiliary_info    == serialized_config.auxiliary_info    );
assert( config.date              == serialized_config.date              );
assert( config.options.grid_size == serialized_config.options.grid_size );
// ...and so on
```

Output:
```
--- Struct to JSON ---
{
    "auxiliary_info": true,
    "date": "2024.04.02",
    "options": {
        "grid_size": 120,
        "phi_order": 5
    },
    "scaling_functions": [
        "identity",
        "log10"
    ],
    "time_period": 124.709,
    "time_steps": 500
}
```

### Complex structure reflection

> [!Note]
> There are no particular limitations on what kinds of nested structures can be reflected, as long as there is a logically sound path to converting one thing to another `utl::json` will figure out a way.

[ [Run this code](https://godbolt.org/z/89KoPb3cf) ]

```cpp
// Set up some complex nested structs
struct Point {
    double x, y, z;
};

struct Task {
    std::string input_path;
    std::string output_path;
    double      time_limit;
};

struct TaskList {
    std::map<std::string, Task> map_of_tasks;
    // this is fine
    
    std::vector<std::vector<Point>> matrix_of_points;
    // this is also fine
    
    // std::vector<std::vector<std::vector<std::map<std::string, Point>>>> tensor_of_maps_of_points;
    // ... this would also be fine
    
    // std::array<std::unordered_map<std::string, Task>, 4> array_of_maps_of_tasks;
    // ... and so will be this
};

UTL_JSON_REFLECT(Point, x, y, z);
UTL_JSON_REFLECT(Task, input_path, output_path, time_limit);
UTL_JSON_REFLECT(TaskList, map_of_tasks, matrix_of_points);

// ...

using namespace utl;

const TaskList task_list = {
    // Map of tasks
    {
        { "task_1", { "input_1.dat", "output_1.dat", 170. } },
        { "task_2", { "input_2.dat", "output_2.dat", 185. } }
    },
    // Matrix of 3D points
    {
        { { 0, 0, 0}, { 1, 0, 0 } },
        { { 0, 1, 0}, { 0, 0, 1 } }
    }
};

// Parse JSON from struct,
// this also doubles as a way of stringifying structs for debugging
std::cout << json::from_struct(task_list).to_string();
```

Output:
```
{
    "map_of_tasks": {
        "task_1": {
            "input_path": "input_1.dat",
            "output_path": "output_1.dat",
            "time_limit": 170
        },
        "task_2": {
            "input_path": "input_2.dat",
            "output_path": "output_2.dat",
            "time_limit": 185
        }
    },
    "matrix_of_points": [
        [
            {
                "x": 0,
                "y": 0,
                "z": 0
            },
            {
                "x": 1,
                "y": 0,
                "z": 0
            }
        ],
        [
            {
                "x": 0,
                "y": 1,
                "z": 0
            },
            {
                "x": 0,
                "y": 0,
                "z": 1
            }
        ]
    ]
}
```

## Tests

`utl::json` parsing was [tested](https://github.com/DmitriBogdanov/UTL/tree/master/tests/module_json/) using the standard [RFC-8259](https://datatracker.ietf.org/doc/html/rfc8259) compliance [testing suite](https://github.com/nst/JSONTestSuite/) with following metrics:

| Metric | Compliance | Note |
| - | - | - |
| Parser accepts valid RFC-8259 JSON | **100%** | Full conformance |
| Parser rejects invalid RFC-8259 JSON | **93.6%** | Missing conformance of 6.4% is due to parser imposing less restrictions on the floating point format, it will accepts values such as `2.`, `01`, `2.e+3` and etc., which go beyond the default JSON specification. |

Parsing and serialization also satisfies [C++ `<charconv>`](https://en.cppreference.com/w/cpp/header/charconv) float round-trip guarantees (which means floats serialized by `utl::json` will be recovered to the exact same value when parsed again by the library).

## Benchmarks

[Benchmarks](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks/module_json/) for parsing and serializing of minimized JSON data corresponding to various entries in the [test suite](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks/data/). 

```
====== BENCHMARKING ON DATA: `strings.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               16.03 |               62.40 |    4.6% |      0.81 | `utl::json`
|    25.3% |               63.43 |               15.76 |    0.4% |      3.11 | `nlohmann`
|    47.5% |               33.71 |               29.66 |    0.3% |      1.65 | `PicoJSON`
|    94.4% |               16.98 |               58.90 |    0.2% |      0.83 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               15.85 |               63.08 |    0.7% |      0.78 | `utl::json`
|    24.4% |               64.88 |               15.41 |    0.2% |      3.19 | `nlohmann`
|    45.6% |               34.77 |               28.76 |    0.5% |      1.69 | `PicoJSON`
|    88.1% |               17.99 |               55.60 |    0.4% |      0.88 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.90 |               84.05 |    0.2% |      0.59 | `utl::json`
|    42.2% |               28.22 |               35.44 |    0.4% |      1.39 | `nlohmann`
|    33.1% |               36.00 |               27.78 |    0.2% |      1.76 | `PicoJSON`
|    75.5% |               15.77 |               63.42 |    0.5% |      0.77 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               12.67 |               78.93 |    0.7% |      0.62 | `utl::json`
|    41.7% |               30.41 |               32.88 |    3.4% |      1.52 | `nlohmann`
|    33.2% |               38.20 |               26.18 |    0.6% |      1.88 | `PicoJSON`
|    75.9% |               16.69 |               59.92 |    1.2% |      0.82 | `RapidJSON`


====== BENCHMARKING ON DATA: `numbers.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               10.95 |               91.35 |    0.7% |      0.54 | `utl::json`
|    22.9% |               47.79 |               20.92 |    0.6% |      2.35 | `nlohmann`
|    28.2% |               38.78 |               25.79 |    0.5% |      1.89 | `PicoJSON`
|   144.6% |                7.57 |              132.11 |    0.4% |      0.37 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               11.26 |               88.84 |    0.2% |      0.55 | `utl::json`
|    22.6% |               49.88 |               20.05 |    0.5% |      2.51 | `nlohmann`
|    28.1% |               39.99 |               25.00 |    1.4% |      1.96 | `PicoJSON`
|   128.9% |                8.73 |              114.51 |    3.1% |      0.43 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.18 |               89.44 |    1.0% |      0.55 | `utl::json`
|    65.2% |               17.14 |               58.33 |    0.4% |      0.85 | `nlohmann`
|    13.9% |               80.48 |               12.43 |    0.6% |      3.95 | `PicoJSON`
|    63.9% |               17.51 |               57.13 |    1.1% |      0.85 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               13.43 |               74.48 |    1.0% |      0.66 | `utl::json`
|    65.0% |               20.64 |               48.44 |    0.6% |      1.02 | `nlohmann`
|    16.3% |               82.14 |               12.17 |    0.4% |      4.04 | `PicoJSON`
|    74.3% |               18.06 |               55.38 |    0.4% |      0.89 | `RapidJSON`


====== BENCHMARKING ON DATA: `twitter.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |                2.48 |              402.89 |    0.5% |      0.12 | `utl::json`
|    45.4% |                5.47 |              182.76 |    0.1% |      0.27 | `nlohmann`
|    77.3% |                3.21 |              311.24 |    0.2% |      0.16 | `PicoJSON`
|   221.6% |                1.12 |              892.87 |    1.3% |      0.05 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |                2.80 |              356.62 |    0.5% |      0.14 | `utl::json`
|    45.2% |                6.21 |              161.12 |    0.4% |      0.31 | `nlohmann`
|    74.9% |                3.74 |              267.05 |    0.6% |      0.18 | `PicoJSON`
|   207.1% |                1.35 |              738.54 |    1.5% |      0.07 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |                1.20 |              834.42 |    2.0% |      0.06 | `utl::json`
|    52.8% |                2.27 |              440.27 |    2.0% |      0.13 | `nlohmann`
|    29.0% |                4.14 |              241.62 |   21.3% |      0.20 | `PicoJSON`
|    94.3% |                1.27 |              786.88 |    0.6% |      0.06 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |                1.60 |              626.16 |    0.4% |      0.08 | `utl::json`
|    57.9% |                2.76 |              362.32 |    0.3% |      0.14 | `nlohmann`
|    44.4% |                3.60 |              277.81 |    0.4% |      0.18 | `PicoJSON`
|    91.1% |                1.75 |              570.23 |    0.6% |      0.09 | `RapidJSON`


====== BENCHMARKING ON DATA: `apache_builds.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |                0.47 |            2,134.76 |    0.7% |      0.02 | `utl::json`
|    41.3% |                1.14 |              881.03 |    1.6% |      0.06 | `nlohmann`
|    77.4% |                0.61 |            1,652.53 |    0.6% |      0.03 | `PicoJSON`
|   172.2% |                0.27 |            3,675.37 |    1.3% |      0.01 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |                0.49 |            2,053.48 |    0.3% |      0.02 | `utl::json`
|    38.7% |                1.26 |              794.02 |    1.4% |      0.06 | `nlohmann`
|    71.5% |                0.68 |            1,468.49 |    0.7% |      0.03 | `PicoJSON`
|   163.1% |                0.30 |            3,348.22 |    0.8% |      0.01 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |                0.30 |            3,283.66 |    0.7% |      0.02 | `utl::json`
|    57.5% |                0.53 |            1,889.72 |    0.8% |      0.03 | `nlohmann`
|    46.2% |                0.66 |            1,518.06 |    0.6% |      0.03 | `PicoJSON`
|   100.5% |                0.30 |            3,301.60 |    2.2% |      0.02 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |                0.40 |            2,488.96 |    2.3% |      0.02 | `utl::json`
|    61.9% |                0.65 |            1,540.09 |    1.5% |      0.03 | `nlohmann`
|    53.8% |                0.75 |            1,338.57 |    1.6% |      0.04 | `PicoJSON`
|    98.9% |                0.41 |            2,461.15 |    3.0% |      0.02 | `RapidJSON`
```

> [!Important]
> Clang with `libc++` below version 20 does not implement C++17 `std::to_chars`, forcing the library to fallback onto a significantly slower number parsing routine. This is mostly a MacOS issue. Fallback presence can be detected with `#ifdef UTL_JSON_FROM_CHARS_FALLBACK`.

### Some thoughts on implementation

The main weak-point of `utl::json` from the performance point of view is parsing  of object-heavy JSONs.

Unfortunately, the issue is mostly caused by `std::map` insertion & iteration, which dominates the runtime. A truly suitable for the purpose container doesn't really exist in the standard library, and would need a custom implementation like in `RapidJSON`, which would reduce the standard library interoperability thus going against the main purpose of this library which is simplicity of use.

Flat maps maps seem like the way to go, slotting in a custom flat map implementation into `json::_object_type_impl` allowed `utl::json` to beat `RapidJSON` on all serializing tasks and significantly closed the gap of `database.json` parsing:

```
// Using associative wrapper for std::vector of pairs instead of std::map we can bridge the performance gap.
// General-case usage however suffers, which is why this decision was ruled against.
// Measurement below was done a database very similar to 'twitter.json' but with even more objects & nesting.

====== BENCHMARKING ON DATA: `database.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               13.22 |               75.65 |    0.6% |      0.95 | `utl::json`
|    24.7% |               53.57 |               18.67 |    3.6% |      3.93 | `nlohmann`
|    43.5% |               30.42 |               32.87 |    0.7% |      2.19 | `PicoJSON`
|   139.5% |                9.48 |              105.51 |    0.2% |      0.68 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               14.28 |               70.02 |    0.5% |      1.03 | `utl::json`
|    25.8% |               55.36 |               18.06 |    0.5% |      3.99 | `nlohmann`
|    41.1% |               34.75 |               28.78 |    0.2% |      2.50 | `PicoJSON`
|   130.4% |               10.95 |               91.33 |    0.4% |      0.79 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |                8.47 |              118.00 |    0.2% |      0.61 | `utl::json`
|    35.7% |               23.74 |               42.12 |    0.3% |      1.71 | `nlohmann`
|    23.6% |               35.85 |               27.89 |    0.4% |      2.57 | `PicoJSON`
|    94.4% |                8.97 |              111.43 |    0.3% |      0.65 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               10.09 |               99.12 |    0.4% |      0.73 | `utl::json`
|    39.2% |               25.74 |               38.85 |    0.5% |      1.85 | `nlohmann`
|    26.8% |               37.61 |               26.59 |    0.4% |      2.71 | `PicoJSON`
|    93.7% |               10.77 |               92.87 |    0.2% |      0.78 | `RapidJSON`
```
