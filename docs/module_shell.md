[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

# utl::shell

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/shell.hpp)

**utl::shell** is a small header that contains:

- Temporary file creation with [RAII](https://en.cppreference.com/w/cpp/language/raii.html) handles
- A function to run shell commands while capturing status / stdout / stderr

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

### Temporary file operations

> ```cpp
> std::string shell::random_ascii_string(size_t length);
> ```

Generates a string of random ASCII characters in ['a', 'z'] range. Uses [std::rand](https://en.cppreference.com/w/cpp/numeric/random/rand) internally.

> ```cpp
> std::string shell::generate_temp_file();
> ```

Generates a temporary file with random unique name in the current folder (using relative path). Returns filename.

Internally module keeps track of all created temporary files so they can be later deleted with `clear_temp_files()`.

> ```cpp
> void clear_temp_files();
> ```

Deletes all temporary files created with `generate_temp_file()` during current runtime. **Called automatically when exiting the program if any files were created.**

**Note:** "Exiting" happens when program returns from `main()` or calls `std::exit()`.

> ```cpp
> void erase_temp_file(const std::string &file);
> ```

Deletes a single temporary file. Used to make methods that can clean up internal temporary files without affecting the global state.

### `argc`/`argv` parsing

> ```cpp
> std::string_view shell::get_exe_path(char** argv);
> ```

Parses program executable path from `argv` as `std::string_view`.

`argc != 0` is a reasonable assumption since the only way to achieve such launch is to run executable through a null `execv()`, most command-line programs assume such scenario to be either impossible or an error on user side.

> ```cpp
> std::vector<std::string_view> shell::get_command_line_args(int argc, char** argv);
> ```

Parses program command line arguments from `argv` as `std::string_view`.

### Shell operations

> ```cpp
> CommandResult shell::run_command(const std::string &command);
> ```

Runs console command using a default system shell (*cmd* for Windows, *bash* for Linux). Return *status* code, *stdout* and *stderr* (see [standard streams](https://en.wikipedia.org/wiki/Standard_streams)) piped from the process.

## Examples

### Creating temporary files

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:16,positionColumn:14,positionLineNumber:16,selectionStartColumn:14,selectionStartLineNumber:16,startColumn:14,startLineNumber:16),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Create+temp.+file+and+fill+it+with+random+text%0A++++const+auto+temp_file_path+%3D+shell::generate_temp_file()%3B%0A++++const+auto+temp_file_text+%3D+%22~~~%22+%2B+shell::random_ascii_string(20)+%2B+%22~~~%22%3B%0A++++std::ofstream(temp_file_path)+%3C%3C+temp_file_text%3B%0A%0A++++std::cout+%3C%3C+%22Temp.+file+path:+%22+%3C%3C+temp_file_path+%3C%3C+%22%5Cn%22%3B%0A++++std::cout+%3C%3C+%22Temp.+file+text:+%22+%3C%3C+temp_file_text+%3C%3C+%22%5Cn%22%3B%0A%0A++++shell::clear_temp_files()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

// Create temp. file and fill it with random text
const auto temp_file_path = shell::generate_temp_file();
const auto temp_file_text = "~~~" + shell::random_ascii_string(20) + "~~~";
std::ofstream(temp_file_path) << temp_file_text;

std::cout << "Temp. file path: " << temp_file_path << "\n";
std::cout << "Temp. file text: " << temp_file_text << "\n";

// We can clear files manually, but otherwise they will be automatically cleared upon exit
// ('exit' is triggered by calling std::exit() or returning from main())
// shell::clear_temp_files();
```

Output:
```
Temp. file path: gebknywycgeaospotmfjsskpcflasr.txt
Temp. file text: ~~~fjocozcfeyajpjrekjml~~~
```

### Getting .exe path and args

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:12,positionColumn:14,positionLineNumber:12,selectionStartColumn:14,selectionStartLineNumber:12,startColumn:14,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout+%3C%3C+%22Exe+path:%5Cn%22+%3C%3C+shell::get_exe_path(argv)+%3C%3C+%22%5Cn%5Cn%22%3B%0A%0A++++std::cout+%3C%3C+%22Command+line+arguments+(if+present):%5Cn%22%3B%0A++++for+(const+auto+%26arg+:+shell::get_command_line_args(argc,+argv))%0A++++++++std::cout+%3C%3C+arg+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

std::cout << "Exe path:\n" << shell::get_exe_path(argv) << "\n\n";

std::cout << "Command line arguments (if present):\n";
for (const auto &arg : shell::get_command_line_args(argc, argv))
    std::cout << arg << "\n";
```

Output:
```
Exe path:
C:\GENERAL_FOLDER\Files\C++\proto_utils\Release\proto_utils.exe

Command line arguments (if present):

```

### Running shell commands

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:50,endLineNumber:12,positionColumn:50,positionLineNumber:12,selectionStartColumn:50,selectionStartLineNumber:12,startColumn:50,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Create+temp.+file+and+fill+it+with+text%0A++++const+auto+temp_file_path+%3D+shell::generate_temp_file()%3B%0A++++const+auto+temp_file_text+%3D+%22~~~FILE+CONTENTS~~~%22%3B%0A++++std::ofstream(temp_file_path,+std::ios_base::app)+%3C%3C+temp_file_text%3B%0A%0A++++//+Run+command+to+show+file+contents+(does+not+work+in+godbolt)%0A++++const+auto+command+%3D+%22cat+%22+%2B+temp_file_path%3B%0A++++const+auto+command_result+%3D+shell::run_command(command)%3B%0A%0A++++std::cout%0A++++++++%3C%3C+%22shell::run_command(%22+%3C%3C+command+%3C%3C+%22):%5Cn%22%0A++++++++%3C%3C+%22command_result.status+%3D+%22+%3C%3C+command_result.status+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22command_result.stdout_output+%3D+%22+%3C%3C+command_result.stdout_output+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22command_result.stderr_output+%3D+%22+%3C%3C+command_result.stderr_output+%3C%3C+%22%5Cn%22%3B%0A%0A++++shell::clear_temp_files()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'1',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ] (`std::system` might not work with online compiler explorer)
```cpp
using namespace utl;

// Create temp. file and fill it with text
const auto temp_file_path = shell::generate_temp_file();
const auto temp_file_text = "~~~FILE CONTENTS~~~";
std::ofstream(temp_file_path, std::ios_base::app) << temp_file_text;

// Run command to show file contents (bash command in this example)
const auto command = "cat " + temp_file_path;
const auto command_result = shell::run_command(command);

std::cout
    << "shell::run_command(" << command << "):\n"
    << "command_result.status = " << command_result.status << "\n"
    << "command_result.stdout_output = " << command_result.stdout_output << "\n"
    << "command_result.stderr_output = " << command_result.stderr_output << "\n";

shell::clear_temp_files();
```

Output:
```
shell::run_command(type lvswemcevuvhoysbbvjqokhhbpudfx.txt):
command_result.status = 0
command_result.stdout_output = ~~~FILE CONTENTS~~~
command_result.stderr_output =
```
