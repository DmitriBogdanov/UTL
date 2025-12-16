#include "include/UTL/strong_type.hpp"

int main() {
    using offset_type = utl::strong_type::arithmetic<int, struct offset_tag>;
    
    constexpr offset_type buffer_start  = 0;
    constexpr offset_type buffer_stride = 3;
    
    // Perform arithmetics
    static_assert(buffer_start + buffer_stride == offset_type{3});
    static_assert(           2 * buffer_stride == offset_type{6});
    
    // Extract value
    static_assert(buffer_stride.get() == 3);
    
    // Explicit cast
    static_assert(static_cast<int>(buffer_stride) == 3);
    
    // Compile time protection
    constexpr int         element_count = 70;
    constexpr offset_type buffer_end    = buffer_start + element_count * buffer_stride;
    
    // > constexpr offset_type buffer_end = buffer_start + element_count;
    //   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //   forgot to multiply by stride, will not compile
    
    static_assert(buffer_end == offset_type{0 + 3 * 70});
}