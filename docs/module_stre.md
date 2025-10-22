[<img src ="images/badge_language_cpp_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](LICENSE.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_docs.svg">](https://dmitribogdanov.github.io/UTL/)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_workflow_windows.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_workflow_ubuntu.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_workflow_macos.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)
[<img src ="images/badge_workflow_freebsd.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/freebsd.yml)

# utl::stre

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/stre.hpp)

**utl::stre** (aka **str**ing **e**xpansions) header contains implementations of most commonly used string utils.

**Motivation:** Despite the seeming triviality of the topic a lot of implementations found online are either horribly inefficient or contain straight up bugs in some edge cases. Here, the goal is to "get it right" once so no time would be spent reinventing the wheel in the future.

## Definitions

```cpp
// Character classification
constexpr bool is_digit       (char ch) noexcept;
constexpr bool is_lowercase   (char ch) noexcept;
constexpr bool is_uppercase   (char ch) noexcept;
constexpr bool is_punctuation (char ch) noexcept;
constexpr bool is_hexadecimal (char ch) noexcept;
constexpr bool is_control     (char ch) noexcept;
constexpr bool is_alphabetic  (char ch) noexcept;
constexpr bool is_alphanumeric(char ch) noexcept;
constexpr bool is_graphical   (char ch) noexcept;
constexpr bool is_printable   (char ch) noexcept;
constexpr bool is_space       (char ch) noexcept;
constexpr bool is_blank       (char ch) noexcept;

// Case conversion
constexpr char to_lower(char ch) noexcept;
constexpr char to_upper(char ch) noexcept;

std::string to_lower(std::string str);
std::string to_upper(std::string str);

// Trimming
constexpr std::string_view trim_left (std::string_view str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim_right(std::string_view str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim      (std::string_view str, char trimmed_char = ' ') noexcept;

constexpr std::string_view trim_left (const char* str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim_right(const char* str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim      (const char* str, char trimmed_char = ' ') noexcept;

std::string trim_left (std::string str, char trimmed_char = ' ');
std::string trim_right(std::string str, char trimmed_char = ' ');
std::string trim      (std::string str, char trimmed_char = ' ');

// Padding
std::string pad_left (std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad      (std::string_view str, std::size_t length, char padding_char = ' ');

// Substring checks
constexpr bool starts_with(std::string_view str, std::string_view substr) noexcept;
constexpr bool ends_with  (std::string_view str, std::string_view substr) noexcept;
constexpr bool contains   (std::string_view str, std::string_view substr) noexcept;

// Substring replacement
std::string replace_all   (std::string str, std::string_view from, std::string_view to);
std::string replace_first (std::string str, std::string_view from, std::string_view to);
std::string replace_last  (std::string str, std::string_view from, std::string_view to);
std::string replace_prefix(std::string str, std::string_view from, std::string_view to);
std::string replace_suffix(std::string str, std::string_view from, std::string_view to);

// Tokenization
std::vector<std::string> tokenize(std::string_view str, std::string_view delimiter);
std::vector<std::string> split   (std::string_view str, std::string_view delimiter);

// Repeating
std::string repeat(            char  ch, std::size_t repeats);
std::string repeat(std::string_view str, std::size_t repeats);

// Escaping
std::string escape(            char ch );
std::string escape(std::string_view str);

// Difference measurement
constexpr std::size_t first_difference(std::string_view lhs, std::string_view rhs) noexcept;
constexpr std::size_t count_difference(std::string_view lhs, std::string_view rhs) noexcept;
```

> [!Note]
> Functions that can utilize mutable input string for a more efficient implementation take `std::string` by value. This means [r-value](https://en.cppreference.com/w/cpp/language/value_category) arguments are moved & reused, while l-values are copied.

## Methods

### Character classification

> ```cpp
> constexpr bool is_digit       (char ch) noexcept;
> constexpr bool is_lowercase   (char ch) noexcept;
> constexpr bool is_uppercase   (char ch) noexcept;
> constexpr bool is_punctuation (char ch) noexcept;
> constexpr bool is_hexadecimal (char ch) noexcept;
> constexpr bool is_control     (char ch) noexcept;
> constexpr bool is_alphabetic  (char ch) noexcept;
> constexpr bool is_alphanumeric(char ch) noexcept;
> constexpr bool is_graphical   (char ch) noexcept;
> constexpr bool is_printable   (char ch) noexcept;
> constexpr bool is_space       (char ch) noexcept;
> constexpr bool is_blank       (char ch) noexcept;
> ```

A safer `constexpr` reimplementation of character classification functions from [`<cctype>`](https://en.cppreference.com/w/cpp/header/cctype.html).

Standard `<cctype>` functions don't operate on `char` parameters as would be intuitive, instead they accept `int` parameters which should be representable as `unsigned char` and invoke UB otherwise, which makes them extremely error-prone for common use cases. In addition, `<cctype>` functions return booleans as `int`, which can be evaluated to any non-zero value, thus breaking some common assumptions.

| `stre::` function   | `<cctype>` equivalent                                        | Description                                         |
| ------------------- | ------------------------------------------------------------ | --------------------------------------------------- |
| `is_digit()`        | [`std::isdigit()`](https://en.cppreference.com/w/cpp/string/byte/isdigit.html) | `true` for digits `0...9`                           |
| `is_lowercase()`    | [`std::islower()`](https://en.cppreference.com/w/cpp/string/byte/islower.html) | `true` for lowercase letters `a...z`                |
| `is_uppercase()`    | [`std::isupper()`](https://en.cppreference.com/w/cpp/string/byte/isupper.html) | `true` for uppercase letters `A...Z`                |
| `is_punctuation()`  | [`std::ispunct()`](https://en.cppreference.com/w/cpp/string/byte/ispunct.html) | `true` for punctuation characters                   |
| `is_hexadecimal()`  | [`std::isxdigit()`](https://en.cppreference.com/w/cpp/string/byte/isxdigit.html) | `true` for hex digits `0...9`, `A..F`, `a..f`       |
| `is_control()`      | [`std::iscntrl()`](https://en.cppreference.com/w/cpp/string/byte/iscntrl.html) | `true` for control characters `0x00...0x1F`, `0x7F` |
| `is_alphabetic()`   | [`std::isalpha()`](https://en.cppreference.com/w/cpp/string/byte/isalpha.html) | `true` for letters `a...z`, `A...Z`                 |
| `is_alphanumeric()` | [`std::isalnum()`](https://en.cppreference.com/w/cpp/string/byte/isalnum.html) | `true` for letters and digits                       |
| `is_graphical()`    | [`std::isgraph()`](https://en.cppreference.com/w/cpp/string/byte/isgraph.html) | `true` for letters, digits and punctuation          |
| `is_printable()`    | [`std::isprint()`](https://en.cppreference.com/w/cpp/string/byte/isprint.html) | `true` for graphical characters and spaces          |
| `is_space()`        | [`std::isspace()`](https://en.cppreference.com/w/cpp/string/byte/isspace.html) | `true` for spaces, tabs, line feeds and etc.        |
| `is_blank()`        | [`std::isblank()`](https://en.cppreference.com/w/cpp/string/byte/isblank.html) | `true` for spaces and tabs                          |

### Case conversions

```cpp
std::string to_lower(       char ch );
std::string to_lower(std::string str);
```

Replaces all uppercase letters `A...Z` in the string `str` (or character `ch`) with corresponding lowercase letters `a...z`.

```cpp
std::string to_upper(       char ch );
std::string to_upper(std::string str);
```

Replaces all lowercase letters `a...z` in the string `str` (or character `ch`) with corresponding uppercase letters `A...Z`.

### Trimming

```cpp
constexpr std::string_view trim_left (std::string_view str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim_right(std::string_view str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim      (std::string_view str, char trimmed_char = ' ') noexcept;

constexpr std::string_view trim_left (const char* str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim_right(const char* str, char trimmed_char = ' ') noexcept;
constexpr std::string_view trim      (const char* str, char trimmed_char = ' ') noexcept;

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

### Substring checks

```cpp
bool starts_with(std::string_view str, std::string_view substr);
bool ends_with  (std::string_view str, std::string_view substr);
bool contains   (std::string_view str, std::string_view substr);
```

Returns `true` if string `str` starts with / ends with / contains the substring `substr`.

### Substring replacement

```cpp
std::string replace_all   (std::string str, std::string_view from, std::string_view to);
```

Replaces **all** occurrences of substring `from` in the string `str` with `to`.

> ```cpp
> std::string replace_first (std::string str, std::string_view from, std::string_view to);
> ```

Replaces the **first** occurrence of substring `from` in the string `str` with `to`.

> ```cpp
> std::string replace_last  (std::string str, std::string_view from, std::string_view to);
> ```

Replaces the **last** occurrence of substring `from` in the string `str` with `to`.

> ```cpp
> std::string replace_prefix(std::string str, std::string_view from, std::string_view to);
> ```

If `str` **starts** with a substring `from`, replaces it with `to`.

> ```cpp
> std::string replace_suffix(std::string str, std::string_view from, std::string_view to);
> ```

If `str` **ends** with a substring `from`, replaces it with `to`.

### Tokenization

```cpp
std::vector<std::string> tokenize(std::string_view str, std::string_view delimiter);
std::vector<std::string> split   (std::string_view str, std::string_view delimiter);
```

Splits string `str` into a vector of `std::string` tokens based on `delimiter`.

In the case of leading / trailing / repeated delimiters, `tokenize()` will omit empty the resulting empty tokens, while `split()` preserves them.

### Repeating

> ```cpp
> std::string repeat_char  (            char  ch, std::size_t repeats);
> std::string repeat_string(std::string_view str, std::size_t repeats);
> ```

Repeats given character or string a given number of times and returns the resulting string.

### Escaping

```cpp
std::string escape(            char ch );
std::string escape(std::string_view str);
```

Escapes all control & non-printable characters in `ch` / `str` using standard C++ notation. See the [corresponding example](#example).

Useful for viewing string contents and serialization without special characters getting in the way.

### Difference measurement

```cpp
constexpr std::size_t first_difference(std::string_view lhs, std::string_view rhs) noexcept;
```

Returns the index of the first character that is different between strings `lhs` and `rhs`.

Differently sized strings are allowed. Should both strings be the same, returns [`std::string_view::npos`](https://en.cppreference.com/w/cpp/string/basic_string_view/npos).

> ```cpp
> constexpr std::size_t count_difference(std::string_view lhs, std::string_view rhs) noexcept;
> ```

Returns the number of characters that is different between strings `lhs` and `rhs`.

Differently sized strings are allowed.

## Examples

### Character classification

[ [Run this code]() ] [ [Open source file](../examples/module_stre/character_classification.cpp) ]

```cpp
using namespace utl;

static_assert(!stre::is_control( '5'));
static_assert( stre::is_control('\f'));
static_assert( stre::is_control('\n'));

static_assert(!stre::is_graphical(' '));
static_assert( stre::is_graphical('X'));
```

### Case conversions

[ [Run this code]() ] [ [Open source file](../examples/module_stre/case_conversions.cpp) ]

```cpp
using namespace utl;

assert(stre::to_lower("Lorem Ipsum") == "lorem ipsum");
assert(stre::to_upper("lorem ipsum") == "LOREM IPSUM");
```

### Trimming

[ [Run this code]() ] [ [Open source file](../examples/module_stre/trimming.cpp) ]

```cpp
using namespace utl;

static_assert(stre::trim_left ("   lorem ipsum   ") ==    "lorem ipsum   ");
static_assert(stre::trim_right("   lorem ipsum   ") == "   lorem ipsum"   );
static_assert(stre::trim      ("   lorem ipsum   ") ==    "lorem ipsum"   );

static_assert(stre::trim("__ASSERT__", '_') == "ASSERT");
```

### Padding

[ [Run this code]() ] [ [Open source file](../examples/module_stre/padding.cpp) ]

```cpp
using namespace utl;

assert(stre::pad_left ("value", 9) == "    value" );
assert(stre::pad_right("value", 9) == "value    " );
assert(stre::pad      ("value", 9) == "  value  " );

assert(stre::pad(" label ", 15, '-') == "---- label ----");
```

### Substring checks

[ [Run this code]() ] [ [Open source file](../examples/module_stre/substring_checks.cpp) ]

```cpp
using namespace utl;

static_assert(stre::starts_with("lorem ipsum", "lorem"));
static_assert(stre::ends_with  ("lorem ipsum", "ipsum"));
static_assert(stre::contains   ("lorem ipsum", "em ip"));
```

### Substring replacement

[ [Run this code]() ] [ [Open source file](../examples/module_stre/substring_replacement.cpp) ]

```cpp
using namespace utl;

assert(stre::replace_all  ("__xx__xx__", "xx", "yy") == "__yy__yy__");
assert(stre::replace_first("__xx__xx__", "xx", "yy") == "__yy__xx__");
assert(stre::replace_last ("__xx__xx__", "xx", "yy") == "__xx__yy__");

assert(stre::replace_prefix("__xx__xx__", "__xx", "--yy") == "--yy__xx__");
assert(stre::replace_suffix("__xx__xx__", "xx__", "yy--") == "__xx__yy--");
```

### Tokenization

[ [Run this code]() ] [ [Open source file](../examples/module_stre/tokenization.cpp) ]

```cpp
using namespace utl;

auto tokens = stre::tokenize("aaa,bbb,,ccc,", ","); // empty tokens are discarded
assert(tokens.size() == 3);
assert(tokens[0] == "aaa");
assert(tokens[1] == "bbb");
assert(tokens[2] == "ccc");

tokens = stre::split("(---)lorem(---)ipsum", "(---)"); // empty tokens are preserved
assert(tokens.size() == 3  );
assert(tokens[0] == ""     );
assert(tokens[1] == "lorem");
assert(tokens[2] == "ipsum");
```

### Repeating

[ [Run this code]() ] [ [Open source file](../examples/module_stre/repeating.cpp) ]

```cpp
using namespace utl;

assert(stre::repeat(  'h', 7) == "hhhhhhh"        );
assert(stre::repeat("xo-", 5) == "xo-xo-xo-xo-xo-");
```

### Escaping

[ [Run this code]() ] [ [Open source file](../examples/module_stre/escaping.cpp) ]

```cpp
using namespace utl;

const std::string text = "this text\r will get messed up due to\r carriage returns.";

std::cout
    << "Original string prints like this:\n" <<              text  << "\n\n"
    << "Escaped  string prints like this:\n" << stre::escape(text) << "\n\n";
```

Output:
```
Original string prints like this:
 carriage returns.p due to

Escaped  string prints like this:
this text\r will get messed up due to\r carriage returns.
```

### Difference measurement

[ [Run this code]() ] [ [Open source file](../examples/module_stre/escaping.cpp) ]

```cpp
using namespace utl;

static_assert(stre::first_difference("xxxxYx", "xxxxXx") == 4);
static_assert(stre::first_difference("xxx"   , "xxxxxx") == 3);

static_assert(stre::count_difference("xxxxYx", "xxxxXx") == 1);
static_assert(stre::count_difference("yyy"   , "xxxxxx") == 6);
```
