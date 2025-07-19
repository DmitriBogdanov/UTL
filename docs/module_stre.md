[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/UTL/blob/master/LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::stre

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/stre.hpp)

**utl::stre** (aka **str**ing **e**xpansions) header contains implementations of most commonly used string utils.

**Motivation:** Despite the seeming triviality of the topic a lot of implementations found online are either horribly inefficient or contain straight up bugs in some edge cases. Here, the goal is to "get it right" once so no time would be spent reinventing the wheel in the future.

## Definitions

```cpp
// Trimming
std::string trim_left (std::string str, char trimmed_char = ' ');
std::string trim_right(std::string str, char trimmed_char = ' ');
std::string trim      (std::string str, char trimmed_char = ' ');

// Padding
std::string pad_left (std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad      (std::string_view str, std::size_t length, char padding_char = ' ');

std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 10);

// Case conversions
std::string to_lower(std::string str);
std::string to_upper(std::string str);

// Substring checks
bool starts_with(std::string_view str, std::string_view substr);
bool ends_with  (std::string_view str, std::string_view substr);
bool contains   (std::string_view str, std::string_view substr);

// Token manipulation
std::string replace_all_occurrences(std::string str, std::string_view from, std::string_view to);

std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter, bool keep_empty_tokens = false);

// Other utils
std::string repeat_char  (            char  ch, std::size_t repeats);
std::string repeat_string(std::string_view str, std::size_t repeats);

std::string escape_control_chars(std::string_view str);

std::size_t index_of_difference(std::string_view str_1, std::string_view str_2);
```

