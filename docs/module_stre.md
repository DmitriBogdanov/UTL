[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_semver.svg">](./docs/guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::stre

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/stre.hpp)

**utl::stre** (aka **str**ing **e**xpansions) header contains implementations of most commonly used string utils.

**Motivation:** Despite the seeming triviality of the topic a lot of implementations found online are either horribly inefficient or contain straight up bugs in some edge cases. Here, the goal is to "get it right" so no time would be spent reinventing the wheel in the future.

## Definitions

```cpp
// Trimming
template <class T> std::string trim_left( T&& str, char trimmed_char = ' ');
template <class T> std::string trim_right(T&& str, char trimmed_char = ' ');
template <class T> std::string trim(      T&& str, char trimmed_char = ' ');

// Padding
std::string pad_left( std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad(      std::string_view str, std::size_t length, char padding_char = ' ');

std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 10);

// Case conversions
template <class T> std::string to_lower(T&& str);
template <class T> std::string to_upper(T&& str);

// Substring checks
bool starts_with(std::string_view str, std::string_view substr);
bool ends_with(  std::string_view str, std::string_view substr);
bool contains(   std::string_view str, std::string_view substr);

// Token manipulation
template<class T> std::string replace_all_occurrences(T&& str, std::string_view from, std::string_view to);

std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter, bool keep_empty_tokens = false);

// Other utils
std::string repeat_char(              char  ch, std::size_t repeats);
std::string repeat_string(std::string_view str, std::size_t repeats);

std::string escape_control_chars(std::string_view str);

std::size_t index_of_difference(std::string_view str_1, std::string_view str_2);
```

> [!Note]
> Functions that can utilize mutable input string for a more efficient implementation are declared with `template <class T>` and use [perfect forwarding](https://en.cppreference.com/w/cpp/utility/forward). This means whenever [r-value](https://en.cppreference.com/w/cpp/language/value_category) arguments are provided they automatically get reused, while l-values are copied.

## Methods

### Trimming

```cpp
template <class T> std::string trim_left( T&& str, char trimmed_char = ' ');
template <class T> std::string trim_right(T&& str, char trimmed_char = ' ');
template <class T> std::string trim(      T&& str, char trimmed_char = ' ');
```

Trims characters equal to `trimmed_char` from the left / right / both sides of the string `str`.

### Padding

```cpp
std::string pad_left( std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad_right(std::string_view str, std::size_t length, char padding_char = ' ');
std::string pad(      std::string_view str, std::size_t length, char padding_char = ' ');
```

Pads string `str` with characters `padding_char` from left / right / both sides until it reaches size `length`.

**Note:** If `str.size >= length` the string is left unchanged.

> ```cpp
> std::string pad_with_leading_zeroes(unsigned int number, std::size_t length = 10);
> ```

Pads given integer with leading zeroes until its length reaches `length`. Useful for numbering files/data entries so they can be lexicographically sorted.

**Note:** If `number` has more than `length` digits, resulting string is the same as `std::to_string(number)`.

### Case conversions

```cpp
template <class T> std::string to_lower(T&& str);
```

Replaces all uppercase letters `ABCDEFGHIJKLMNOPQRSTUVWXYZ` in the string `str` with corresponding lowercase letters `abcdefghijklmnopqrstuvwxyz`.

```cpp
template <class T> std::string to_upper(T&& str);
```

Replaces all lowercase letters `abcdefghijklmnopqrstuvwxyz` in the string `str` with corresponding uppercase letters `ABCDEFGHIJKLMNOPQRSTUVWXYZ`.

### Substring checks

```cpp
bool starts_with(std::string_view str, std::string_view substr);
bool ends_with(  std::string_view str, std::string_view substr);
bool contains(   std::string_view str, std::string_view substr);
```

Returns `true` if string `str` starts with / ends with / contains the substring `substr`.

### Token manipulation

```cpp
template<class T> std::string replace_all_occurrences(T&& str, std::string_view from, std::string_view to);
```

Scans through the string `str` and replaces all occurrences of substring `from` with a string `to`.

```cpp
std::vector<std::string> split_by_delimiter(std::string_view str, std::string_view delimiter, bool keep_empty_tokens = false);
```

Splits string `str` into a vector of `std::string` tokens based on `delimiter`.

By default `keep_empty_tokens` is `false` and `""` is not considered to be a valid token — in case of leading / trailing / repeated delimiters, only non-empty tokens are going to be inserted into the resulting vector. Setting `keep_empty_tokens` to `true` overrides this behavior and keeps all the empty tokens intact.

### Other utils

> ```cpp
> std::string repeat_char(              char  ch, std::size_t repeats);
> std::string repeat_string(std::string_view str, std::size_t repeats);
> ```

Repeats given character or string a given number of times and returns as a string.

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:10,positionColumn:1,positionLineNumber:10,selectionStartColumn:1,selectionStartLineNumber:10,startColumn:1,startLineNumber:10),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++using+namespace+utl%3B%0A%0A++++assert(stre::trim_left(+%22+++lorem+ipsum+++%22)+%3D%3D++++%22lorem+ipsum+++%22)%3B%0A++++assert(stre::trim_right(%22+++lorem+ipsum+++%22)+%3D%3D+%22+++lorem+ipsum%22+++)%3B%0A++++assert(stre::trim(++++++%22+++lorem+ipsum+++%22)+%3D%3D++++%22lorem+ipsum%22+++)%3B%0A%0A++++assert(stre::trim(%22__ASSERT_MACRO__%22,+!'_!')+%3D%3D+%22ASSERT_MACRO%22)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

