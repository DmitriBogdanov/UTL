[<img src ="images/icon_cpp_std_17.svg">](https://en.cppreference.com/w/cpp/17.html)
[<img src ="images/icon_license_mit.svg">](../LICENSE.md)
[<img src ="images/icon_semver.svg">](guide_versioning.md)
[<img src ="images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)
[<img src ="images/icon_no_dependencies.svg">](https://github.com/DmitriBogdanov/UTL/tree/master/include/UTL)

[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/windows.yml?logo=github&label=Windows">](https://github.com/DmitriBogdanov/UTL/actions/workflows/windows.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/ubuntu.yml?logo=github&label=Ubuntu">](https://github.com/DmitriBogdanov/UTL/actions/workflows/ubuntu.yml)
[<img src ="https://img.shields.io/github/actions/workflow/status/DmitriBogdanov/UTL/macos.yml?logo=github&label=MacOS">](https://github.com/DmitriBogdanov/UTL/actions/workflows/macos.yml)

# utl::progressbar

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/progressbar.hpp)

**utl::progressbar** header adds configurable progress bars for CLI apps.

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

| Option                  | Description                                                 |
| ----------------------- | ----------------------------------------------------------- |
| `style.fill`            | Character used for "filled" part of the bar                 |
| `style.empty `          | Character used for "empty" part of the bar                  |
| `style.left`            | Character used for the left end of the bar                  |
| `style.right`           | Character used for the right end of the bar                 |
| `style.estimate_prefix` | Text displayed before the time estimate                     |
| `style.estimate_suffix` | Text displayed after the time estimate                      |
| `show_bar`              | Whether to render the main bar display                      |
| `show_percentage`       | Whether to render a numeric label after the bar             |
| `show_estimate`         | Whether to render a remaining time estimate                 |
| `bar_length`            | Progress bar length in characters                           |
| `update_rate`           | How often should the bar update, `1e-2` corresponds to `1%` |

**Note:** Progress bar style doesn't update until the next redraw. Immediate redraw can be triggered using `update_style()`.

> ```cpp
> Percentage();
> void Percentage::set_progress(double value);
> void Percentage::finish();
> ```

Start, update & finish progress bar display. Progress is a `value` in `[0, 1]` range, corresponding to a portion of total workload.

> ```cpp
> void update_style();
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

Style parameters that can be adjusted:

| Option                  | Description                                        |
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
> void update_style();
> ```

Redraws progress bar to update its style configuration immediately.

## Examples

### Progress bar

[ [Run this code](https://godbolt.org/z/1jWzzGncr) ]

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

[ [Run this code](https://godbolt.org/z/vWPz5d8KW) ]

```cpp
using namespace utl;
using namespace std::chrono_literals;

const int  iterations = 1500;
const auto some_work  = [] { std::this_thread::sleep_for(10ms); };

progressbar::Percentage bar;

bar.show_bar              = false;
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
