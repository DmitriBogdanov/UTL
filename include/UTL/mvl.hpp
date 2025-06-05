// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::mvl
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_mvl.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_MVL)
#ifndef UTLHEADERGUARD_MVL
#define UTLHEADERGUARD_MVL

#define UTL_MVL_VERSION_MAJOR 0 // [!] module in early experimental stage,
#define UTL_MVL_VERSION_MINOR 1 //     functional, but needs significant work
#define UTL_MVL_VERSION_PATCH 0 //     to complete and bring up-to-date

// _______________________ INCLUDES _______________________

#include <algorithm>        // swap(), find(), count(), is_sorted(), min_element(),
                            // max_element(), sort(), stable_sort(), min(), max(), remove_if(), copy()
#include <cassert>          // assert() // Note: Perhaps temporary
#include <charconv>         // to_chars()
#include <cmath>            // isfinite()
#include <cstddef>          // size_t, ptrdiff_t, nullptr_t
#include <exception>        // exception
#include <functional>       // reference_wrapper<>, multiplies<>
#include <initializer_list> // initializer_list<>
#include <iomanip>          // setw()
#include <ios>              // right(), boolalpha(), ios::boolalpha
#include <iterator>         // random_access_iterator_tag, reverse_iterator<>
#include <memory>           // unique_ptr<>
#include <numeric>          // accumulate()
#include <ostream>          // ostream
#include <sstream>          // ostringstream
#include <stdexcept>        // out_of_range, invalid_argument
#include <string>           // string
#include <string_view>      // string_view<>
#include <type_traits>      // conditional_t<>, enable_if_t<>, void_t<>, true_type, false_type, remove_reference_t<>
#include <utility>          // move()
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// [experimental!] This header is still work in progress and likely to undergo significant changes to
// its API & implementation.
//
// This module tries to implement an "unreasonably flexible yet convenient" template for vectors and matrices,
// in order to do so we have to rely heavily on either conditional compilation or automatic code generation
// (or both). There are multiple seemingly viable approaches to it, yet the most of them proved to be either
// unsuitable or unreasonable from the implementation standpoint. Below is a little rundown of implementations
// that were attempted and used/discared for various reasons.
//
// Currently matrix/vector/view/etc code reuse is implemented though a whole bunch of conditional compilation with
// SFINAE, abuse of conditional inheritance, conditional types and constexpr if's. While not perfect, this is
// the best working approach so far. Below is a list of approaches that has been considered & tried:
//
// 1) No code reuse, all classes implemented manually - unrealistically cumbersome, huge code duplication
//
//    => [-] UNSUITABLE APPROACH
//
// 2) Regular OOP with virtual classes - having vtables in lightweight containers is highly undesirable,
//    we can avoid this using CRTP however we run into issues with multiple inheritance (see below)
//
//    => [-] UNSUITABLE APPROACH
//
// 3) CRTP - we unavoidably run into multiple inheritance issues (diamond problem) with concepts like:
//       [mutable-matrix-like] : [const-matrix-like] & [mutable-iterable] where both parents derive from
//       [const-iterable]
//
//    While those issues are resolvable, their handling becomes more and more difficult as parent classes grow larger.
//    A usual solution for this would be virtual inheritance, however it prevents us from calling derived methods inside
//    the base class due to "static downcast via virtual inheritance" rule, which makes 'static_cast<Derived*>(this)'
//    illegal. We can resolve all name collisions manually, however it leads to a code that is absolutely horrific and
//    easy to mess up.
//
//    There is also an issue of extreme boilerplate, since we want to propagate '_types<>' wrapper and the 'Final'
//    return type to all base classes we end up with extremely unwieldy template arguments that make compile errors and
//    refactoring a challenge.
//
//    => [~] SUITABLE, BUT VERY PROBLEMATIC APPROACH
//
// 4) Template-based mixins without CRTP - these allow to add functionality on top of a class, essentially it works
//    like "inverted" CRTP from the code logic POV. While being almost a perfect solution, it doesn't allow us to
//    return objects of derived type from base class methods, which is a requirement for the intended matrix API.
//
//    => [-] UNSUITABLE APPROACH
//
// 5) Macro-based mixins - insert functionality directly in a class, a simple way of doing exactly what we want.
//    "Mixed-in" methods are simply functions implemented in terms of certain "required" methods.
//    Doesn't lead to huge template chains and doesn't have any inheritance at all. The main downside is being
//    preprocessor based, which exposes some identifiers to a global namespace and makes all "connection points"
//    in logic dependant on correct naming inside macros, rather than actual semantical symbols.
//
//    Difficult to work with due to being ignored by the language server, gets increasingly cumbersome as the number of
//    classes grows and still has significant code duplication (vector/matrix * dense/strided/sparse *
//    * container/view/const_view = 18 manually created definitions).
//
//    => [+-] SUITABLE, BUT HEAVILY FLAWED APPROACH
//
// 6) A single class with all parts of API correctly enabled/disabled through SFINAE. Closest thing to a sensible
//    approach with minimal (basically none) code duplication. However it has a number of non-trivial quirks
//    when it comes to conditionally compiling member variables, types & functions (every group has a quirk of
//    its own, functions are the least problematic). Some of such quirks limit possible conditional API (member
//    types suffer from this) or, for example, prevent use of initialization lists on conditionally compiled
//    constructors (big sad).
//
//    A less obvious downside compared to macros, is that it doesn't get handles as well by the language server
//    autocomplete - SFINAE-disabled methods still show up as autocomplete suggestions, even if they get marked
//    as missing immediately upon writing the whole name.
//
//    => [+] DIFFICULT, BUT WORKABLE, BEST APPROACH SO FAR
//
// NOTES ON SPAN:
//
// We can easily implement Matlab-like API to take matrix blocks like this:
//    matrix(Span{0, 10}, 4)
// which would be equivalent to
//    matrix.block(0, 4, 10, 1)
// we only need a thin 'Span' POD with 2 members and a few overloads for 'operator()' that redirect span
// to 'this->block(...)'
//
// NOTES ON NAMING:
//
// Macro naming is a bit of a mess as of now.

// ____________________ IMPLEMENTATION ____________________

