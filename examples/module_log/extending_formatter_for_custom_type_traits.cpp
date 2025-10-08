#include "include/UTL/log.hpp"

using namespace utl;

// Several custom classes
struct Class1 { std::string to_string() const { return "Class 1"; }; };
struct Class2 { std::string to_string() const { return "Class 2"; }; };
struct Class3 { std::string to_string() const { return "Class 3"; }; };

// Type trait corresponding to those classes
template <class T, class = void>
struct has_to_string : std::false_type {};

template <class T>
struct has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

// Extend formatter to support anything that provides '.to_string()' member function
template <class T>
struct log::Formatter<T, std::enable_if_t<has_to_string<T>::value>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) {
        Formatter<std::string>{}(buffer, arg.to_string());
    }
};

int main() {
    // Test
    assert(log::stringify(Class1{}) == "Class 1");
    assert(log::stringify(Class2{}) == "Class 2");
    assert(log::stringify(Class3{}) == "Class 3");
}