[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_semver.svg">](./docs/guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::shell

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/shell.hpp)

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

CommandResult run_command(const std::string& command);
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
> CommandResult run_command(const std::string &command);
> ```

Runs command using a default system shell (`cmd` for Windows, `bash` for Linux).

Return `status`, `stdout` and `stderr` (see [standard streams](https://en.wikipedia.org/wiki/Standard_streams)) piped from the shell.

**Note:** It is assumed that `command` does not redirect it's own streams. In case stream redirection is necessary, `command` can usually be wrapped in a subshell, for example in `bash` command `echo HELLO >&2` can be rewritten as`(echo HELLO >&2)` to add a subshell.

## Examples

### Working with temporary files

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:29,endLineNumber:15,positionColumn:29,positionLineNumber:15,selectionStartColumn:29,selectionStartLineNumber:15,startColumn:29,startLineNumber:15),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++const+auto+handle+%3D+utl::shell::TemporaryHandle::overwrite(%22temporary.txt%22)%3B%0A%0A++++//+Write+to+temporary+file%0A++++handle.ofstream()+%3C%3C+%22TEXT%22%3B%0A%0A++++//+Read+from+temporary+file%0A++++std::string++++++++++text%3B%0A++++handle.ifstream()+%3E%3E+text%3B%0A%0A++++assert(text+%3D%3D+%22TEXT%22)%3B%0A%0A++++//+Append+some+more+text%0A++++handle.ofstream(std::ios_base::app)+%3C%3C+%22MORE+TEXT%22%3B%0A%0A++++//+Temp.+file+is+deleted+once+handle+is+destroyed%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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
> Online compiler explorer does not work with `std::system`, failing the command is expected.

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:34,endLineNumber:8,positionColumn:34,positionLineNumber:8,selectionStartColumn:34,selectionStartLineNumber:8,startColumn:34,startLineNumber:8),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++const+auto+res+%3D+utl::shell::run_command(%22echo+TEXT%22)%3B%0A++++//+usually+used+to+invoke+scripts+and+other+executables%0A%0A++++assert(res.status+%3D%3D++++++0)%3B%0A++++assert(res.out++++%3D%3D+%22TEXT%22)%3B%0A++++assert(res.err++++%3D%3D+++++%22%22)%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
const auto res = utl::shell::run_command("echo TEXT");
// usually used to invoke scripts and other executables

assert(res.status ==      0);
assert(res.out    == "TEXT");
assert(res.err    ==     "");
```