> [!Note]
> Functions that can utilize mutable input string for a more efficient implementation take `std::string` by value. This means [r-value](https://en.cppreference.com/w/cpp/language/value_category) arguments are moved & reused, while l-values are copied.

## Methods

### Trimming

```cpp
std::string trim_left (std::string str, char trimmed_char = ' ');
std::string trim_right(std::string str, char trimmed_char = ' ');
std::string trim      (std::string str, char trimmed_char = ' ');
```

Trims characters equal to `trimmed_char` from the left / right / both sides of the string `str`.

### Padding

```cpp
std::string pad_left (std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad      (std::string_view str, std::size_t length, char padding_char = ' ');
```

Pads string `str` with character `padding_char` from left / right / both sides until it reaches given `length`.

**Note:** If `str.size() >= length` the string is left unchanged.

> ```cpp
> std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 10);
> ```

Pads a `number` with leading zeroes until it reaches given `length`. Useful for numbering files/data entries are labeled in lexicographic order.

**Note:** If `number` has more than `length` digits, resulting string is the same as `std::to_string(number)`.

### Case conversions

```cpp
std::string to_lower(std::string str);
```

Replaces all uppercase letters `ABCDEFGHIJKLMNOPQRSTUVWXYZ` in the string `str` with corresponding lowercase letters `abcdefghijklmnopqrstuvwxyz`.

```cpp
std::string to_upper(std::string str);
```

Replaces all lowercase letters `abcdefghijklmnopqrstuvwxyz` in the string `str` with corresponding uppercase letters `ABCDEFGHIJKLMNOPQRSTUVWXYZ`.

### Substring checks

```cpp
bool starts_with(std::string_view str, std::string_view substr);
bool ends_with  (std::string_view str, std::string_view substr);
bool contains   (std::string_view str, std::string_view substr);
```

Returns `true` if string `str` starts with / ends with / contains the substring `substr`.

### Token manipulation

```cpp
std::string replace_all_occurrences(std::string str, std::string_view from, std::string_view to);
```

Scans through the string `str` and replaces all occurrences of substring `from` with a string `to`.

```cpp
std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter, bool keep_empty_tokens = false);
```

Splits string `str` into a vector of `std::string` tokens based on `delimiter`.

By default `keep_empty_tokens` is `false` and `""` is not considered to be a valid token — in case of leading / trailing / repeated delimiters, only non-empty tokens are going to be inserted into the resulting vector. Setting `keep_empty_tokens` to `true` overrides this behavior and keeps all the empty tokens intact.

### Other utils

> ```cpp
> std::string repeat_char  (            char  ch, std::size_t repeats);
> std::string repeat_string(std::string_view str, std::size_t repeats);
> ```

Repeats given character or string a given number of times and returns the resulting string.

```cpp
std::string escape_control_chars(std::string_view str);
```

Escapes all control & non-printable characters in the string `str` using standard C++ notation (see [corresponding example](#using-other-utilities) for a better idea).

Useful when printing strings to the terminal during logging & debugging.

```cpp
std::size_t index_of_difference(std::string_view str_1, std::string_view str_2);
```

Returns the index of the first character that is different between string `str_1` and `str_2`.

When both strings are the same, returns `str_1.size()`.

Throws `std::logical_error` if `str_1.size() != str_2.size()`.

## Examples

### Trimming strings

[ [Run this code](https://godbolt.org/z/hjf1anMcb) ]

```cpp
using namespace utl;

assert(stre::trim_left ("   lorem ipsum   ") ==    "lorem ipsum   ");
assert(stre::trim_right("   lorem ipsum   ") == "   lorem ipsum"   );
assert(stre::trim      ("   lorem ipsum   ") ==    "lorem ipsum"   );

assert(stre::trim("__ASSERT_MACRO__", '_') == "ASSERT_MACRO");
```

### Padding strings

[ [Run this code](https://godbolt.org/z/nG7zdqP11) ]

```cpp
using namespace utl;

assert(stre::pad_left ("value", 9) == "    value" );
assert(stre::pad_right("value", 9) == "value    " );
assert(stre::pad      ("value", 9) == "  value  " );

assert(stre::pad(" label ", 15, '-') == "---- label ----");

assert(stre::pad_with_leading_zeroes(17, 5) == "00017");
```

### Converting string case

[ [Run this code](https://godbolt.org/z/YjnhjYqa3) ]

```cpp
using namespace utl;

assert(stre::to_lower("Lorem Ipsum") == "lorem ipsum");
assert(stre::to_upper("lorem ipsum") == "LOREM IPSUM");
```

### Substring checks

[ [Run this code](https://godbolt.org/z/v8Enfn9a6) ]

```cpp
using namespace utl;

assert(stre::starts_with("lorem ipsum", "lorem"));
assert(stre::ends_with  ("lorem ipsum", "ipsum"));
assert(stre::contains   ("lorem ipsum", "em ip"));
```

### Token manipulation

[ [Run this code](https://godbolt.org/z/osMK6x1be) ]

```cpp
using namespace utl;

// Replacing tokens
assert(stre::replace_all_occurrences("xxxAAxxxAAxxx",  "AA",  "BBB") == "xxxBBBxxxBBBxxx" );

// Splitting by delimiter
auto tokens = stre::split_by_delimiter("aaa,bbb,ccc,", ",");
assert(tokens.size() == 3);
assert(tokens[0] == "aaa");
assert(tokens[1] == "bbb");
assert(tokens[2] == "ccc");

// Splitting by complex delimiter while keeping the empty tokens
tokens = stre::split_by_delimiter("(---)lorem(---)ipsum", "(---)", true);
assert(tokens.size() == 3);
assert(tokens[0] == ""     );
assert(tokens[1] == "lorem");
assert(tokens[2] == "ipsum");
```

### Other utilities

[ [Run this code](https://godbolt.org/z/xKG6qM79n) ]

```cpp
using namespace utl;

// Repeating chars/strings
assert(stre::repeat_char  (  'h', 7) == "hhhhhhh"        );
assert(stre::repeat_string("xo-", 5) == "xo-xo-xo-xo-xo-");

// Escaping control chars in a string   
const std::string text = "this text\r will get messed up due to\r carriage returns.";
std::cout
    << "Original string prints like this:\n" <<                            text  << "\n\n"
    << "Escaped  string prints like this:\n" << stre::escape_control_chars(text) << "\n\n";

// Getting index of difference
assert(stre::index_of_difference("xxxAxx", "xxxxxx") == 3);
```

Output:
```
Original string prints like this:
 carriage returns.p due to

Escaped  string prints like this:
this text\r will get messed up due to\r carriage returns.
```