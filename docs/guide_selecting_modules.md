# Including only specific modules while using amalgamated header

[<- back to README.md](..)

While using [individual headers](../include/UTL) is generally more robust, the amalgamated [`UTL.hpp`](../single_include/UTL.hpp) still allows selection of individual libraries to include (for example, to reduce compile times when only some of the modules are needed).

This is achievable by declaring `#define UTL_PICK_MODULES` and `#define <module_identifier>` for all necessary modules before including the library header, see [examples](#selecting-specific-modules).

## Module Identifiers

| Name                  | Identifier                 |
| --------------------- | -------------------------- |
| `utl::bit`            | `UTLMODULE_BIT`            |
| `utl::enum_reflect`   | `UTLMODULE_ENUM_REFLECT`   |
| `utl::integral`       | `UTLMODULE_INTEGRAL`       |
| `utl::json`           | `UTLMODULE_JSON`           |
| `utl::log`            | `UTLMODULE_LOG`            |
| `utl::math`           | `UTLMODULE_MATH`           |
| `utl::mvl`            | `UTLMODULE_MVL`            |
| `utl::parallel`       | `UTLMODULE_PARALLEL`       |
| `utl::predef`         | `UTLMODULE_PREDEF`         |
| `utl::profiler`       | `UTLMODULE_PROFILER`       |
| `utl::progressbar`    | `UTLMODULE_PROGRESSBAR`    |
| `utl::random`         | `UTLMODULE_RANDOM`         |
| `utl::shell`          | `UTLMODULE_SHELL`          |
| `utl::sleep`          | `UTLMODULE_SLEEP`          |
| `utl::stre`           | `UTLMODULE_STRE`           |
| `utl::struct_reflect` | `UTLMODULE_STRUCT_REFLECT` |
| `utl::table`          | `UTLMODULE_TABLE`          |
| `utl::time`           | `UTLMODULE_TIME`           |

## Examples

### Selecting specific modules

[ [Run this code](https://godbolt.org/z/T1qEd4Yno) ]
```cpp
// Indicate that you want to select specific modules
#define UTL_PICK_MODULES

// List modules that should be compiled
#define UTLMODULE_MATH
#define UTLMODULE_PROGRESSBAR

#include "UTL.hpp"


int main() {
    // < your code here >
}
```

### Selecting modules in different locations

[ [Run this code](https://godbolt.org/z/fbsqssvdq) ]
```cpp
// Pick some modules in one header
#define UTL_PICK_MODULES
#define UTLMODULE_JSON
#include "UTL.hpp"

// ...

// Pick some more modules in another header
#define UTL_PICK_MODULES
#define UTLMODULE_RANDOM
#define UTLMODULE_LOG
#include "UTL.hpp"

// ...

// Translation unit that includes both headers in any order will
// have access to utl::json, utl::random and utl::log


// This allows one to pull only necessary modules in each header.
// Essentially, as long as UTL_PICK_MODULES is defined, "UTL.hpp"
// acts as a collection of individual module headers concatenated
// into a single file and enabled with #define UTLMODULE_{NAME}
//
// Alternatively, just grab individual modules directly from the repo.


int main() {
    // < your code here >
}
```
