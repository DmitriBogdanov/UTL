#include "include/UTL/log.hpp"

using namespace utl;

// Several custom classes
struct class_1 { std::string to_string() const { return "Class 1"; }; };
struct class_2 { std::string to_string() const { return "Class 2"; }; };
struct class_3 { std::string to_string() const { return "Class 3"; }; };

// Type trait corresponding to those classes
template <class T, class = void>
struct has_to_string : std::false_type {};

template <class T>
struct has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

// Extend formatter to support anything that provides '.to_string()' member function
template <class T>
struct log::formatter<T, std::enable_if_t<has_to_string<T>::value>> {
    template <class Buffer>
    void operator()(Buffer& buffer, const T& arg) {
        formatter<std::string>{}(buffer, arg.to_string());
    }
};

int main() {
    // Test
    assert(log::stringify(class_1{}) == "Class 1");
    assert(log::stringify(class_2{}) == "Class 2");
    assert(log::stringify(class_3{}) == "Class 3");
}