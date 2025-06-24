[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_semver.svg">](./docs/guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::table

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/table.hpp)

**utl::table** implements LaTeX-like table drawing methods.

Useful in benchmarks and prototyping of algorithms to represent results as a table.

## Definitions

```cpp
using uint = std::streamsize;

// Table setup
void create(std::initializer_list<uint> &&widths);

// (optional)
void set_formats(std::initializer_list<ColumnFormat> &&formats);
void set_ostream(std::ostream &new_ostream);
void set_latex_mode(bool toggle);

// Drawing
template <class T, class... Types>
void cell(const T& value, const Types&... other_values); // table cells

void hline(); // horizontal line

// Format flags
ColumnFormat NONE;                          // default
ColumnFormat FIXED(     uint decimals = 3); // fixed      floats with given precision
ColumnFormat DEFAULT(   uint decimals = 6); // default    floats with given precision
ColumnFormat SCIENTIFIC(uint decimals = 3); // scientific floats with given precision
ColumnFormat BOOL;                          // bools as text
```

## Methods

### Table setup

> ```cpp
> void create(std::initializer_list<uint> &&widths);
> ```

Sets up table with given column widths. Similar to LaTeX `|c{1cm}|c{1cm}|c{1cm}|` syntax.

> ```cpp
> void set_formats(std::initializer_list<ColumnFormat> &&formats);
> ```