namespace utl::mvl {

// ===================
// --- Type Traits ---
// ===================

// Macro for generating type traits with all their boilerplate
//
// While it'd be nice to avoid macro usage alltogether, having a few macros for generating standardized boilerplate
// that gets repeated several dozen times DRASTICALLY improves the maintainability of the whole conditional compilation
// mechanism down the line. They will be later #undef'ed.

#define utl_mvl_define_trait(trait_name_, ...)                                                                         \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value;                                                            \
                                                                                                                       \
    template <class T>                                                                                                 \
    using trait_name_##_enable_if = std::enable_if_t<trait_name_<T>::value, bool>

// Shortcuts for different types of requirements

#define utl_mvl_define_trait_has_binary_op(trait_name_, op_)                                                           \
    utl_mvl_define_trait(trait_name_, std::declval<std::decay_t<T>>() op_ std::declval<std::decay_t<T>>())

#define utl_mvl_define_trait_has_assignment_op(trait_name_, op_)                                                       \
    utl_mvl_define_trait(trait_name_, std::declval<std::decay_t<T>&>() op_ std::declval<std::decay_t<T>>())
// for operators like '+=' lhs should be a reference

#define utl_mvl_define_trait_has_unary_op(trait_name_, op_)                                                            \
    utl_mvl_define_trait(trait_name_, op_ std::declval<std::decay_t<T>>())

#define utl_mvl_define_trait_has_member(trait_name_, member_)                                                          \
    utl_mvl_define_trait(trait_name_, std::declval<std::decay_t<T>>().member_)

#define utl_mvl_define_trait_has_member_type(trait_name_, member_)                                                     \
    utl_mvl_define_trait(trait_name_, std::declval<typename std::decay_t<T>::member_>())

// --- Type traits ---
// -------------------

utl_mvl_define_trait(_has_ostream_output_op, std::declval<std::ostream>() << std::declval<T>());

utl_mvl_define_trait_has_binary_op(_has_binary_op_plus, +);
utl_mvl_define_trait_has_binary_op(_has_binary_op_minus, -);
utl_mvl_define_trait_has_binary_op(_has_binary_op_multiplies, *);
utl_mvl_define_trait_has_binary_op(_has_binary_op_less, <);
utl_mvl_define_trait_has_binary_op(_has_binary_op_greater, >);
utl_mvl_define_trait_has_binary_op(_has_binary_op_equal, ==);

utl_mvl_define_trait_has_assignment_op(_has_assignment_op_plus, +=);
utl_mvl_define_trait_has_assignment_op(_has_assignment_op_minus, -=);
utl_mvl_define_trait_has_assignment_op(_has_assignment_op_multiplies, *=);

utl_mvl_define_trait_has_unary_op(_has_unary_op_plus, +);
utl_mvl_define_trait_has_unary_op(_has_unary_op_minus, -);

utl_mvl_define_trait_has_member(_has_member_i, i);
utl_mvl_define_trait_has_member(_has_member_j, j);
utl_mvl_define_trait_has_member(_has_member_value, value);

utl_mvl_define_trait_has_member(_is_tensor, is_tensor);
utl_mvl_define_trait_has_member(_is_sparse_entry_1d, is_sparse_entry_1d);
utl_mvl_define_trait_has_member(_is_sparse_entry_2d, is_sparse_entry_2d);

// =======================
// --- Stringification ---
// =======================

// Stringification implementation from 'utl::log' module, see its source for more notes.

// --- Internal type traits ---
// ----------------------------

utl_mvl_define_trait(_has_string_append, std::string() += std::declval<T>());
utl_mvl_define_trait(_has_real, std::declval<T>().real());
utl_mvl_define_trait(_has_imag, std::declval<T>().imag());
utl_mvl_define_trait(_has_begin, std::declval<T>().begin());
utl_mvl_define_trait(_has_end, std::declval<T>().end());
utl_mvl_define_trait(_has_input_iter, std::next(std::declval<T>().begin()));
utl_mvl_define_trait(_has_get, std::get<0>(std::declval<T>()));
utl_mvl_define_trait(_has_tuple_size, std::tuple_size<T>::value);
utl_mvl_define_trait(_has_ostream_insert, std::declval<std::ostream>() << std::declval<T>());

// --- Internal utils ---
// ----------------------

template <class>
inline constexpr bool _always_false_v = false;

template <class T>
constexpr int _log_10_ceil(T num) {
    return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
}

template <class T>
constexpr int _max_float_digits =
    4 + std::numeric_limits<T>::max_digits10 + std::max(2, _log_10_ceil(std::numeric_limits<T>::max_exponent10));

template <class T>
constexpr int _max_int_digits = 2 + std::numeric_limits<T>::digits10;

// --- Stringifier ---
// -------------------

// TODO:
// Replace or rethink all of this stringifying stuff, perhaps it would be best to just have a super-simple
// one and let other functions take custom stringifier from 'utl::log'

template <class T>
void _append_stringified(std::string& str, const T& value);

inline void _append_stringified_bool(std::string& str, bool value) { str += value ? "true" : "false"; }

template <class T>
void _append_stringified_integer(std::string& str, T value) {
    std::array<char, _max_int_digits<T>> buffer;
    const auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    if (error_code != std::errc())
        throw std::runtime_error(
            "Integer stringification encountered std::to_chars() formatting error while serializing a value.");
    str.append(buffer.data(), number_end_ptr - buffer.data());
}

template <class T>
void _append_stringified_float(std::string& str, T value) {
    std::array<char, _max_float_digits<T>> buffer;
    const auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    if (error_code != std::errc())
        throw std::runtime_error(
            "Float stringification encountered std::to_chars() formatting error while serializing a value.");
    str.append(buffer.data(), number_end_ptr - buffer.data());
}

template <class T>
void _append_stringified_complex(std::string& str, T value) {
    _append_stringified_float(str, value.real());
    str += " + ";
    _append_stringified_float(str, value.imag());
    str += " i";
}

template <class T>
void _append_stringified_stringlike(std::string& str, const T& value) {
    str += value;
}

template <class T>
void _append_stringified_string_convertible(std::string& str, const T& value) {
    str += std::string(value);
}

template <class T>
void _append_stringified_array(std::string& str, const T& value) {
    str += "{ ";
    if (value.begin() != value.end())
        for (auto it = value.begin();;) {
            _append_stringified(str, *it);
            if (++it == value.end()) break;
            str += ", ";
        }
    str += " }";
}

template <class Tuple, std::size_t... Idx>
void _append_stringified_tuple_impl(std::string& str, Tuple value, std::index_sequence<Idx...>) {
    ((Idx == 0 ? "" : str += ", ", _append_stringified(str, std::get<Idx>(value))), ...);
}

template <template <class...> class Tuple, class... Args>
void _append_stringified_tuple(std::string& str, const Tuple<Args...>& value) {
    str += "< ";
    _append_stringified_tuple_impl(str, value, std::index_sequence_for<Args...>{});
    str += " >";
}

template <class T>
void _append_stringified_printable(std::string& str, const T& value) {
    str += (std::ostringstream() << value).str();
}

// --- Selector ---
// ----------------

template <class T>
void _append_stringified(std::string& str, const T& value) {
    if constexpr (std::is_same_v<T, bool>) _append_stringified_bool(str, value);
    else if constexpr (std::is_same_v<T, char>) _append_stringified_stringlike(str, value);
    else if constexpr (std::is_integral_v<T>) _append_stringified_integer(str, value);
    else if constexpr (std::is_floating_point_v<T>) _append_stringified_float(str, value);
    else if constexpr (_has_real_v<T> && _has_imag_v<T>) _append_stringified_complex(str, value);
    else if constexpr (std::is_convertible_v<T, std::string_view>) _append_stringified_stringlike(str, value);
    else if constexpr (std::is_convertible_v<T, std::string>) _append_stringified_string_convertible(str, value);
    else if constexpr (_has_begin_v<T> && _has_end_v<T> && _has_input_iter_v<T>) _append_stringified_array(str, value);
    else if constexpr (_has_get_v<T> && _has_tuple_size_v<T>) _append_stringified_tuple(str, value);
    else if constexpr (_has_ostream_insert_v<T>) _append_stringified_printable(str, value);
    else static_assert(_always_false_v<T>, "No valid stringification exists for the type.");
}

// --- Public API ---
// ------------------

template <class... Args>
void append_stringified(std::string& str, Args&&... args) {
    (_append_stringified(str, std::forward<Args>(args)), ...);
}

template <class... Args>
[[nodiscard]] std::string stringify(Args&&... args) {
    std::string buffer;
    append_stringified(buffer, std::forward<Args>(args)...);
    return buffer;
}

// Override "common special cases" that can be improved relative to a generic implementation
[[nodiscard]] inline std::string stringify(int value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(long value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(long long value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(unsigned int value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(unsigned long value) { return std::to_string(value); }
[[nodiscard]] inline std::string stringify(unsigned long long value) { return std::to_string(value); }

// We wrap stringifying function in functor-class so we can use it a default template callable argument.
// Templates can't infer template parameters from default arguments:
//
//    template<class Func>
//    void do_stuff(Func f = default_f);  // <- CAN'T infer 'Func'
//
//    template<class Func = default_functor>
//    void do_stuff(Func f = Func());     // <- CAN infer 'Func'
//
// which is why the "standard" way of doing it (standard as in used by STL containers) is to use functors as default
// template arguments, if user passes a callable it will override the 'Func' and we get the usual behaviour.
//
template <class T>
struct default_stringifier {
    [[nodiscard]] std::string operator()(const T& value) const { return stringify(value); }
};

// ======================
// --- Codegen Macros ---
// ======================

#define utl_mvl_assert(condition_) assert(condition_)
// if (!__VA_ARGS__) throw std::runtime_error("Failed assert on line " + std::to_string(__LINE__))

// ========================
// --- Helper Functions ---
// ========================

// Shortcut for lambda-type-based SFINAE.
//
// Callables in this module are usually takes as a template type since 'std::function<>' introduces very significant
// overhead with its type erasure. With template args all lambdas and functors can be nicely inlined, however we lose
// the ability to overload functions that take callable the way we could with 'std::function<>'.
//
// As a workaround we can use SFINAE to reject "overloads" tha don't have a particular signature, effectively achieving
// the behaviour we need.
template <class FuncType, class Signature>
using _has_signature_enable_if = std::enable_if_t<std::is_convertible_v<FuncType, std::function<Signature>>, bool>;

template <class T>
[[nodiscard]] std::unique_ptr<T[]> _make_unique_ptr_array(size_t size) {
    return std::unique_ptr<T[]>(new T[size]);
}

// Marker for unreachable code
[[noreturn]] inline void _unreachable() {
// (Implementation from https://en.cppreference.com/w/cpp/utility/unreachable)
// Use compiler specific extensions if possible.
// Even if no extension is used, undefined behavior is still raised by
// an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

// =======================
// --- Utility Classes ---
// =======================

// Wrapper that allows passing standard set of member types down the CRTP chain
// (types have to be passed through template args one way or another. Trying to access
// 'derived_type::value_type' from the CRTP base class will fail, since in order for it to be available
// the derived class has to be "implemented" which can only happen after the base class is "implemented",
// which requires the type => a logical loop. So we wrap all the types in a class a pass them down
// the chain a "pack with everything" type of deal. This pack then gets "unwrapped" with
// '_utl_storage_define_types' macro in every class => we have all the member types defined).
//
// (!) Since CRTP approach has been deprecated in favor of mixins, the original reasoning is no longer
// accurate, however it is this a useful class whenever a pack of types has to be passed through a
// template (like, for example, in iterator implementation)
template <class T>
struct _types {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
};

// A minimal equivalent of the once proposed 'std::observer_ptr<>'.
// Used in views to allow observer pointers with the same interface as 'std::unique_ptr',
// which means a generic '.data()` can be implemented without having to make a separate version for views and containers
template <class T>
class _observer_ptr {
public:
    using element_type = T;

private:
    element_type* _data = nullptr;

public:
    // - Constructors -
    constexpr _observer_ptr() noexcept = default;
    constexpr _observer_ptr(std::nullptr_t) noexcept {}
    explicit _observer_ptr(element_type* ptr) : _data(ptr) {}

    _observer_ptr& operator=(element_type* ptr) {
        this->_data = ptr;
        return *this;
    }

    // - Interface -
    [[nodiscard]] constexpr element_type* get() const noexcept { return _data; }
};

// =================
// --- Iterators ---
// =================

// Iterator template for 1D iterator over a tensor.
// Reduces code duplication for const & non-const variants by bundling some conditional logic.
// Uses 'operator[]' of 'ParentPointer' for all logic, thus allowing arbitrary tensors that
// don't have to be contiguous or even ordered in memory.
template <class ParentPointer, class Types, bool is_const_iter = false>
class _flat_iterator {
    using parent_pointer = ParentPointer;

public:
    // Iterator reqs: [General]
    // Contains member types      =>
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = typename Types::difference_type;
    using value_type        = typename Types::value_type;
    using pointer = typename std::conditional_t<is_const_iter, typename Types::const_pointer, typename Types::pointer>;
    using reference =
        typename std::conditional_t<is_const_iter, typename Types::const_reference, typename Types::reference>;

    // Iterator reqs: [General]
    // Constructible
    _flat_iterator(parent_pointer parent, difference_type idx) : _parent(parent), _idx(idx) {}
    // Copy-constructible => by default
    // Copy-assignable    => by default
    // Destructible       => by default
    // Swappable
    friend void swap(_flat_iterator& lhs, _flat_iterator& rhs) { std::swap(lhs._idx, rhs._idx); }

    // Iterator reqs: [General]
    // Can be incremented (prefix & postfix)
    _flat_iterator& operator++() {
        ++_idx;
        return *this;
    } // prefix
    _flat_iterator operator++(int) {
        _flat_iterator temp = *this;
        ++_idx;
        return temp;
    } // postfix

    // Iterator reqs: [Input iterator]
    // Supports equality/inequality comparisons
    friend bool operator==(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx == it2._idx; };
    friend bool operator!=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx != it2._idx; };
    // Can be dereferenced as an rvalue
    reference   operator*() const { return _parent->operator[](_idx); }
    pointer     operator->() const { return &_parent->operator[](_idx); }

    // Iterator reqs: [Output iterator]
    // Can be dereferenced as an lvalue (only for mutable iterator types) => not needed

    // Iterator reqs: [Forward iterator]
    // Default-constructible
    _flat_iterator() : _parent(nullptr), _idx(0) {}
    // "Multi-pass" - dereferencing & incrementing does not affects dereferenceability => satisfied

    // Iterator reqs: [Bidirectional iterator]
    // Can be decremented
    _flat_iterator& operator--() {
        --_idx;
        return *this;
    } // prefix
    _flat_iterator operator--(int) {
        _flat_iterator temp = *this;
        --_idx;
        return temp;
    } // postfix

    // Iterator reqs: [Random Access iterator]
    // See: https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
    // Supports arithmetic operators: it + n, n + it, it - n, it1 - it2
    friend _flat_iterator operator+(const _flat_iterator& it, difference_type diff) {
        return _flat_iterator(it._parent, it._idx + diff);
    }
    friend _flat_iterator operator+(difference_type diff, const _flat_iterator& it) {
        return _flat_iterator(it._parent, it._idx + diff);
    }
    friend _flat_iterator operator-(const _flat_iterator& it, difference_type diff) {
        return _flat_iterator(it._parent, it._idx - diff);
    }
    friend difference_type operator-(const _flat_iterator& it1, const _flat_iterator& it2) {
        return it1._idx - it2._idx;
    }
    // Supports inequality comparisons (<, >, <= and >=) between iterators
    friend bool     operator<(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx < it2._idx; }
    friend bool     operator>(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx > it2._idx; }
    friend bool     operator<=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx <= it2._idx; }
    friend bool     operator>=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx >= it2._idx; }
    // Standard assumption: Both iterators are from the same container => no need to compare '_parent'
    // Supports compound assignment operations += and -=
    _flat_iterator& operator+=(difference_type diff) {
        _idx += diff;
        return *this;
    }
    _flat_iterator& operator-=(difference_type diff) {
        _idx -= diff;
        return *this;
    }
    // Supports offset dereference operator ([])
    reference operator[](difference_type diff) const { return _parent->operator[](_idx + diff); }

private:
    parent_pointer  _parent;
    difference_type _idx; // not size_type because we have to use it in 'difference_type' operations most of the time
};

// ======================================
// --- Sparse Matrix Pairs & Triplets ---
// ======================================

// Note:
// All sparse entries and multi-dimensional indices can be sorted lexicographically

template <class T>
struct SparseEntry1D {
    size_t i;
    T      value;

    constexpr static bool is_sparse_entry_1d = true;

    [[nodiscard]] bool operator<(const SparseEntry1D& other) const noexcept { return this->i < other.i; }
    [[nodiscard]] bool operator>(const SparseEntry1D& other) const noexcept { return this->i > other.i; }
};

template <class T>
struct SparseEntry2D {
    size_t i;
    size_t j;
    T      value;

    constexpr static bool is_sparse_entry_2d = true;

    [[nodiscard]] bool operator<(const SparseEntry2D& other) const noexcept {
        return (this->i < other.i) || (this->j < other.j);
    }
    [[nodiscard]] bool operator>(const SparseEntry2D& other) const noexcept {
        return (this->i > other.i) || (this->j > other.j);
    }
};

struct Index2D {
    size_t i;
    size_t j;

    [[nodiscard]] bool operator<(const Index2D& other) const noexcept {
        return (this->i < other.i) || (this->j < other.j);
    }
    [[nodiscard]] bool operator>(const Index2D& other) const noexcept {
        return (this->i > other.i) || (this->j > other.j);
    }
    [[nodiscard]] bool operator==(const Index2D& other) const noexcept {
        return (this->i == other.i) && (this->j == other.j);
    }
};

// Functions that take 2 sparse entries '{ i, j, v1 }' and '{ i, j, v2 }' and return '{ i, j, op(v1, v2) }'.
// Same thing for 1D sparse entries. LHS/RHS index correctness only gets checked in debug.
//
// Declaring these functions saves us from having to duplicate implementations of 'apply_binary_operator()'
// for 1D and 2D sparse entries that have different number of constructor args. Instead, those cases will
// be handled by '_apply_binary_op_to_sparse_entry()' that is SFINAE-overloaded to work with both.
//
// Implementations use perfect forwarding for everything and do the "overloading" through SFINAE with type traits.
//
// In reality 'op' doesn't ever get forwarded as an r-value but since are doing things generic why not forward it
// properly as well. This is also somewhat ugly, but it works well enough and there isn't much reason to rewrite
// this with some new abstraction.
//
template <class L, class R, class Op, _is_sparse_entry_1d_enable_if<L> = true, _is_sparse_entry_1d_enable_if<R> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entries(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.i == right.i);
    return {left.i, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right).value)};
}

