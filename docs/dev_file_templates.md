# File templates

This file contains templates to ensure a standardized style of source files, documentation and etc.

## Module template

```cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::xxxxxxxxxxxx
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_xxxxxxxxxxxx.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTL_MODULE_XXXXXXXXXXXX)

#ifndef utl_xxxxxxxxxxxx_headerguard
#define utl_xxxxxxxxxxxx_headerguard

#define UTL_XXXXXXXXXXXX_VERSION_MAJOR 1
#define UTL_XXXXXXXXXXXX_VERSION_MINOR 0
#define UTL_XXXXXXXXXXXX_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

namespace utl::xxxxxxxxxxxx::impl {

// NOTE: IMPL

} // namespace utl::xxxxxxxxxxxx

// ______________________ PUBLIC API ______________________

namespace utl::xxxxxxxxxxxx {

// NOTE: API

} // namespace utl::xxxxxxxxxxxx

#endif
#endif // module utl::xxxxxxxxxxxx

```

Replace `XXXXXXXXXXXX` with **module name**.

## Test template

```cpp
#include "tests/common.hpp"

#include "include/UTL/XXXXXXXXXXXX.hpp"

// _______________________ INCLUDES _______________________

// None

// ____________________ IMPLEMENTATION ____________________

TEST_CASE("TEST_NAME") {
    // TODO:
}
```

Replace `XXXXXXXXXXXX` with **module name**.

**Dependencies on other modules:**

Not allowed.

**Test cases and naming:**

File can contain multiple test cases with names `<case> / <subcase>` or one case with name `<case>`. Here `<case>` is a a copy of filename, but capitalized and separated with spaces rather than `_` for better readability.

**Reason for not using `SUBCASE()`:**

**1.** Less indentation

**2.** There is not `SUBCASE_TEMPLATE()` to replace `TEST_CASE_TEMPLATE()`, which is frequently needed to test templates

**Difference between case and subcase:**

Subcases should be used when there are multiple test cases relying on the same boilerplate and separating them into different files would cause code duplication. They can also be used for test cases that are too small & granular to warrant a separate file.

## Benchmark template

```cpp
#include "benchmarks/common.hpp"

#include "include/UTL/xxxxxxxxxxxx.hpp"

// _______________________ INCLUDES _______________________

// UTL dependencies
// None

// Libraries to benchmarks against
// None

// Standard headers
// None

// ____________________ IMPLEMENTATION ____________________

// =================
// --- Benchmark ---
// =================

void benchmark_xxx() {
    // TODO:
}

// ========================
// --- Benchmark runner ---
// ========================

int main() {
    benchmark_xxx();
}
```

Replace `XXXXXXXXXXXX` with **module name**.

**Dependencies on other modules:**

Discouraged, but allowed.

## Example template

```cpp
#include "include/UTL/XXXXXXXXXXXX.hpp"

int main() {
    // TODO:
}
```

Replace `XXXXXXXXXXXX` with **module name**.

**Dependencies on other modules:**

Allowed if it helps an example, generally should be avoided.

**Godbolt links:**

Online examples should copy the local ones, but with a different include format:

```
#include <https://raw.githubusercontent.com/DmitriBogdanov/UTL/master/include/UTL/XXXXXXXXXXXX.hpp>
```

Docs might contain an even shorter form without `main()`.

Links should be shortened due to some Markdown editor struggling with full links, shortened Godbolt links never expire.

Compiler should be: `x86-64 gcc (trunk)`

Flags should be: `-std=c++17`

# In-code headers

## Header 1

```cpp

// ================
// --- Header 1 ---
// ================

```

## Header 2

```cpp

// --- Header 2 ---
// ----------------

```

## Header 3

```cpp

// - Header 3 -
```