assert(stre::trim_left( "   lorem ipsum   ") ==    "lorem ipsum   ");
assert(stre::trim_right("   lorem ipsum   ") == "   lorem ipsum"   );
assert(stre::trim(      "   lorem ipsum   ") ==    "lorem ipsum"   );

assert(stre::trim("__ASSERT_MACRO__", '_') == "ASSERT_MACRO");
```

### Padding strings

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:56,endLineNumber:9,positionColumn:56,positionLineNumber:9,selectionStartColumn:56,selectionStartLineNumber:9,startColumn:56,startLineNumber:9),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++using+namespace+utl%3B%0A++++%0A++++assert(stre::pad_left(+%22value%22,+9)+%3D%3D+%22++++value%22)%3B%0A++++assert(stre::pad_right(%22value%22,+9)+%3D%3D+%22value++++%22)%3B%0A++++assert(stre::pad(++++++%22value%22,+9)+%3D%3D+%22++value++%22)%3B%0A++++%0A++++assert(stre::pad(%22+label+%22,+15,+!'-!')+%3D%3D+%22----+label+----%22)%3B%0A%0A++++assert(stre::pad_with_leading_zeroes(17)+%3D%3D+%220000000017%22)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

assert(stre::pad_left( "value", 9) == "    value");
assert(stre::pad_right("value", 9) == "value    ");
assert(stre::pad(      "value", 9) == "  value  ");

assert(stre::pad(" label ", 15, '-') == "---- label ----");

assert(stre::pad_with_leading_zeroes(17) == "0000000017");
```

### Converting string case

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:60,endLineNumber:8,positionColumn:60,positionLineNumber:8,selectionStartColumn:60,selectionStartLineNumber:8,startColumn:60,startLineNumber:8),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++using+namespace+utl%3B%0A++++%0A++++assert(stre::to_lower(%22Lorem+Ipsum%22)+%3D%3D+%22lorem+ipsum%22)%3B%0A++++assert(stre::to_upper(%22lorem+ipsum%22)+%3D%3D+%22LOREM+IPSUM%22)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