template <class L, class R, class Op, _is_sparse_entry_2d_enable_if<L> = true, _is_sparse_entry_2d_enable_if<R> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entries(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.i == right.i);
    utl_mvl_assert(left.j == right.j);
    return {left.i, left.j, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right).value)};
}

template <class L, class R, class Op, _is_sparse_entry_1d_enable_if<L> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entry_and_value(L&& left, R&& right_value, Op&& op) {
    return {left.i, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right_value))};
}

template <class L, class R, class Op, _is_sparse_entry_2d_enable_if<L> = true>
std::decay_t<L> _apply_binary_op_to_sparse_entry_and_value(L&& left, R&& right_value, Op&& op) {
    return {left.i, left.j, std::forward<Op>(op)(std::forward<L>(left).value, std::forward<R>(right_value))};
}

template <class L, class R, class Op, _is_sparse_entry_1d_enable_if<R> = true>
std::decay_t<R> _apply_binary_op_to_value_and_sparse_entry(L&& left_value, R&& right, Op&& op) {
    return {right.i, std::forward<Op>(op)(std::forward<L>(left_value), std::forward<R>(right).value)};
}

template <class L, class R, class Op, _is_sparse_entry_2d_enable_if<R> = true>
std::decay_t<R> _apply_binary_op_to_value_and_sparse_entry(L&& left_value, R&& right, Op&& op) {
    return {right.i, right.j, std::forward<Op>(op)(std::forward<L>(left_value), std::forward<R>(right).value)};
}

// TODO:
// Fix binary ops, figure out what the hell did I even do here

// =============
// --- Enums ---
// =============

// Parameter enums
//
// Their combination specifies compiled tensor API.
enum class Dimension { VECTOR, MATRIX };
enum class Type { DENSE, STRIDED, SPARSE };
enum class Ownership { CONTAINER, VIEW, CONST_VIEW };

// Config enums
//
// They specify conditional logic for a tensor.
// Combination of config & parameter enums fully defines a GenericTensor.
enum class Checking { NONE, BOUNDS };
enum class Layout { /* 1D */ FLAT, /* 2D */ RC, CR, /* Other */ SPARSE };

// Shortcut template used to deduce type of '_data' based on tensor 'ownership' parameter
template <Ownership ownership, class ContainerResult, class ViewResult, class ConstViewResult>
using _choose_based_on_ownership =
    std::conditional_t<ownership == Ownership::CONTAINER, ContainerResult,
                       std::conditional_t<ownership == Ownership::VIEW, ViewResult, ConstViewResult>>;

// Shortcut template used to check that both arguments are tensors
// that have the same value type, mostly used in binary operators
// for nicer error messages and to prevent accidental conversions
template <class L, class R>
using _are_tensors_with_same_value_type_enable_if =
    std::enable_if_t<_is_tensor_v<L> && _is_tensor_v<R> &&
                         std::is_same_v<typename std::decay_t<L>::value_type, typename std::decay_t<R>::value_type>,
                     bool>;

// =====================================
// --- Boilerplate generation macros ---
// =====================================

// Macros used to pass around unwieldy chains of tensor template arguments.
//
#define utl_mvl_tensor_arg_defs                                                                                        \
    class T, Dimension _dimension, Type _type, Ownership _ownership, Checking _checking, Layout _layout

#define utl_mvl_tensor_arg_vals T, _dimension, _type, _ownership, _checking, _layout

// Incredibly important macros used for conditional compilation of member functions.
// They automatically create the boilerplate that makes member functions dependant on the template parameters,
// which is necessary for conditional compilation and "forward" to a 'enable_if' condition.
//
// 'utl_mvl_require' is intended to be used inside template methods (template in a sense that they have other template
// args besides this conditional compilation boilerplate which technically makes all methods template).
//
// 'utl_mvl_reqs' is used when we want only conditional compilation (aka majority of cases) and cuts down on
// boilerplate even more.
//
#define utl_mvl_require(condition_)                                                                                    \
    class value_type = T, Dimension dimension = _dimension, Type type = _type, Ownership ownership = _ownership,       \
          Checking checking = _checking, Layout layout = _layout, std::enable_if_t<condition_, bool> = true

#define utl_mvl_reqs(condition_) template <utl_mvl_require(condition_)>

// A somewhat scuffed version of trait-defining macro used to create SFINAE-restrictions
// on tensor params in free functions. Only supports trivial conditions of the form
// '<parameter> [==][!=] <value>'. Perhaps there is a better way of doing it, but I'm not yet sure.
//
// Used mostly to restrict linear algebra operations for sparse/nonsparse so we can get
// a "SFINAE-driven overloading" on operators that take both arguments with perfect forwarding,
// which means they can't be overloaded in a regular way
//
#define utl_mvl_define_tensor_param_restriction(trait_name_, expr_)                                                    \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = (std::decay_t<T>::params::expr_);                                                 \
                                                                                                                       \
    template <class T>                                                                                                 \
    using trait_name_##_enable_if = std::enable_if_t<trait_name_##_v<T>, bool>;

utl_mvl_define_tensor_param_restriction(_is_sparse_tensor, type == Type::SPARSE);
utl_mvl_define_tensor_param_restriction(_is_nonsparse_tensor, type != Type::SPARSE);
utl_mvl_define_tensor_param_restriction(_is_matrix_tensor, dimension == Dimension::MATRIX);

// ===========================
// --- Data Member Classes ---
// ===========================

// Unlike class method, member values can't be templated, which prevents us from using regular 'enable_if_t' SFINAE
// for their conditional compilation. The (seemingly) best workaround to compile members conditionally is to inherit
// 'std::conditional<T, EmptyClass>' where 'T' is a "dummy" class with the sole purpose of having data members to
// inherit. This does not introduce virtualization (which is good, that how we want it).

template <int id>
struct _nothing {};

template <utl_mvl_tensor_arg_defs>
class _2d_extents {
private:
    using size_type = typename _types<T>::size_type;

public:
    size_type _rows = 0;
    size_type _cols = 0;
};

template <utl_mvl_tensor_arg_defs>
class _2d_strides {
private:
    using size_type = typename _types<T>::size_type;

public:
    size_type _row_stride = 0;
    size_type _col_stride = 0;
};

template <utl_mvl_tensor_arg_defs>
struct _2d_dense_data {
private:
    using value_type = typename _types<T>::value_type;
    using _data_t    = _choose_based_on_ownership<_ownership, std::unique_ptr<value_type[]>, _observer_ptr<value_type>,
                                               _observer_ptr<const value_type>>;

public:
    _data_t _data;
};

template <utl_mvl_tensor_arg_defs>
struct _2d_sparse_data {
private:
    using value_type = typename _types<T>::value_type;
    using _triplet_t = _choose_based_on_ownership<_ownership, SparseEntry2D<value_type>,
                                                  SparseEntry2D<std::reference_wrapper<value_type>>,
                                                  SparseEntry2D<std::reference_wrapper<const value_type>>>;

public:
    using triplet_type = _triplet_t;

    std::vector<triplet_type> _data;
};

// ===================
// --- Tensor Type ---
// ===================

template <utl_mvl_tensor_arg_defs>
class GenericTensor
    // Conditionally compile member variables through inheritance
    : public std::conditional_t<_dimension == Dimension::MATRIX, _2d_extents<utl_mvl_tensor_arg_vals>, _nothing<1>>,
      public std::conditional_t<_dimension == Dimension::MATRIX && _type == Type::STRIDED,
                                _2d_strides<utl_mvl_tensor_arg_vals>, _nothing<2>>,
      public std::conditional_t<_type == Type::DENSE || _type == Type::STRIDED, _2d_dense_data<utl_mvl_tensor_arg_vals>,
                                _nothing<3>>,
      public std::conditional_t<_type == Type::SPARSE, _2d_sparse_data<utl_mvl_tensor_arg_vals>, _nothing<4>>
