[<img src ="images/badge_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/badge_license_mit.svg">](../LICENSE.md)
[<img src ="images/badge_semver.svg">](guide_versioning.md)
[<img src ="images/badge_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/badge_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="images/badge_windows_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="images/badge_ubuntu_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="images/badge_macos_passing.svg">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::shell

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/shell.hpp)

**utl::shell** is a small header that contains:

- Temporary file creation with [RAII](https://en.cppreference.com/w/cpp/language/raii.html) handles
- A function to run shell commands while capturing status / stdout / stderr

It is mainly useful for invoking scripts and other executables in a portable (but not particularly secure) way.

## Definitions

```cpp
// Temporary files
struct TemporaryHandle {
    TemporaryHandle()                       = delete;
    TemporaryHandle(const TemporaryHandle&) = delete;
    TemporaryHandle(TemporaryHandle&&)      = default;
    
    // Construction
    static TemporaryHandle    create(std::filesystem::path filepath);
    static TemporaryHandle    create(                              );
    static TemporaryHandle overwrite(std::filesystem::path filepath);
    static TemporaryHandle overwrite(                              );
    
    // Getters
    std::ifstream ifstream(std::ios::openmode mode = std::ios::in ) const;
    std::ofstream ofstream(std::ios::openmode mode = std::ios::out) const;
    
    const std::filesystem::path& path() const noexcept;
    const std::string          &  str() const noexcept;
};

// Shell commands
struct CommandResult {
    int         status;
    std::string out;
    std::string err;
};

CommandResult run_command(std::string_view command);
```

## Methods

### Temporary files

> ```cpp
> TemporaryHandle()                       = delete;
> TemporaryHandle(const TemporaryHandle&) = delete;
> TemporaryHandle(TemporaryHandle&&)      = default;
> ```

`TemporaryHandle` "owns" the file lifetime and has move-only semantics. 

> ```cpp
> static TemporaryHandle    create(std::filesystem::path filepath); // (1)
> static TemporaryHandle    create(                              ); // (2)
> ```

Overload **(1)** generates temporary file with a given `filepath`.

Overload **(2)** generates file with a unique name inside the system temporary directory.

Does not overwrite existing files in case of a name collision, throws `std::runtime_error` if new file would replace an existing one.

> ```cpp
> static TemporaryHandle overwrite(std::filesystem::path filepath); // (1)
> static TemporaryHandle overwrite(                              ); // (2)
> ```

Overload **(1)** generates temporary file with a given `filepath`.

Overload **(2)** generates file with a unique name inside the system temporary directory.

Overwrites existing files in case of a name collision.

> ```cpp
> std::ifstream ifstream(std::ios::openmode mode = std::ios::in ) const;
> std::ofstream ofstream(std::ios::openmode mode = std::ios::out) const;
> ```

Returns `std::ifstream` / `std::ofstream` associated with the temporary file.

Throws `std::runtime_error` in case of an IO failure.

> ```cpp
> const std::filesystem::path& path() const noexcept;
> const std::string          &  str() const noexcept;
> ```

Returns `std::filesystem::path` / `std::string` associated with the temporary file.

### Shell commands

> ```cpp
> struct CommandResult {
>     int         status;
>     std::string out;
>     std::string err;
> };
>
> CommandResult run_command(std::string_view command);
> ```

Runs command using a default system shell (`cmd` for Windows, `bash` for Linux, `zsh` for MacOS and some Linux distros).

Return `status`, `stdout` and `stderr` (see [standard streams](https://en.wikipedia.org/wiki/Standard_streams)) piped from the shell.

**Note:** It is assumed that `command` does not redirect it's own streams. In case stream redirection is necessary, `command` can usually be wrapped in a subshell, for example in `bash` command `echo HELLO >&2` can be rewritten as`(echo HELLO >&2)` to add a subshell.

## Examples

### Working with temporary files

[ [Run this code](https://godbolt.org/z/ovKooPsEd) ]

```cpp
const auto handle = utl::shell::TemporaryHandle::overwrite("temporary.txt");

// Write to temporary file
handle.ofstream() << "TEXT";

// Read from temporary file
std::string          text;
handle.ifstream() >> text;

assert(text == "TEXT");

// Append some more text
handle.ofstream(std::ios_base::app) << "MORE TEXT";

// Temp. file is deleted once handle is destroyed
```

### Running shell commands

> [!Warning]
> Online compiler explorer does not support `std::system`, failing the `[ Run this code ]` is expected.

[ [Run this code](https://godbolt.org/z/csnMvEc67) ]

```cpp
#ifdef __linux__

const auto res = utl::shell::run_command("echo TEXT");
// usually used to invoke scripts and other executables

assert(res.status ==      0);
assert(res.out    == "TEXT");
assert(res.err    ==     "");

#endif
```