assert(stre::to_lower("Lorem Ipsum") == "lorem ipsum");
assert(stre::to_upper("lorem ipsum") == "LOREM IPSUM");
```

### Using substring checks

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:55,endLineNumber:9,positionColumn:1,positionLineNumber:5,selectionStartColumn:55,selectionStartLineNumber:9,startColumn:1,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++using+namespace+utl%3B%0A++++%0A++++assert(stre::starts_with(%22lorem+ipsum%22,+%22lorem%22))%3B%0A++++assert(stre::ends_with(++%22lorem+ipsum%22,+%22ipsum%22))%3B%0A++++assert(stre::contains(+++%22lorem+ipsum%22,+%22em+ip%22))%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

assert(stre::starts_with("lorem ipsum", "lorem"));
assert(stre::ends_with(  "lorem ipsum", "ipsum"));
assert(stre::contains(   "lorem ipsum", "em ip"));
```

### Performing token manipulations

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:34,endLineNumber:22,positionColumn:34,positionLineNumber:22,selectionStartColumn:34,selectionStartLineNumber:22,startColumn:34,startLineNumber:22),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++using+namespace+utl%3B%0A++++%0A++++//+Replacting+tokens%0A++++assert(stre::replace_all_occurences(%22xxxAAxxxAAxxx%22,++%22AA%22,++%22BBB%22)+%3D%3D+%22xxxBBBxxxBBBxxx%22+)%3B%0A++++%0A++++//+Splitting+by+delimer%0A++++auto+tokens+%3D+stre::split_by_delimiter(%22aaa,bbb,ccc,%22,+%22,%22)%3B%0A++++assert(tokens.size()+%3D%3D+3)%3B%0A++++assert(tokens%5B0%5D+%3D%3D+%22aaa%22)%3B%0A++++assert(tokens%5B1%5D+%3D%3D+%22bbb%22)%3B%0A++++assert(tokens%5B2%5D+%3D%3D+%22ccc%22)%3B%0A++++%0A++++//+Splitting+by+complex+delimer+while+keeping+the+empty+tokens%0A++++tokens+%3D+stre::split_by_delimiter(%22(---)lorem(---)ipsum%22,+%22(---)%22,+true)%3B%0A++++assert(tokens.size()+%3D%3D+3)%3B%0A++++assert(tokens%5B0%5D+%3D%3D+%22%22)%3B%0A++++assert(tokens%5B1%5D+%3D%3D+%22lorem%22)%3B%0A++++assert(tokens%5B2%5D+%3D%3D+%22ipsum%22)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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
assert(tokens[0] == "");
assert(tokens[1] == "lorem");
assert(tokens[2] == "ipsum");
```

### Using other utilities

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:6,positionColumn:1,positionLineNumber:6,selectionStartColumn:1,selectionStartLineNumber:6,startColumn:1,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++using+namespace+utl%3B%0A%0A++++//+Repeating+chars/strings%0A++++assert(stre::repeat_char(++++!'h!',+7)+%3D%3D+%22hhhhhhh%22++++++++)%3B%0A++++assert(stre::repeat_string(%22xo-%22,+5)+%3D%3D+%22xo-xo-xo-xo-xo-%22)%3B%0A%0A++++//+Escaping+control+chars+in+a+string+++%0A++++const+std::string+text+%3D+%22this+text%5Cr+will+get+messed+up+due+to%5Cr+carriage+returns.%22%3B%0A++++std::cout%0A++++++++%3C%3C+%22Original+string+prints+like+this:%5Cn%22+%3C%3C++++++++++++++++++++++++++++text++%3C%3C+%22%5Cn%5Cn%22%0A++++++++%3C%3C+%22Escaped++string+prints+like+this:%5Cn%22+%3C%3C+stre::escape_control_chars(text)+%3C%3C+%22%5Cn%5Cn%22%3B%0A%0A++++//+Getting+index+of+difference%0A++++assert(stre::index_of_difference(%22xxxAxx%22,+%22xxxxxx%22)+%3D%3D+3)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

// Repeating chars/strings
assert(stre::repeat_char(    'h', 7) == "hhhhhhh"        );
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