// > After this point no non-static member variables will be introduced
{
    // --- Parameter reflection ---
    // ----------------------------
public:
    struct params {
        constexpr static auto dimension = _dimension;
        constexpr static auto type      = _type;
        constexpr static auto ownership = _ownership;
        constexpr static auto checking  = _checking;
        constexpr static auto layout    = _layout;

        // Prevent impossible layouts
        static_assert((dimension == Dimension::VECTOR) == (layout == Layout::FLAT), "Flat layout <=> matrix is 1D.");
        static_assert((type == Type::SPARSE) == (layout == Layout::SPARSE), "Sparse layout <=> matrix is sparse.");
    };

    constexpr static bool is_tensor = true;

    // --- Member types ---
    // --------------------
private:
    using _type_wrapper = _types<T>;

public:
    using self            = GenericTensor;
    using value_type      = typename _type_wrapper::value_type;
    using size_type       = typename _type_wrapper::size_type;
    using difference_type = typename _type_wrapper::difference_type;
    using reference       = typename _type_wrapper::reference;
    using const_reference = typename _type_wrapper::const_reference;
    using pointer         = typename _type_wrapper::pointer;
    using const_pointer   = typename _type_wrapper::const_pointer;

    using owning_reflection = GenericTensor<value_type, params::dimension, params::type, Ownership::CONTAINER,
                                            params::checking, params::layout>;
    // container type corresponding to 'self', this is the return type of algebraic operations on a tensor

    // --- Iterators ---
    // -----------------
public:
    using const_iterator         = _flat_iterator<const self*, _type_wrapper, true>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using iterator = std::conditional_t<_ownership != Ownership::CONST_VIEW, _flat_iterator<self*, _type_wrapper>,
                                        const_iterator>; // for const views 'iterator' is the same as 'const_iterator'
    using reverse_iterator = std::reverse_iterator<iterator>;

    [[nodiscard]] const_iterator cbegin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(this, this->size()); }
    [[nodiscard]] const_iterator begin() const { return this->cbegin(); }
    [[nodiscard]] const_iterator end() const { return this->cend(); }

    [[nodiscard]] const_reverse_iterator crbegin() const { return const_reverse_iterator(this->cend()); }
    [[nodiscard]] const_reverse_iterator crend() const { return const_reverse_iterator(this->cbegin()); }
    [[nodiscard]] const_reverse_iterator rbegin() const { return this->crbegin(); }
    [[nodiscard]] const_reverse_iterator rend() const { return this->crend(); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] iterator begin() { return iterator(this, 0); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] iterator end() { return iterator(this, this->size()); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] reverse_iterator rbegin() { return reverse_iterator(this->end()); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] reverse_iterator rend() { return reverse_iterator(this->begin()); }

    // --- Basic getters ---
    // ---------------------
public:
    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type size() const noexcept { return this->rows() * this->cols(); }

    utl_mvl_reqs(type == Type::SPARSE)
    [[nodiscard]] size_type size() const noexcept { return this->_data.size(); }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] size_type rows() const noexcept { return this->_rows; }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] size_type cols() const noexcept { return this->_cols; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] constexpr size_type row_stride() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return 0;
        if constexpr (self::params::layout == Layout::CR) return 1;
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] constexpr size_type col_stride() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return 1;
        if constexpr (self::params::layout == Layout::CR) return 0;
        _unreachable();
    }
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type row_stride() const noexcept { return this->_row_stride; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type col_stride() const noexcept { return this->_col_stride; }

    utl_mvl_reqs(type == Type::DENSE || type == Type::STRIDED)
    [[nodiscard]] const_pointer data() const noexcept { return this->_data.get(); }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] pointer data() noexcept { return this->_data.get(); }

    [[nodiscard]] bool empty() const noexcept { return (this->size() == 0); }

    // --- Advanced getters ---
    // ------------------------
    utl_mvl_reqs(_has_binary_op_equal<value_type>::value)
    [[nodiscard]] bool contains(const_reference value) const {
        return std::find(this->cbegin(), this->cend(), value) != this->cend();
    }

    utl_mvl_reqs(_has_binary_op_equal<value_type>::value)
    [[nodiscard]] size_type count(const_reference value) const {
        return std::count(this->cbegin(), this->cend(), value);
    }

    utl_mvl_reqs(_has_binary_op_less<value_type>::value)
    [[nodiscard]] bool is_sorted() const { return std::is_sorted(this->cbegin(), this->cend()); }

    template <class Compare>
    [[nodiscard]] bool is_sorted(Compare cmp) const {
        return std::is_sorted(this->cbegin(), this->cend(), cmp);
    }

    [[nodiscard]] std::vector<value_type> to_std_vector() const { return std::vector(this->cbegin(), this->cend()); }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    self transposed() const {
        self res(this->cols(), this->rows());
        this->for_each([&](const value_type& element, size_type i, size_type j) { res(j, i) = element; });
        return res;
    }

    utl_mvl_reqs(ownership == Ownership::CONTAINER)
    [[nodiscard]] self clone() const { return *this; }

    utl_mvl_reqs(ownership == Ownership::CONTAINER)
    [[nodiscard]] self move() & { return std::move(*this); }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout>
    [[nodiscard]] bool
    compare_contents(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                         other_checking, other_layout>& other) const {
        // Surface-level checks
        if ((this->rows() != other.rows()) || (this->cols() != other.cols())) return false;
        // Compare while respecting sparsity
        constexpr bool is_sparse_l = (self::params::type == Type::SPARSE);
        constexpr bool is_sparse_r = (std::remove_reference_t<decltype(other)>::params::type == Type::SPARSE);
        // Same sparsity comparison
        if constexpr (is_sparse_l == is_sparse_r) {
            return this->size() == other.size() &&
                   this->true_for_all([&](const_reference e, size_type i, size_type j) { return e == other(i, j); });
        }
        // Different sparsity comparison
        // TODO: Impl here and use .all_of() OR .any_of()
        return true;
    }

    // --- Indexation ---
    // ------------------
public:
    // Vector API
    [[nodiscard]] const_reference front() const { return this->operator[](0); }
    [[nodiscard]] const_reference back() const { return this->operator[](this->size() - 1); }
    [[nodiscard]] reference       front() { return this->operator[](0); }
    [[nodiscard]] reference       back() { return this->operator[](this->size() - 1); }

private:
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type _get_memory_offset_strided_impl(size_type idx, size_type i, size_type j) const {
        if constexpr (self::params::layout == Layout::RC) return idx * this->col_stride() + this->row_stride() * i;
        if constexpr (self::params::layout == Layout::CR) return idx * this->row_stride() + this->col_stride() * j;
        _unreachable();
    }

public:
    utl_mvl_reqs(type == Type::DENSE)
    [[nodiscard]] size_type get_memory_offset_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return idx;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] size_type get_memory_offset_of_ij(size_type i, size_type j) const {
        return this->get_idx_of_ij(i, j);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type get_memory_offset_of_idx(size_type idx) const {
        const auto ij = this->get_ij_of_idx(idx);
        return _get_memory_offset_strided_impl(idx, ij.i, ij.j);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type get_memory_offset_of_ij(size_type i, size_type j) const {
        const auto idx = this->get_idx_of_ij(i, j);
        return _get_memory_offset_strided_impl(idx, i, j);
    }

public:
    // - Flat indexation -
    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX &&
                 (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] reference operator[](size_type idx) { return this->data()[this->get_memory_offset_of_idx(idx)]; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] const_reference operator[](size_type idx) const {
        return this->data()[this->get_memory_offset_of_idx(idx)];
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::VECTOR || type == Type::SPARSE)
    [[nodiscard]] reference operator[](size_type idx) {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return this->_data[idx].value;
    }

    utl_mvl_reqs(dimension == Dimension::VECTOR || type == Type::SPARSE)
    [[nodiscard]] const_reference operator[](size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return this->_data[idx].value;
    }

    // - 2D indexation -
    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX &&
                 (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] reference operator()(size_type i, size_type j) {
        return this->data()[this->get_memory_offset_of_ij(i, j)];
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] const_reference operator()(size_type i, size_type j) const {
        return this->data()[this->get_memory_offset_of_ij(i, j)];
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] reference operator()(size_type i, size_type j) {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(i, j);
        return this->_data[this->get_idx_of_ij(i, j)].value;
    }
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] const_reference operator()(size_type i, size_type j) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(i, j);
        return this->_data[this->get_idx_of_ij(i, j)].value;
    }

    // --- Index conversions ---
    // -------------------------

    // - Bound checking -
private:
    void _bound_check_idx(size_type idx) const {
        if (idx >= this->size())
            throw std::out_of_range(
                stringify("idx (which is ", idx, ") >= this->size() (which is ", this->size(), ")"));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    void _bound_check_ij(size_type i, size_type j) const {
        if (i >= this->rows())
            throw std::out_of_range(stringify("i (which is ", i, ") >= this->rows() (which is ", this->rows(), ")"));
        else if (j >= this->cols())
            throw std::out_of_range(stringify("j (which is ", j, ") >= this->cols() (which is ", this->cols(), ")"));
    }

    // - Dense & strided implementations -
private:
    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type _unchecked_get_idx_of_ij(size_type i, size_type j) const {
        if constexpr (self::params::layout == Layout::RC) return i * this->cols() + j;
        if constexpr (self::params::layout == Layout::CR) return j * this->rows() + i;
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] Index2D _unchecked_get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::layout == Layout::RC) return {idx / this->cols(), idx % this->cols()};
        if constexpr (self::params::layout == Layout::CR) return {idx % this->rows(), idx / this->rows()};
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    [[nodiscard]] size_type _total_allocated_size() const noexcept {
        // Note 1: Allocated size of the strided matrix is NOT equal to .size() (which is same as rows * cols)
        // This is due to all the padding between the actual elements
        // Note 2: The question of whether .size() should return the number of 'strided' elements or the number
        // of actually allocated elements is rather perplexing, while the second option is more "correct" its
        // usefulness is dubious at bets, while the first one does in fact allow convenient 1D iteration over
        // all the elements. Option 1 also provides an API consistent with strided views, which is why it ended
        // up being the one chosen
        //
        // This method returns a true allocated size
        if constexpr (self::params::layout == Layout::RC)
            return (this->rows() - 1) * this->row_stride() + this->rows() * this->cols() * this->col_stride();
        if constexpr (self::params::layout == Layout::CR)
            return (this->cols() - 1) * this->col_stride() + this->rows() * this->cols() * this->row_stride();
        _unreachable();
    }

public:
    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type get_idx_of_ij(size_type i, size_type j) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_ij(i, j);
        return _unchecked_get_idx_of_ij(i, j);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] Index2D get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return _unchecked_get_ij_of_idx(idx);
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type extent_major() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return this->rows();
        if constexpr (self::params::layout == Layout::CR) return this->cols();
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type extent_minor() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return this->cols();
        if constexpr (self::params::layout == Layout::CR) return this->rows();
        _unreachable();
    }

    // - Sparse implementations -
