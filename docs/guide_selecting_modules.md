# Including only specific modules while using amalgamated header

[<- back to README.md](..)

While using [individual headers](../include/UTL) is generally more robust, the amalgamated [`UTL.hpp`](../single_include/UTL.hpp) still allows selection of individual libraries to include (for example, to reduce compile times when only some of the modules are needed).

This is achievable by declaring `#define UTL_PICK_MODULES` and `#define <module_identifier>` for all necessary modules before including the library header, see [examples](#selecting-specific-modules).

## Module Identifiers

| Name                  | Identifier                  |
| --------------------- | --------------------------- |
| `utl::bit`            | `UTL_MODULE_BIT`            |
| `utl::enum_reflect`   | `UTL_MODULE_ENUM_REFLECT`   |
| `utl::integral`       | `UTL_MODULE_INTEGRAL`       |
| `utl::json`           | `UTL_MODULE_JSON`           |
| `utl::log`            | `UTL_MODULE_LOG`            |
| `utl::math`           | `UTL_MODULE_MATH`           |
| `utl::mvl`            | `UTL_MODULE_MVL`            |
| `utl::parallel`       | `UTL_MODULE_PARALLEL`       |
| `utl::predef`         | `UTL_MODULE_PREDEF`         |
| `utl::profiler`       | `UTL_MODULE_PROFILER`       |
| `utl::progressbar`    | `UTL_MODULE_PROGRESSBAR`    |
| `utl::random`         | `UTL_MODULE_RANDOM`         |
| `utl::shell`          | `UTL_MODULE_SHELL`          |
| `utl::sleep`          | `UTL_MODULE_SLEEP`          |
| `utl::stre`           | `UTL_MODULE_STRE`           |
| `utl::struct_reflect` | `UTL_MODULE_STRUCT_REFLECT` |
| `utl::table`          | `UTL_MODULE_TABLE`          |
| `utl::time`           | `UTL_MODULE_TIME`           |

## Examples

### Selecting specific modules

[ [Run this code](https://godbolt.org/z/v9nzddEvv) ]
```cpp
// Indicate that you want to select specific modules
#define UTL_PICK_MODULES

// List modules that should be compiled
#define UTL_MODULE_MATH
#define UTL_MODULE_PROGRESSBAR

#include "UTL.hpp"


int main() {
    // < your code here >
}
```

### Selecting modules in different locations

[ [Run this code](https://godbolt.org/z/zdKW3fhqx) ]
```cpp
// Pick some modules in one header
#define UTL_PICK_MODULES
#define UTL_MODULE_JSON
#include "UTL.hpp"

// ...

// Pick some more modules in another header
#define UTL_PICK_MODULES
#define UTL_MODULE_RANDOM
#define UTL_MODULE_LOG
#include "UTL.hpp"

// ...

// Translation unit that includes both headers in any order will
// have access to utl::json, utl::random and utl::log


// This allows one to pull only necessary modules in each header.
// Essentially, as long as UTL_PICK_MODULES is defined, "UTL.hpp"
// acts as a collection of individual module headers concatenated
// into a single file and enabled with #define UTL_MODULE_{NAME}
//
// Alternatively, just grab individual modules directly from the repo.


int main() {
    // < your code here >
}
```
