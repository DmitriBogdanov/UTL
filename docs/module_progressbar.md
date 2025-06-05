[<img src ="images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="images/icon_license_mit.svg">](./LICENSE.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/single_include)

# utl::progressbar

[<- to README.md](..)

[<- to implementation.hpp](https://github.com/DmitriBogdanov/UTL/blob/master/include/UTL/progressbar.hpp)

**utl::progressbar** adds configurable progress bars for CLI apps.

Below is basic showcase:

```
// progressbar::Percentage with default style
[############..................] 42.67% (remaining: 8 sec)

// progressbar::Ruler with default style
0    10   20   30   40   50   60   70   80   90   100%
|----|----|----|----|----|----|----|----|----|----|
#######################
```

## Definitions

```cpp
// 'Percentage' progress bar
struct Percentage {
public:
    // - Style configuration -
    struct Style {
        char        fill            = '#';
        char        empty           = '.';
        char        left            = '[';
        char        right           = ']';
        std::string estimate_prefix = "(remaining: ";
        std::string estimate_suffix = ")";
    } style;

    bool show_bar        = true;
    bool show_percentage = true;
    bool show_estimate   = true;
    
    std::size_t bar_length  = 30;
    double      update_rate = 2.5e-3;
    
    // - Methods -
    Percentage();
    void set_progress(double value);
    void finish();
    
    void update_style();
};

// 'Ruler' progress bar
class Ruler {
public:
    // - Style configuration -
    struct Style {
        char fill            = '#';
        char ruler_line      = '-';
        char ruler_delimiter = '|';
    } style;

    bool show_ticks = true;
    bool show_ruler = true;
    bool show_bar   = true;
    
    // - Methods -
    Ruler();
    void set_progress(double percentage);
    void finish();
    
    void update_style();
};
```

## Methods

### `Percentage` progress bar

> [!Note]
>
> This is a general progress bar suitable for most applications. It should be a default option unless environment is extremely limited.

> ```cpp
> // - Style configuration -
> struct Style {
>     char        fill            = '#';
>     char        empty           = '.';
>     char        left            = '[';
>     char        right           = ']';
>     std::string estimate_prefix = "(remaining: ";
>     std::string estimate_suffix = ")";
> } style;
> 
> bool show_bar        = true;
> bool show_percentage = true;
> bool show_estimate   = true;
> 
> std::size_t bar_length  = 30;
> double      update_rate = 2.5e-3;
> ```

Style parameters that can be adjusted:

| Option                  | Descriptions                                                 |
| ----------------------- | ------------------------------------------------------------ |
| `style.fill`            | Character used for "filled" part of the bar                  |
| `style.empty `          | Character used for "empty" part of the bar                   |
| `style.left`            | Character used for the left end if the bar                   |
| `style.right`           | Character used for the right end if the bar                  |
| `style.estimate_prefix` | Text displayed before the time estimate                      |
| `style.estimate_suffix` | Text displayed after the time estimate                       |
| `show_bar`              | Whether to render the main bar display                       |
| `show_percentage`       | Whether to render a numeric label after the bar              |
| `show_estimate`         | Whether to render a remaining time estimate                  |
| `bar_length`            | Progress bar length in characters                            |
| `update_rate`           | How often should the bar update, `0.01` corresponds to a single % |

**Note:** Progress bar style doesn't update until the next redraw. Immediate redraw can be triggered using `update_style()`.

> ```cpp
> Percentage();
> void Percentage::set_progress(double value);
> void Percentage::finish();
> ```

Start, update & finish progress bar display. Progress is a `value` in `[0, 1]` range, corresponding to a portion of total workload.

> ```cpp
> update_style();
> ```

Redraws progress bar to update its style configuration immediately.

### `Ruler` progress bar

> [!Note]
>
> This is a very minimalistic progress bar, it should be used for terminals that do not support `\r`.

> ```cpp
> // - Style configuration -
> struct Style {
>     char fill            = '#';
>     char ruler_line      = '-';
>     char ruler_delimiter = '|';
> } style;
> 
> bool show_ticks = true;
> bool show_ruler = true;
> bool show_bar   = true;
> ```

Construct progress bar object with following options:

| Option                  | Descriptions                                       |
| ----------------------- | -------------------------------------------------- |
| `style.fill`            | Character used for "filled" part of the bar        |
| `style.ruler_line `     | Character used for "line" part of the ruler above  |
| `style.ruler_delimiter` | Character used for delimiter on of the ruler above |
| `show_ticks`            | Whether to render the main bar display             |
| `show_ruler`            | Whether to render a numeric label after the bar    |
| `show_bar`              | Whether to render a remaining time estimate        |

**Note:** Disabling `show_bar` makes little practical sense, considering it makes progress bar not display any progress, but it is still provided for the sake of API uniformity.

> ```cpp
> Ruler();
> void Ruler::set_progress(double percentage);
> void Ruler::finish();
> ```

Start, update & finish progress bar display. Progress is a `value` in `[0, 1]` range, corresponding to a portion of total workload.

> ```cpp
> update_style();
> ```

Redraws progress bar to update its style configuration immediately.

## Examples

### Progress bar for some workload

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:13,endLineNumber:3,positionColumn:13,positionLineNumber:3,selectionStartColumn:13,selectionStartLineNumber:3,startColumn:13,startLineNumber:3),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A++++using+namespace+std::chrono_literals%3B%0A%0A++++const+int++iterations+%3D+1500%3B%0A++++const+auto+some_work++%3D+%5B%5D+%7B+std::this_thread::sleep_for(10ms)%3B+%7D%3B%0A%0A++++progressbar::Percentage+bar%3B%0A++++for+(int+i+%3D+0%3B+i+%3C+iterations%3B+%2B%2Bi)+%7B%0A++++++++some_work()%3B%0A++++++++bar.set_progress((i+%2B+1.)+/+iterations)%3B%0A++++%7D%0A++++bar.finish()%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;
using namespace std::chrono_literals;

const int  iterations = 1500;
const auto some_work  = [] { std::this_thread::sleep_for(10ms); };

progressbar::Percentage bar;
for (int i = 0; i < iterations; ++i) {
    some_work();
    bar.set_progress((i + 1.) / iterations);
}
bar.finish();
```

Output (at some point in time):
```
[############..................] 42.67% (remaining: 8 sec)
```

### Progress bar with custom style

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:71,endLineNumber:8,positionColumn:71,positionLineNumber:8,selectionStartColumn:71,selectionStartLineNumber:8,startColumn:71,startLineNumber:8),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A++++using+namespace+std::chrono_literals%3B%0A++++%0A++++const+int++iterations+%3D+1500%3B%0A++++const+auto+some_work++%3D+%5B%5D+%7B+std::this_thread::sleep_for(10ms)%3B+%7D%3B%0A++++%0A++++progressbar::Percentage+bar%3B%0A++++%0A++++bar.show_bar+%3D+false%3B%0A++++bar.style.estimate_prefix+%3D+%22complete,+remaining+time:+%22%3B%0A++++bar.style.estimate_suffix+%3D+%22%22%3B%0A%0A++++bar.update_style()%3B%0A++++%0A++++for+(int+i+%3D+0%3B+i+%3C+iterations%3B+%2B%2Bi)+%7B%0A++++++++some_work()%3B%0A++++++++bar.set_progress((i+%2B+1.)+/+iterations)%3B%0A++++%7D%0A++++bar.finish()%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;
using namespace std::chrono_literals;

const int  iterations = 1500;
const auto some_work  = [] { std::this_thread::sleep_for(10ms); };

progressbar::Percentage bar;

bar.show_bar = false;
bar.style.estimate_prefix = "complete, remaining time: ";
bar.style.estimate_suffix = "";

bar.update_style();

for (int i = 0; i < iterations; ++i) {
    some_work();
    bar.set_progress((i + 1.) / iterations);
}
bar.finish();
```

Output (at some point in time):
```
68.00% complete, remaining time: 4 sec
```