private:
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] size_type _search_ij(size_type i, size_type j) const noexcept {
        // Returns this->size() if {i, j} wasn't found.
        // Linear search for small .size() (more efficient fue to prediction and cache locality)
        if (true) {
            for (size_type idx = 0; idx < this->size(); ++idx)
                if (this->_data[idx].i == i && this->_data[idx].j == j) return idx;
            return this->size();
        }
        // TODO: Binary search for larger .size() (N(log2(size)) instead of N(size) asymptotically)
    }

public:
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] size_type get_idx_of_ij(size_type i, size_type j) const {
        const size_type idx = this->_search_ij(i, j);
        // Return this->size() if {i, j} wasn't found. Throw with bound checking.
        if constexpr (self::params::checking == Checking::BOUNDS)
            if (idx == this->size())
                throw std::out_of_range(stringify("Index { ", i, ", ", j, "} in not a part of sparse matrix"));
        return idx;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] Index2D get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return Index2D{this->_data[idx].i, this->_data[idx].j};
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] bool contains_index(size_type i, size_type j) const noexcept {
        return this->_search_ij(i, j) != this->size();
    }

    // --- Reductions ---
    // ------------------
    utl_mvl_reqs(_has_binary_op_plus<value_type>::value)
    [[nodiscard]] value_type sum() const { return std::accumulate(this->cbegin(), this->cend(), value_type()); }

    utl_mvl_reqs(_has_binary_op_multiplies<value_type>::value)
    [[nodiscard]] value_type product() const {
        return std::accumulate(this->cbegin(), this->cend(), value_type(), std::multiplies<value_type>());
    }

    utl_mvl_reqs(_has_binary_op_less<value_type>::value)
    [[nodiscard]] value_type min() const { return *std::min_element(this->cbegin(), this->cend()); }

    utl_mvl_reqs(_has_binary_op_less<value_type>::value)
    [[nodiscard]] value_type max() const { return *std::max_element(this->cbegin(), this->cend()); }

    // --- Predicate operations ---
    // ----------------------------
    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference)> = true>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        for (size_type idx = 0; idx < this->size(); ++idx)
            if (predicate(this->operator[](idx), idx)) return true;
        return false;
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type)> = true>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        for (size_type idx = 0; idx < this->size(); ++idx)
            if (predicate(this->operator[](idx), idx)) return true;
        return false;
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        // Loop over all 2D indices using 1D loop with idx->ij conversion
        // This is just as fast and ensures looping only over existing elements in non-dense matrices
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            if (predicate(this->operator[](idx), ij.i, ij.j)) return true;
        }
        return false;
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference)> = true>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        auto inverse_predicate = [&](const_reference e) -> bool { return !predicate(e); };
        return !this->true_for_any(inverse_predicate);
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type)> = true>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        auto inverse_predicate = [&](const_reference e, size_type idx) -> bool { return !predicate(e, idx); };
        return !this->true_for_any(inverse_predicate);
    }

    template <class PredType, _has_signature_enable_if<PredType, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        // We can reuse .true_for_any() with inverted predicate due to following conjecture:
        // FOR_ALL (predicate)  ~  ! FOR_ANY (!predicate)
        auto inverse_predicate = [&](const_reference e, size_type i, size_type j) -> bool {
            return !predicate(e, i, j);
        };
        return !this->true_for_any(inverse_predicate);
    }

    // --- Const algorithms ---
    // ------------------------
    template <class FuncType, _has_signature_enable_if<FuncType, void(const_reference)> = true>
    const self& for_each(FuncType func) const {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx));
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(const_reference, size_type)> = true>
    const self& for_each(FuncType func) const {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx), idx);
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    const self& for_each(FuncType func) const {
        // Loop over all 2D indices using 1D loop with idx->ij conversion.
        // This is just as fast and ensures looping only over existing elements in non-dense matrices.
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            func(this->operator[](idx), ij.i, ij.j);
        }
        return *this;
    }

    // --- Mutating algorithms ---
    // ---------------------------
    template <class FuncType, _has_signature_enable_if<FuncType, void(reference)> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx));
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(reference, size_type)> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx), idx);
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, void(reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            func(this->operator[](idx), ij.i, ij.j);
        }
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(const_reference)> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem) { elem = func(elem); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(const_reference, size_type)> = true,
              utl_mvl_require(dimension == Dimension::VECTOR && ownership != Ownership::CONST_VIEW)>
    self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i) { elem = func(elem, i); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType,
              _has_signature_enable_if<FuncType, value_type(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
    self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i, size_type j) { elem = func(elem, i, j); };
        return this->for_each(func_wrapper);
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW)
    self& fill(const_reference value) {
        for (size_type idx = 0; idx < this->size(); ++idx) this->operator[](idx) = value;
        return *this;
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type()> = true,
              utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem) { elem = func(); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(size_type)> = true,
              utl_mvl_require(dimension == Dimension::VECTOR && ownership != Ownership::CONST_VIEW)>
    self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i) { elem = func(i); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _has_signature_enable_if<FuncType, value_type(size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
    self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i, size_type j) { elem = func(i, j); };
        return this->for_each(func_wrapper);
    }

    template <class Compare, utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& sort(Compare cmp) {
        std::sort(this->begin(), this->end(), cmp);
        return *this;
    }
    template <class Compare, utl_mvl_require(ownership != Ownership::CONST_VIEW)>
    self& stable_sort(Compare cmp) {
        std::stable_sort(this->begin(), this->end(), cmp);
        return *this;
    }

    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && _has_binary_op_less<value_type>::value)
    self& sort() {
        std::sort(this->begin(), this->end());
        return *this;
    }
    utl_mvl_reqs(ownership != Ownership::CONST_VIEW && _has_binary_op_less<value_type>::value)
    self& stable_sort() {
        std::stable_sort(this->begin(), this->end());
        return *this;
    }

    // --- Sparse Subviews ---
    // -----------------------

    // - Const views -
private:
    using _cref_triplet_array =
        std::vector<SparseEntry2D<std::reference_wrapper<const value_type>>>; // NOTE: Generalize for 1D

public:
    using sparse_const_view_type = GenericTensor<value_type, self::params::dimension, Type::SPARSE,
                                                 Ownership::CONST_VIEW, self::params::checking, Layout::SPARSE>;

    template <class UnaryPredicate, _has_signature_enable_if<UnaryPredicate, bool(const_reference)> = true>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        const auto forwarded_predicate = [&](const_reference elem, size_type, size_type) -> bool {
            return predicate(elem);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate, _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type)> = true>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        const auto forwarded_predicate = [&](const_reference elem, size_type i, size_type j) -> bool {
            const size_type idx = this->get_idx_of_ij(i, j);
            return predicate(elem, idx);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate,
              _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX)>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        // We can't preallocate triplets without scanning predicate through the whole matrix,
        // so we just push back entries into a vector and use to construct a sparse view
        _cref_triplet_array triplets;
        const auto          add_triplet_if_predicate = [&](const_reference elem, size_type i, size_type j) -> void {
            if (predicate(elem, i, j)) triplets.push_back({i, j, elem});
        };

        this->for_each(add_triplet_if_predicate);

        triplets.shrink_to_fit();
        return sparse_const_view_type(this->rows(), this->cols(), std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] sparse_const_view_type diagonal() const {
        // Sparse matrices have no better way of getting a diagonal than filtering (i ==j)
        if constexpr (self::params::type == Type::SPARSE) {
            return this->filter([](const_reference, size_type i, size_type j) { return i == j; });
        }
        // Non-sparse matrices can just iterate over diagonal directly
        else {
            const size_type     min_size = std::min(this->rows(), this->cols());
            _cref_triplet_array triplets;
            triplets.reserve(min_size);
            for (size_type k = 0; k < min_size; ++k) triplets.push_back({k, k, this->operator()(k, k)});
            return sparse_const_view_type(this->rows(), this->cols(), std::move(triplets));
        }
    }

    // - Mutable views -
private:
    using _ref_triplet_array =
        std::vector<SparseEntry2D<std::reference_wrapper<value_type>>>; // NOTE: Generalize for 1D