Sets up column [std::ios](https://en.cppreference.com/w/cpp/io/ios_base/flags) flags. Mainly used with build-in `table::` flags to change float formatting.

> ```cpp
> void set_ostream(std::ostream &new_ostream);
> ```

Redirects output to given `std::ostream`. By default `std::cout` is used.

> ```cpp
> void set_latex_mode(bool toggle);
> ```

Enables/disables [LaTeX](https://en.wikipedia.org/wiki/LaTeX)-compatible formatting.

Tables rendered with this option on will use LaTeX formatting and automatically wrap numbers in formula blocks.

This is useful for exporting tables that can be copy-pasted into a LaTeX document.

### Drawing

> ```cpp
> template <class T, class... Types>
> void cell(const T& value, const Types&... other_values);
> ```

Draws cells with given values, accepts any number of arguments and can be used to draw entire rows in a single line (see [examples](#drawing-a-table)). Similar to LaTeX `val1 & val2 & val3 \\` except line breaks are placed automatically based on the table width.

> ```cpp
> void hline();
> ```

Draws a horizontal line. Similar to LaTeX `\hline`.

### Format flags

> ```cpp
> ColumnFormat NONE;
> ColumnFormat FIXED(     uint decimals = 3);
> ColumnFormat DEFAULT(   uint decimals = 6);
> ColumnFormat SCIENTIFIC(uint decimals = 3);
> ColumnFormat BOOL;
> ```

Predefined format flags. `NONE` sets no flags. `FIXED(n)`, `DEFAULT(n)` and `SCIENTIFIC(n)` set corresponding float representations and precision. `BOOL` makes booleans render as `true` & `false`.

## Examples 

### Drawing a table

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:20,endLineNumber:11,positionColumn:20,positionLineNumber:11,selectionStartColumn:20,selectionStartLineNumber:11,startColumn:20,startLineNumber:11),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++table::create(%7B+16,+16,+16,+16,+20+%7D)%3B%0A++++table::set_formats(%7B+table::NONE,+table::DEFAULT(),+table::FIXED(2),table::SCIENTIFIC(3),+table::BOOL+%7D)%3B%0A%0A++++table::hline()%3B%0A++++table::cell(%22Method%22,+%22Threads%22,+%22Speedup%22,+%22Error%22,+%22Err.+within+range%22)%3B%0A++++table::hline()%3B%0A++++table::cell(%22Gauss%22,++++++16,+11.845236,+1.96e-4,+false)%3B%0A++++table::cell(%22Jacobi%22,+++++16,+15.512512,+1.37e-5,+false)%3B%0A++++table::cell(%22Seidel%22,+++++16,+13.412321,+1.74e-6,+true+)%3B%0A++++table::cell(%22Relaxation%22,+16,+13.926783,+1.17e-6,+true+)%3B%0A++++table::hline()%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

table::create({ 16, 16, 16, 16, 20 });
table::set_formats({ table::NONE, table::DEFAULT(), table::FIXED(2),table::SCIENTIFIC(3), table::BOOL });

table::hline();
table::cell("Method", "Threads", "Speedup", "Error", "Err. within range");
table::hline();
table::cell("Gauss",      16, 11.845236, 1.96e-4, false);
table::cell("Jacobi",     16, 15.512512, 1.37e-5, false);
table::cell("Seidel",     16, 13.412321, 1.74e-6, true );
table::cell("Relaxation", 16, 13.926783, 1.17e-6, true );
table::hline();
```

Output:
```
|----------------|----------------|----------------|----------------|--------------------|
|          Method|         Threads|         Speedup|           Error|   Err. within range|
|----------------|----------------|----------------|----------------|--------------------|
|           Gauss|              16|           11.85|       1.960e-04|               false|
|          Jacobi|              16|           15.51|       1.370e-05|               false|
|          Seidel|              16|           13.41|       1.740e-06|                true|
|      Relaxation|              16|           13.93|       1.170e-06|                true|
|----------------|----------------|----------------|----------------|--------------------|
```

### LaTeX-compatible table

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:20,endLineNumber:12,positionColumn:20,positionLineNumber:12,selectionStartColumn:20,selectionStartLineNumber:12,startColumn:20,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++table::create(%7B+10,+8,+8,+20,+18+%7D)%3B%0A++++table::set_formats(%7B+table::NONE,+table::DEFAULT(),+table::FIXED(2),table::SCIENTIFIC(3),+table::BOOL+%7D)%3B%0A%0A++++table::set_latex_mode(true)%3B%0A++++//+%3C-+adding+this+line+makes+table+render+in+LaTeX+format%0A%0A++++table::hline()%3B%0A++++table::cell(%22Method%22,+%22Threads%22,+%22Speedup%22,+%22Error%22,+%22Err.+within+range%22)%3B%0A++++table::hline()%3B%0A++++table::cell(%22Gauss%22,++++++16,+11.845236,+1.96e-4,+false)%3B%0A++++table::cell(%22Jacobi%22,+++++16,+15.512512,+1.37e-5,+false)%3B%0A++++table::cell(%22Seidel%22,+++++16,+13.412321,+1.74e-6,+true+)%3B%0A++++table::cell(%22Relaxation%22,+16,+13.926783,+1.17e-6,+true+)%3B%0A++++table::hline()%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

table::create({ 10, 8, 8, 20, 18 });
table::set_formats({ table::NONE, table::DEFAULT(), table::FIXED(2),table::SCIENTIFIC(3), table::BOOL });

table::set_latex_mode(true);
// <- adding this line makes table render in LaTeX format

table::hline();
table::cell("Method", "Threads", "Speedup", "Error", "Err. within range");
table::hline();
table::cell("Gauss",      16, 11.845236, 1.96e-4, false);
table::cell("Jacobi",     16, 15.512512, 1.37e-5, false);
table::cell("Seidel",     16, 13.412321, 1.74e-6, true );
table::cell("Relaxation", 16, 13.926783, 1.17e-6, true );
table::hline();
```

Output:
```
\hline
    Method &  Threads &  Speedup &                Error &  Err. within range \\
\hline
     Gauss &     $16$ &  $11.85$ & $1.96 \cdot 10^{-4}$ &              false \\
    Jacobi &     $16$ &  $15.51$ & $1.37 \cdot 10^{-5}$ &              false \\
    Seidel &     $16$ &  $13.41$ & $1.74 \cdot 10^{-6}$ &               true \\
Relaxation &     $16$ &  $13.93$ & $1.17 \cdot 10^{-6}$ &               true \\
\hline
```