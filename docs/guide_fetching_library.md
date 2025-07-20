# Fetching the library with CMake `FetchContent()`

[<- back to README.md](..)

While header-only libraries can be trivially included without any use of the build system, some users might prefer to manage their dependencies in a unified manner through CMake. For this purpose UTL provides two `INTERFACE` targets:

```cmake
UTL::include
UTL::single_include
```

corresponding to [`include/`](https://github.com/DmitriBogdanov/UTL/tree/master/include) and [`single_include/`](https://github.com/DmitriBogdanov/UTL/tree/master/single_include) directories.

## `FetchContent()` example

CMake [`FetchContent()`](https://cmake.org/cmake/help/latest/module/FetchContent.html) can be used to automatically download and expose UTL as a library:

```cmake
# Fetch library from GitHub
include(FetchContent)

FetchContent_Declare(
    UTL
    GIT_REPOSITORY https://github.com/DmitriBogdanov/UTL.git
    GIT_TAG        v5.1.2
)

FetchContent_MakeAvailable(UTL)
```

**Note:** While `GIT_TAG` argument can be omitted to download latest commit automatically, it is not recommended due to possible breaking changes in the newer versions of the library.

## `CPM` example

The same thing can be done using [CPM](https://github.com/cpm-cmake/CPM.cmake) for dependency management:

```cmake
# Fetch library from GitHub
include(cmake/CPM.cmake)

CPMAddPackage("gh:DmitriBogdanov/UTL#v5.1.2")
```

**Note:** Use [commit SHA](https://github.com/DmitriBogdanov/UTL/commits/master/) instead of the tag to download specific commits.

## Linking the library

Fetched library can now be linked to a target:

```cmake
# Link library to the executable
add_executable(main_target main.cpp)

target_link_libraries(main_target UTL::include)
```

Which allows us to use it in `main.cpp`:

```cpp
#include "UTL/json.hpp"

int main() {
    // <code using utl::json>
}
```

## Questions and answers

**Q: Does this library set any CMake options or variables?**

**A:** No, `CMakeLists.txt` only exposes two interface targets.

**Q: What if I'm using an older commit that doesn't correspond to the current documentation?**

**A:** `FetchContent()` downloads this repo into `build/_deps/utl-src` together with corresponding documentation, which can be used offline since UTL docs are mostly written in standard markdown with relative links. Some things like badges might link to a repo, but most relevant content stays available.