public:
    using sparse_view_type = GenericTensor<value_type, self::params::dimension, Type::SPARSE, Ownership::VIEW,
                                           self::params::checking, Layout::SPARSE>;

    template <class UnaryPredicate, utl_mvl_require(ownership != Ownership::CONST_VIEW),
              _has_signature_enable_if<UnaryPredicate, bool(const_reference)> = true>
    [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        const auto forwarded_predicate = [&](const_reference elem, size_type, size_type) -> bool {
            return predicate(elem);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate, utl_mvl_require(ownership != Ownership::CONST_VIEW),
              _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type)> = true>
    [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        const auto forwarded_predicate = [&](const_reference elem, size_type i, size_type j) -> bool {
            const size_type idx = this->get_idx_of_ij(i, j);
            return predicate(elem, idx);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <class UnaryPredicate,
              _has_signature_enable_if<UnaryPredicate, bool(const_reference, size_type, size_type)> = true,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
    [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        // This method implements actual filtering, others just forward predicates to it
        _ref_triplet_array triplets;
        // We can't preallocate triplets without scanning predicate through the whole matrix,
        // so we just push back entries into a vector and use to construct a sparse view
        const auto         add_triplet_if_predicate = [&](reference elem, size_type i, size_type j) -> void {
            if (predicate(elem, i, j)) triplets.push_back({i, j, elem});
        };

        this->for_each(add_triplet_if_predicate);

        triplets.shrink_to_fit();
        return sparse_view_type(this->rows(), this->cols(), std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] sparse_view_type diagonal() {
        /* Sparse matrices have no better way of getting a diagonal than filtering (i == j) */
        if constexpr (self::params::type == Type::SPARSE) {
            return this->filter([](const_reference, size_type i, size_type j) { return i == j; });
        } /* Non-sparse matrices can just iterate over diagonal directly */
        else {
            const size_type    min_size = std::min(this->rows(), this->cols());
            _ref_triplet_array triplets;
            triplets.reserve(min_size);
            for (size_type k = 0; k < min_size; ++k) triplets.push_back({k, k, this->operator()(k, k)});
            return sparse_view_type(this->rows(), this->cols(), std::move(triplets));
        }
    }

    // --- Block Subviews ---
    // ----------------------
public:
    // - Const views -
    using block_const_view_type =
        std::conditional_t<self::params::type == Type::SPARSE, sparse_const_view_type,
                           GenericTensor<value_type, self::params::dimension, Type::STRIDED, Ownership::CONST_VIEW,
                                         self::params::checking, self::params::layout>>;

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] block_const_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                              size_type block_cols) const {
        // Sparse matrices have no better way of getting a block than filtering by { i, j }

        // Do the same thing as in .filter(), but shrink resulting view size to
        // { block_rows, block_cols } and shift indexation by { block_i, block_j }
        _cref_triplet_array triplets;

        const auto add_triplet_if_inside_block = [&](const_reference elem, size_type i, size_type j) -> void {
            if ((block_i <= i) && (i < block_i + block_rows) && (block_j <= j) && (j < block_j + block_cols))
                triplets.push_back({i - block_i, j - block_j, elem});
        };

        this->for_each(add_triplet_if_inside_block);

        triplets.shrink_to_fit();
        return block_const_view_type(block_rows, block_cols, std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type != Type::SPARSE)
    [[nodiscard]] block_const_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                              size_type block_cols) const {
        if constexpr (self::params::layout == Layout::RC) {
            const size_type row_stride = this->row_stride() + this->col_stride() * (this->cols() - block_cols);
            const size_type col_stride = this->col_stride();
            return block_const_view_type(block_rows, block_cols, row_stride, col_stride,
                                         &this->operator()(block_i, block_j));
        }
        if constexpr (self::params::layout == Layout::CR) {
            const size_type row_stride = this->row_stride();
            const size_type col_stride = this->col_stride() + this->row_stride() * (this->rows() - block_rows);
            return block_const_view_type(block_rows, block_cols, row_stride, col_stride,
                                         &this->operator()(block_i, block_j));
        }
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] block_const_view_type row(size_type i) const { return this->block(i, 0, 1, this->cols()); }

    utl_mvl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] block_const_view_type col(size_type j) const { return this->block(0, j, this->rows(), 1); }

    // - Mutable views -
    using block_view_type =
        std::conditional_t<self::params::type == Type::SPARSE, sparse_view_type,
                           GenericTensor<value_type, self::params::dimension, Type::STRIDED, Ownership::VIEW,
                                         self::params::checking, self::params::layout>>;

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                        size_type block_cols) {
        // Sparse matrices have no better way of getting a block than filtering by { i, j }

        // Do the same thing as in .filter(), but shrink resulting view size to
        // { block_rows, block_cols } and shift indexation by { block_i, block_j }
        _ref_triplet_array triplets;

        const auto add_triplet_if_inside_block = [&](reference elem, size_type i, size_type j) -> void {
            if ((block_i <= i) && (i < block_i + block_rows) && (block_j <= j) && (j < block_j + block_cols))
                triplets.push_back({i - block_i, j - block_j, elem});
        };

        this->for_each(add_triplet_if_inside_block);

        triplets.shrink_to_fit();
        return block_view_type(block_rows, block_cols, std::move(triplets));
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type != Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                        size_type block_cols) {
        if constexpr (self::params::layout == Layout::RC) {
            const size_type row_stride = this->row_stride() + this->col_stride() * (this->cols() - block_cols);
            const size_type col_stride = this->col_stride();
            return block_view_type(block_rows, block_cols, row_stride, col_stride, &this->operator()(block_i, block_j));
        }
        if constexpr (self::params::layout == Layout::CR) {
            const size_type row_stride = this->row_stride();
            const size_type col_stride = this->col_stride() + this->row_stride() * (this->rows() - block_rows);
            return block_view_type(block_rows, block_cols, row_stride, col_stride, &this->operator()(block_i, block_j));
        }
        _unreachable();
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type row(size_type i) { return this->block(i, 0, 1, this->cols()); }

    utl_mvl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type col(size_type j) { return this->block(0, j, this->rows(), 1); }

    // --- Sparse operations ---
    // -------------------------

private:
    using _triplet_t = _choose_based_on_ownership<_ownership, SparseEntry2D<value_type>,
                                                  SparseEntry2D<std::reference_wrapper<value_type>>,
                                                  SparseEntry2D<std::reference_wrapper<const value_type>>>;

public:
    using sparse_entry_type = _triplet_t;

    utl_mvl_reqs(type == Type::SPARSE)
    [[nodiscard]] const std::vector<sparse_entry_type>& entries() const noexcept { return this->_data; }

    utl_mvl_reqs(type == Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] std::vector<sparse_entry_type>& entries() noexcept { return this->_data; }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& insert_triplets(const std::vector<sparse_entry_type>& triplets) {
        // Bulk-insert triplets and sort by index
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };

        this->_data.insert(this->_data.end(), triplets.begin(), triplets.end());
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& rewrite_triplets(std::vector<sparse_entry_type>&& triplets) {
        // Move-construct all triplets at once and sort by index
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };

        this->_data = std::move(triplets);
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& erase_triplets(std::vector<Index2D> indices) {
        // Erase triplets with {i, j} from 'indices' using the fact that both
        // 'indices' and triplets are sorted. We can scan through triplets once
        // while advancing 'cursor' when 'indices[cursor]' gets deleted, which
        // result in all necessary triplets being marked for erasure in order.
        std::sort(indices.begin(), indices.end());
        std::size_t cursor = 0;

        const auto erase_condition = [&](const sparse_entry_type& triplet) -> bool {
            /* Stop erasing once all target indices are handled */
            if (cursor == indices.size()) return false;
            if (indices[cursor].i == triplet.i && indices[cursor].j == triplet.j) {
                ++cursor;
                return true;
            }
            return false;
        };

        const auto iter = std::remove_if(this->_data.begin(), this->_data.end(), erase_condition);
        this->_data.erase(iter, this->_data.end());

        // Re-sort triplets just in case
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    // --- Constructors ---
    // --------------------

    // - Matrix -
public:
    // Rule of five:
    // copy-ctor       - [+] (deduced from copy-assignment)
    // move-ctor       - [+] (= default)
    // copy-assignment - [+] (custom for dense/strided, same as default for sparse)
    // move-assignment - [+] (= default)
    // destructor      - [+] (= default)

    // Move-ctor is default for all types
    GenericTensor(self&& other) noexcept = default;

    // Move-assignment is default for all types
    self& operator=(self&& other) noexcept = default;

    // Copy-ctor is deduced from assignment operator
    GenericTensor(const self& other) { *this = other; }

    // Copy-assignment
    self& operator=(const self& other) {
        // Note: copy-assignment operator CANNOT be templated, it has to be implemented with 'if constexpr'
        this->_rows = other.rows();
        this->_cols = other.cols();
        if constexpr (self::params::type == Type::DENSE) {
            this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
            std::copy(other.begin(), other.end(), this->begin());
        }
        if constexpr (self::params::type == Type::STRIDED) {
            this->_row_stride = other.row_stride();
            this->_col_stride = other.col_stride();
            this->_data       = std::move(_make_unique_ptr_array<value_type>(this->size()));
            std::copy(other.begin(), other.end(), this->begin());
        }
        if constexpr (self::params::type == Type::SPARSE) { this->_data = other._data; }
        return *this;
    }

    // Default-ctor (containers)
    utl_mvl_reqs(ownership == Ownership::CONTAINER)
    GenericTensor() noexcept {}

    // Default-ctor (views)
    utl_mvl_reqs(ownership != Ownership::CONTAINER)
    GenericTensor() noexcept = delete;

    // Copy-assignment over the config boundaries
    // We can change checking config, copy from matrices with different layouts,
    // copy from views and even matrices of other types
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                              ownership == Ownership::CONTAINER)>
    self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                        other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(value_type());
        other.for_each([&](const value_type& element, size_type i, size_type j) { this->operator()(i, j) = element; });
        return *this;
        // copying from sparse to dense works, all elements that weren't in the sparse matrix remain default-initialized
    }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                              ownership == Ownership::CONTAINER)>
    self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                        other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(value_type());
        // Not quite sure whether swapping strides when changing layouts like this is okay,
        // but it seems to be correct
        if constexpr (self::params::layout != other_layout) std::swap(this->_row_stride, this->_cols_stride);
        other.for_each([&](const value_type& element, size_type i, size_type j) { this->operator()(i, j) = element; });
        return *this;
        // copying from sparse to strided works, all elements that weren't in the sparse matrix remain
        // default-initialized
    }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::SPARSE &&
                              ownership == Ownership::CONTAINER)>
    self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                        other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        std::vector<sparse_entry_type> triplets;

        // Other sparse matrices can be trivially copied
        if constexpr (other_type == Type::SPARSE) {
            triplets.reserve(other.size());
            other.for_each([&](const value_type& elem, size_type i, size_type j) { triplets.push_back({i, j, elem}); });
        }
        // Non-sparse matrices are filtered by non-default-initialized-elements to construct a sparse subset
        else {
            other.for_each([&](const_reference elem, size_type i, size_type j) {
                if (elem != value_type()) triplets.push_back({i, j, elem});
            });
        }

        this->rewrite_triplets(std::move(triplets));
        return *this;
    }

    // Copy-ctor over the config boundaries (deduced from assignment over config boundaries)
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership == Ownership::CONTAINER)>
    GenericTensor(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership, other_checking,
                                      other_layout>& other) {
        *this = other;
    }

    // Move-assignment over config boundaries
    // Note: Unlike copying, we can't change layout, only checking config
    // Also 'other' can no longer be a view or have a different type
    template <Checking other_checking, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                                                       ownership == Ownership::CONTAINER)>
    self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type, self::params::ownership,
                                  other_checking, self::params::layout>&& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(other._data);
        return *this;
    }

    template <Checking other_checking, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                                                       ownership == Ownership::CONTAINER)>
    self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type, self::params::ownership,
                                  other_checking, self::params::layout>&& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = std::move(other._data);
        return *this;
    }

    template <Checking other_checking, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::SPARSE &&
                                                       ownership == Ownership::CONTAINER)>
    self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type, self::params::ownership,
                                  other_checking, self::params::layout>&& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(other._data);
        return *this;
    }

    // Move-ctor over the config boundaries (deduced from move-assignment over config boundaries)
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && ownership == Ownership::CONTAINER)>
    GenericTensor(GenericTensor<value_type, self::params::dimension, other_type, other_ownership, other_checking,
                                other_layout>&& other) {
        *this = std::move(other);
    }

    // Init-with-value
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, const_reference value = value_type()) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(value);
    }

    // Init-with-lambda
    template <class FuncType, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                                              ownership == Ownership::CONTAINER)>
    explicit GenericTensor(size_type rows, size_type cols, FuncType init_func) {
        // .fill() already takes care of preventing improper values of 'FuncType', no need to do the check here
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));
        this->fill(init_func);
    }

    // Init-with-ilist
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    GenericTensor(std::initializer_list<std::initializer_list<value_type>> init) {
        this->_rows = init.size();
        this->_cols = (*init.begin()).size();
        this->_data = std::move(_make_unique_ptr_array<value_type>(this->size()));

        // Check dimensions (throw if cols have different dimensions)
        for (auto row_it = init.begin(); row_it < init.end(); ++row_it)
            if (static_cast<size_type>((*row_it).end() - (*row_it).begin()) != this->cols())
                throw std::invalid_argument("Initializer list dimensions don't match.");

        // Copy elements
        for (size_type i = 0; i < this->rows(); ++i)
            for (size_type j = 0; j < this->cols(); ++j) this->operator()(i, j) = (init.begin()[i]).begin()[j];
    }

    // Init-with-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, pointer data_ptr) noexcept {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(decltype(this->_data)(data_ptr));
    }

    // - Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::VIEW)
    explicit GenericTensor(size_type rows, size_type cols, pointer data_ptr) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::VIEW)>
    GenericTensor(GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = other.data();
    }

    // - Const Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONST_VIEW)
    explicit GenericTensor(size_type rows, size_type cols, const_pointer data_ptr) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::DENSE &&
                              ownership == Ownership::CONST_VIEW)>
    GenericTensor(const GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                      other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = other.data();
    }

    // - Strided Matrix -

    // Init-with-value
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           const_reference value = value_type()) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->_total_allocated_size()));
        this->fill(value);
    }

    // Init-with-lambda
    template <class FuncType, utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                                              ownership == Ownership::CONTAINER)>
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           FuncType init_func) {
        // .fill() already takes care of preventing improper values of 'FuncType', no need to do the check here
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->_total_allocated_size()));
        this->fill(init_func);
    }

    // Init-with-ilist
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    GenericTensor(std::initializer_list<std::initializer_list<value_type>> init, size_type row_stride,
                  size_type col_stride) {
        this->_rows       = init.size();
        this->_cols       = (*init.begin()).size();
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(_make_unique_ptr_array<value_type>(this->_total_allocated_size()));

        // Check dimensions (throw if cols have different dimensions)
        for (auto row_it = init.begin(); row_it < init.end(); ++row_it)
            if ((*row_it).end() - (*row_it).begin() != this->_cols)
                throw std::invalid_argument("Initializer list dimensions don't match.");

        // Copy elements
        for (size_type i = 0; i < this->rows(); ++i)
            for (size_type j = 0; j < this->cols(); ++j) this->operator()(i, j) = (init.begin()[i]).begin()[j];
    }

    // Init-with-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           pointer data_ptr) noexcept {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = std::move(decltype(this->_data)(data_ptr));
    }

    // - Strided Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::VIEW)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           pointer data_ptr) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::VIEW)>
    GenericTensor(GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = other.data();
    }

    // - Const Strided Matrix View -

    // Init-from-data
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONST_VIEW)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           const_pointer data_ptr) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              utl_mvl_require(dimension == Dimension::MATRIX && type == Type::STRIDED &&
                              ownership == Ownership::CONST_VIEW)>
    GenericTensor(const GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                      other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = other.data();
    }

    // - Sparse Matrix / Sparse Matrix View / Sparse Matrix Const View -

    // Init-from-data (copy)
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    explicit GenericTensor(size_type rows, size_type cols, const std::vector<sparse_entry_type>& data) {
        this->_rows = rows;
        this->_cols = cols;
        this->insert_triplets(std::move(data));
    }

    // Init-from-data (move)
    utl_mvl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    explicit GenericTensor(size_type rows, size_type cols, std::vector<sparse_entry_type>&& data) {
        this->_rows = rows;
        this->_cols = cols;
        this->rewrite_triplets(std::move(data));
    }
};

