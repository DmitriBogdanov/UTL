# File templates

This file contains templates to ensure a standardized style of source files, documentation and etc.

## Module template

```cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::XXXXXXXXXXXX
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_XXXXXXXXXXXX.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_XXXXXXXXXXXX)
#ifndef UTLHEADERGUARD_XXXXXXXXXXXX
#define UTLHEADERGUARD_XXXXXXXXXXXX

#define UTL_XXXXXXXXXXXX_VERSION_MAJOR 1
#define UTL_XXXXXXXXXXXX_VERSION_MINOR 0
#define UTL_XXXXXXXXXXXX_VERSION_PATCH 0

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

namespace utl::XXXXXXXXXXXX::impl {

// NOTE: IMPL

} // namespace utl::XXXXXXXXXXXX

// ______________________ PUBLIC API ______________________

namespace utl::XXXXXXXXXXXX {

// NOTE: API

} // namespace utl::XXXXXXXXXXXX

#endif
#endif // module utl::XXXXXXXXXXXX

```

Replace `XXXXXXXXXXXX` with **module name**.

## Module tests template

```cpp
// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "module_XXXXXXXXXXXX.hpp"

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// NOTE: IMPL

```

Replace `XXXXXXXXXXXX` with **module name**.

## In-code headers

### Header 1

```cpp

// ================
// --- Header 1 ---
// ================

```

### Header 2

```cpp

// --- Header 2 ---
// ----------------

```

### Header 3

```cpp

// - Header 3 -
```