# Including only specific modules while using amalgamated header

[<- back to README.md](..)

While using [individual headers](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL) is generally more robust, the amalgamated [`UTL.hpp`](./../single_include/UTL.hpp) still allows selection of individual libraries to include (for example, to reduce compile times when only some of the modules are needed).

This is achievable by declaring `#define UTL_PICK_MODULES` and `#define <module_identifier>` for all necessary modules before including the library header, see [examples](#selecting-specific-modules).

## Module Identifiers

| Name | Identifier |
| --- | --- |
| `utl::bit` | `UTLMODULE_BIT` |
| `utl::enum_reflect` | `UTLMODULE_ENUM_REFLECT` |
| `utl::integral` | `UTLMODULE_INTEGRAL` |
| `utl::json` | `UTLMODULE_JSON` |
| `utl::log` | `UTLMODULE_LOG` |
| `utl::math` | `UTLMODULE_MATH` |
| `utl::mvl` | `UTLMODULE_MVL` |
| `utl::parallel` | `UTLMODULE_PARALLEL` |
| `utl::predef` | `UTLMODULE_PREDEF` |
| `utl::profiler` | `UTLMODULE_PROFILER` |
| `utl::progressbar` | `UTLMODULE_PROGRESSBAR` |
| `utl::random` | `UTLMODULE_RANDOM` |
| `utl::shell` | `UTLMODULE_SHELL` |
| `utl::sleep` | `UTLMODULE_SLEEP` |
| `utl::stre` | `UTLMODULE_STRE` |
| `utl::struct_reflect` | `UTLMODULE_STRUCT_REFLECT` |
| `utl::table` | `UTLMODULE_TABLE` |
| `utl::time` | `UTLMODULE_TIME` |

## Examples

### Selecting specific modules

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:10,positionColumn:1,positionLineNumber:10,selectionStartColumn:1,selectionStartLineNumber:10,startColumn:1,startLineNumber:10),source:'//+Indicate+that+you+want+to+select+specific+modules%0A%23define+UTL_PICK_MODULES%0A%0A//+List+modules+that+should+be+compiled%0A%23define+UTLMODULE_MATH%0A%23define+UTLMODULE_PROGRESSBAR%0A%0A%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A%0Aint+main()+%7B%0A++++//+%3C+your+code+here+%3E++%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:31,positionColumn:1,positionLineNumber:31,selectionStartColumn:1,selectionStartLineNumber:31,startColumn:1,startLineNumber:31),source:'//+Pick+some+modules+in+one+header%0A%23define+UTL_PICK_MODULES%0A%23define+UTLMODULE_JSON%0A%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A//+...%0A%0A//+Pick+some+more+modules+in+another+header%0A%23define+UTL_PICK_MODULES%0A%23define+UTLMODULE_RANDOM%0A%23define+UTLMODULE_LOG%0A%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0A//+...%0A%0A//+Translation+unit+that+includes+both+headers+in+any+order+will%0A//+have+access+to+utl::json,+utl::random+and+utl::log%0A%0A%0A//+This+allows+one+to+pull+only+necessary+modules+in+each+header.%0A//+Essentially,+as+long+as+UTL_PICK_MODULES+is+defined,+%22UTL.hpp%22%0A//+acts+as+a+collection+of+individual+module+headers+concatenated%0A//+into+a+single+file+and+enabled+with+%23define+UTLMODULE_%7BNAME%7D%0A//%0A//+Alternatively,+just+grab+individual+modules+directly+from+the+repo.%0A%0A%0Aint+main()+%7B%0A++++//+%3C+your+code+here+%3E%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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