// ===========================
// --- Predefined Typedefs ---
// ===========================

constexpr auto _default_checking        = Checking::NONE;
constexpr auto _default_layout_dense_2d = Layout::RC;

// - Dense 2D -
template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using Matrix = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using MatrixView = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::VIEW, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using ConstMatrixView = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::CONST_VIEW, checking, layout>;

// - Strided 2D -
template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using StridedMatrix = GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::CONTAINER, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using StridedMatrixView = GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::VIEW, checking, layout>;

template <class T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using ConstStridedMatrixView =
    GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::CONST_VIEW, checking, layout>;

// - Sparse 2D -
template <class T, Checking checking = _default_checking>
using SparseMatrix = GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::CONTAINER, checking, Layout::SPARSE>;

template <class T, Checking checking = _default_checking>
using SparseMatrixView = GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::VIEW, checking, Layout::SPARSE>;

template <class T, Checking checking = _default_checking>
using ConstSparseMatrixView =
    GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::CONST_VIEW, checking, Layout::SPARSE>;

// ==================
// --- Formatters ---
// ==================

namespace format {

// --- Implementation ---
// ----------------------

// The "header" of all human-readable formats that displays
// some meta info with tensor type and dimensions.
template <utl_mvl_tensor_arg_defs>
[[nodiscard]] std::string _tensor_meta_string(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor) {
    std::string buffer;

    if constexpr (_type == Type::DENSE) buffer += "Dense";
    if constexpr (_type == Type::STRIDED) buffer += "Strided";
    if constexpr (_type == Type::SPARSE) buffer += "Sparse";
    if constexpr (_dimension == Dimension::VECTOR) buffer += stringify(" vector [size = ", tensor.size(), "]:\n");
    if constexpr (_dimension == Dimension::MATRIX)
        buffer += stringify(" matrix [size = ", tensor.size(), "] (", tensor.rows(), " x ", tensor.cols(), "):\n");

    return buffer;
}

// Human-readable formats automatically collapse matrices
// that are too  large to be reasonably parsed by a human
constexpr std::size_t _max_displayed_flat_size = 30 * 30;

template <utl_mvl_tensor_arg_defs>
[[nodiscard]] std::string _as_too_large(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor) {
    return stringify(_tensor_meta_string(tensor), "  <hidden due to large size>\n");
}

// Generic method to do "dense matrix print" with given delimiters.
// Cuts down on repetition since a lot of formats only differ in the delimiters used.
template <class T, Type type, Ownership ownership, Checking checking, Layout layout, class Func>
[[nodiscard]] std::string
_generic_dense_format(const GenericTensor<T, Dimension::MATRIX, type, ownership, checking, layout>& tensor,     //
                      std::string_view                                                              begin,      //
                      std::string_view                                                              row_begin,  //
                      std::string_view                                                              col_delim,  //
                      std::string_view                                                              row_end,    //
                      std::string_view                                                              row_delim,  //
                      std::string_view                                                              end,        //
                      Func                                                                          stringifier //
) {
    if (tensor.empty()) return (std::string() += begin) += end;

    Matrix<std::string> strings(tensor.rows(), tensor.cols());

    // Stringify
    if constexpr (type == Type::SPARSE) strings.fill("-");
    tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) { strings(i, j) = stringifier(elem); });
    // this takes care of sparsity, if the matrix is sparse we prefill 'strings' with "-" and then fill appropriate
    // {i, j} with actual stringified values from the tensor. For dense matrices no unnecessary work is done.

    // Get column widths - we want matrix to format nice and aligned
    std::vector<std::size_t> column_widths(strings.cols(), 0);
    for (std::size_t i = 0; i < strings.rows(); ++i)
        for (std::size_t j = 0; j < strings.cols(); ++j)
            column_widths[j] = std::max(column_widths[j], strings(i, j).size());

    // Format with proper alignment
    std::string buffer(begin);
    for (std::size_t i = 0; i < strings.rows(); ++i) {
        buffer += row_begin;
        for (std::size_t j = 0; j < strings.cols(); ++j) {
            if (strings(i, j).size() < column_widths[j]) buffer.append(column_widths[j] - strings(i, j).size(), ' ');
            buffer += strings(i, j);
            if (j + 1 < strings.cols()) buffer += col_delim;
        }
        buffer += row_end;
        if (i + 1 < strings.rows()) buffer += row_delim;
    }
    buffer += end;

    return buffer;
}

// --- Human-readable formats ---
// ------------------------------

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_vector(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    if (tensor.size() > _max_displayed_flat_size) return _as_too_large(tensor);

    std::string buffer = _tensor_meta_string(tensor);

    buffer += "  { ";
    for (std::size_t idx = 0; idx < tensor.size(); ++idx) {
        buffer += stringifier(tensor[idx]);
        if (idx + 1 < tensor.size()) buffer += ", ";
    }
    buffer += " }\n";

    return buffer;
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_dictionary(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor,
                                        Func                                          stringifier = Func()) {
    if (tensor.size() > _max_displayed_flat_size) return _as_too_large(tensor);

    std::string buffer = _tensor_meta_string(tensor);

    if constexpr (_dimension == Dimension::MATRIX) {
        tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) {
            buffer += stringify("(", i, ", ", j, ") = ");
            buffer += stringifier(elem);
            buffer += '\n';
        });
    } else {
        tensor.for_each([&](const T& elem, std::size_t i) {
            buffer += stringify("(", i, ") = ");
            buffer += stringifier(elem);
            buffer += '\n';
        });
    }

    return buffer;
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_matrix(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    if (tensor.size() > _max_displayed_flat_size) return _as_too_large(tensor);

    return _generic_dense_format(tensor, _tensor_meta_string(tensor), "  [ ", " ", " ]\n", "", "", stringifier);
}

// --- Export formats ---
// ----------------------

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_raw(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "", "", " ", "\n", "", "", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_csv(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "", "", ", ", "\n", "", "", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_json(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "[\n", "    [ ", ", ", " ]", ",\n", "\n]\n", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_mathematica(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor,
                                         Func                                          stringifier = Func()) {
    return _generic_dense_format(tensor, "{\n", "    { ", ", ", " }", ",\n", "\n}\n", stringifier);
}

template <utl_mvl_tensor_arg_defs, class Func = default_stringifier<T>>
[[nodiscard]] std::string as_latex(const GenericTensor<utl_mvl_tensor_arg_vals>& tensor, Func stringifier = Func()) {
    return _generic_dense_format(tensor, "\\begin{pmatrix}\n", "  ", " & ", " \\\\\n", "", "\\end{pmatrix}\n",
                                 stringifier);
}


} // namespace format

// ================================
// --- Linear algebra operators ---
// ================================

// --- Unary operator implementation ----
// --------------------------------------

template <class L, class Op,                                                    //
          _is_tensor_enable_if<L> = true,                                       //
          class return_type       = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_unary_op(L&& left, Op&& op) {
    using reference = typename std::decay_t<L>::reference;

    // Reuse r-value if possible
    return_type res = std::forward<L>(left);

    // '.for_each()' takes care of possible sparsity
    res.for_each([&](reference elem) { elem = op(std::move(elem)); });

    return res;
}

// --- Unary operator API ----
// ---------------------------

// std <functional> has function objects for every operator possible,
// except unary '+' for some reason, which is why we have to implement it ourselves.
// This is the exact same thing as 'std::negate<>' but for operator '+'.
template <class T>
struct _unary_plus_functor {
    constexpr T operator()(const T& lhs) const { return +lhs; }
};

template <class L, _is_tensor_enable_if<L> = true, class value_type = typename std::decay_t<L>::value_type,
          _has_unary_op_plus_enable_if<value_type> = true>
auto operator+(L&& left) {
    return apply_unary_op(std::forward<L>(left), _unary_plus_functor<value_type>());
}

template <class L, _is_tensor_enable_if<L> = true, class value_type = typename std::decay_t<L>::value_type,
          _has_unary_op_minus_enable_if<value_type> = true>
auto operator-(L&& left) {
    return apply_unary_op(std::forward<L>(left), std::negate<value_type>());
}

// --- Binary operator implementation ---
// --------------------------------------

// Doing things "in a dumb but simple way" would be to just have all operators take arguments as const-refs and
// return a copy, however we can speed things up a lot by properly using perfect forwarding, which would reuse
// r-values if possible to avoid allocation. Doing so would effectively change something like this:
//    res = A + B - C - D + E
// from 5 (!) copies to only 1, since first operator will create an r-value that gets propagated and reused by
// all others. This however introduces it's own set of challenges since instead of traditional overloading we
// now have to resort to SFINAE-driven imitation of it and carefully watch what we restrict.
//
// So here are the key points of implementing all of this:
//
//    1. All unary and binary operators ('-', '+', '*' and etc.) can benefit from reusing r-values and moving
//       one of the arguments into the result rather than copy. This can only happen to CONTAINER args that
//       correspond to the return type.
//
//    2. Binary operators have following return types for different tensors:
//       - (1)  dense +  dense =>  dense      (complexity O(N^2))
//       - (2)  dense + sparse =>  dense      (complexity O(N)  )
//       - (3) sparse +  dense =>  dense      (complexity O(N)  )
//       - (4) sparse + sparse => sparse      (complexity O(N)  )
//       return types inherit option arguments from the lhs and always have 'ownership == CONTAINER',
//       while argument can be anything, including views. To implement the list above efficiently there
//       is no other way but to make 4 separate implementation, tailored for each level of sparsity.
//
//    3. All binary operators can be written in a generic form like 'apply_binary_operator(A, B, op)'
//       and specialized with operator functors froms std <functional>. All we need for individual
//       operators is to figure out correct boilerplate with perfect forwarding. "Overload resolution"
//       will be performed by the 'apply_operator' method.
//
// It's a good question whether to threat binary '*' as element-wise product (which would be in line with other
// operators) or a matrix product, but in the end it seems doing it element-wise would be too confusing for the
// user, so we leave '*' as a matrix product and declare element-wise product as a function 'elementwise_product()'
// since no existing operators seem suitable for such overload.

// (1)  dense +  dense =>  dense
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_nonsparse_tensor_enable_if<L>                 = true,                                       //
          _is_nonsparse_tensor_enable_if<R>                 = true,                                       //
          class return_type                                 = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using reference = typename std::decay_t<L>::reference;
    using size_type = typename std::decay_t<L>::size_type;

    return_type res;

    // Reuse r-value arguments if possible while preserving the order of operations
    if constexpr (std::is_rvalue_reference_v<L> && std::decay_t<L>::params::ownership == Ownership::CONTAINER) {
        res = std::forward<L>(left);
        res.for_each([&](reference elem, size_type i, size_type j) { elem = op(std::move(elem), right(i, j)); });
    } else if constexpr (std::is_rvalue_reference_v<R> && std::decay_t<R>::params::ownership == Ownership::CONTAINER) {
        res = std::forward<R>(right);
        res.for_each([&](reference elem, size_type i, size_type j) { elem = op(left(i, j), std::move(elem)); });
    } else {
        res = return_type(left.rows(), left.cols());
        res.for_each([&](reference elem, size_type i, size_type j) { elem = op(left(i, j), right(i, j)); });
    }

    return res;
}

// (2)  dense + sparse =>  dense
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_nonsparse_tensor_enable_if<L>                 = true,                                       //
          _is_sparse_tensor_enable_if<R>                    = true,                                       //
          class return_type                                 = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using reference = typename std::decay_t<L>::reference;
    using size_type = typename std::decay_t<L>::size_type;

    // Reuse r-value if possible
    return_type res = std::forward<L>(left);

    // '.for_each()' to only iterate sparse elements
    right.for_each([&](reference elem, size_type i, size_type j) { res(i, j) = op(std::move(res(i, j)), elem); });

    return res;
}

// (3) sparse +  dense =>  dense
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_sparse_tensor_enable_if<L>                    = true,                                       //
          _is_nonsparse_tensor_enable_if<R>                 = true,                                       //
          class return_type                                 = typename std::decay_t<R>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using reference = typename std::decay_t<R>::reference;
    using size_type = typename std::decay_t<R>::size_type;

    // Reuse r-value if possible
    return_type res = std::forward<R>(right);

    // '.for_each()' to only iterate sparse elements
    left.for_each([&](reference elem, size_type i, size_type j) { res(i, j) = op(elem, std::move(res(i, j))); });

    return res;
}

// (4) sparse + sparse => sparse
template <class L, class R, class Op,                                                                     //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                       //
          _is_sparse_tensor_enable_if<L>                    = true,                                       //
          _is_sparse_tensor_enable_if<R>                    = true,                                       //
          class return_type                                 = typename std::decay_t<L>::owning_reflection //
          >
return_type apply_binary_op(L&& left, R&& right, Op&& op) {
    utl_mvl_assert(left.rows() == right.rows());
    utl_mvl_assert(left.cols() == right.cols());

    using sparse_entry = typename std::decay_t<L>::sparse_entry_type;
    using value_type   = typename std::decay_t<L>::value_type;

    std::vector<sparse_entry> res_triplets;
    res_triplets.reserve(std::max(left.size(), right.size()));
    // not enough when matrices have different sparsity patterns, but good enough for initial guess

    std::size_t i = 0, j = 0;

    // Merge sparsity patterns
    while (i < left.size() && j < right.size()) {
        // Entry present only in lhs sparsity pattern
        if (left._data[i] < right._data[j]) {
            res_triplets.emplace_back(
                _apply_binary_op_to_sparse_entry_and_value(std::forward<L>(left)._data[i++], value_type{}, op));
        }
        // Entry present only in rhs sparsity pattern
        else if (left._data[i] > right._data[j]) {
            res_triplets.emplace_back(
                _apply_binary_op_to_value_and_sparse_entry(value_type{}, std::forward<R>(right)._data[j++], op));
        }
        // Entry present in both sparsity patterns
        else {
            res_triplets.emplace_back(_apply_binary_op_to_sparse_entries(std::forward<L>(left)._data[i++],
                                                                         std::forward<R>(right)._data[j++], op));
        }
    }
    // Copy the rest of lhs (if needed)
    while (i < left.size()) res_triplets.emplace_back(std::forward<L>(left)._data[i++]);
    // Copy the rest of rhs (if needed)
    while (j < right.size()) res_triplets.emplace_back(std::forward<R>(right)._data[j++]);

    return return_type(left.rows(), left.cols(), std::move(res_triplets));
}

// --- Binary operator API ---
// ---------------------------

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type = typename std::decay_t<L>::value_type, _has_binary_op_plus_enable_if<value_type> = true>
auto operator+(L&& left, R&& right) {
    return apply_binary_op(std::forward<L>(left), std::forward<R>(right), std::plus<value_type>());
}

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type = typename std::decay_t<L>::value_type, _has_binary_op_minus_enable_if<value_type> = true>
auto operator-(L&& left, R&& right) {
    return apply_binary_op(std::forward<L>(left), std::forward<R>(right), std::minus<value_type>());
}

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type                                = typename std::decay_t<L>::value_type,
          _has_binary_op_multiplies_enable_if<value_type> = true>
auto elementwise_product(L&& left, R&& right) {
    return apply_binary_op(std::forward<L>(left), std::forward<R>(right), std::multiplies<value_type>());
}

// --- Augmented assignment operator API ---
// -----------------------------------------

// These can just reuse corresponding binary operators while 'std::move()'ing lhs to avoid copying.

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type = typename std::decay_t<L>::value_type, _has_assignment_op_plus_enable_if<value_type> = true>
L& operator+=(L&& left, R&& right) {
    return (left = std::move(left) + right);
}

template <class L, class R, _are_tensors_with_same_value_type_enable_if<L, R> = true,
          class value_type                               = typename std::decay_t<L>::value_type,
          _has_assignment_op_minus_enable_if<value_type> = true>
L& operator-=(L&& left, R&& right) {
    return (left = std::move(left) - right);
}

// --- Matrix multiplication ---
// -----------------------------

// Just like with binary operators, we run into the need to have 4 different implementations:
//    - (1)  dense +  dense =>  dense      (complexity O(N^3))
//    - (2)  dense + sparse =>  dense      (complexity O(N^2))
//    - (3) sparse +  dense =>  dense      (complexity O(N^2))
//    - (4) sparse + sparse => sparse      (complexity O(N)  )
//
// Would be gread to implement proper "smart" GEMM and CRS-multiplication for sparse matrices, however that adds
// an absolutely huge amount of complexity for x1.5-x3 speedup on matrix multiplication, which is already kind of
// an afterthought provided here mainly for convenience and not for actual number crunching usage.
//
// We do however use some basic optimizations like loop reordering / temporaries / blocking to have a sensible baseline,
// we properly account for sparsity which brings multiplication with sparse matrices from O(N^3) down to O(N^2) / O(N).

// (1)  dense +  dense =>  dense
//
// From benchmarks 1D blocking over "k" with a decently large block size seems to be more reliable than 2D/3D blocking.
// When measuring varying matrix sizes and datatypes, for matrices with less than ~1k rows/cols (aka matrices that fully
// fit into L2 cache) 1D blocking doesn't  seem to have much of an effect (positive or negative), while 2D/3D has often
// lead to a slowdown for specific matrix  sizes. At large enough sizes blocking leads to a noticeable (~2x) speedup.
// Note that this is all very hardware-specific, so it's difficult to make a truly generic judgement. In general,
// blocking seems to be worth it.
//
// Note that unlike other binary operators, here there is no possible benefit in r-value reuse.
//
template <class L, class R,                                                                                //
          _are_tensors_with_same_value_type_enable_if<L, R> = true,                                        //
          _is_nonsparse_tensor_enable_if<L>                 = true,                                        //
          _is_nonsparse_tensor_enable_if<R>                 = true,                                        //
          class value_type                                  = typename std::decay_t<L>::value_type,        //
          class return_type                                 = typename std::decay_t<L>::owning_reflection, //
          _has_binary_op_multiplies_enable_if<value_type>   = true,                                        //
          _has_assignment_op_plus_enable_if<value_type>     = true                                         //
          >
return_type operator*(const L& left, const R& right) {
    utl_mvl_assert(left.cols() == right.rows());

    using size_type = typename std::decay_t<L>::size_type;

    const size_type N_i = left.rows(), N_k = left.cols(), N_j = right.cols();
    // (N_i)x(N_k) * (N_k)x(N_j) => (N_i)x(N_j)

    constexpr size_type block_size_kk = 32;

    return_type res(N_i, N_j, value_type{});

    for (size_type kk = 0; kk < N_k; kk += block_size_kk) {
        const size_type k_extent = std::min(N_k, kk + block_size_kk);
        // needed for matrices that aren't a multiple of block size
        for (size_type i = 0; i < N_i; ++i) {
            for (size_type k = kk; k < k_extent; ++k) {
                const auto& r = left(i, k);
                for (size_type j = 0; j < N_j; ++j) res(i, j) += r * right(k, j);
            }
        }
    }

    return res;
}

// (2)  dense + sparse =>  dense

// TODO:

// (3) sparse +  dense =>  dense

// TODO:

// (4) sparse + sparse => sparse

// TODO:

// Clear out internal macros
#undef utl_mvl_tensor_arg_defs
#undef utl_mvl_tensor_arg_vals
#undef utl_mvl_require
#undef utl_mvl_reqs

} // namespace utl::mvl

#endif
#endif // module utl::mvl
