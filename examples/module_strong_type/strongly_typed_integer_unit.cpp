#include "include/UTL/strong_type.hpp"

int main() {
    using ByteOffset = utl::strong_type::Arithmetic<int, struct OffsetTag>;
    
    constexpr ByteOffset buffer_start  = 0;
    constexpr ByteOffset buffer_stride = 3;
    
    // Perform arithmetics
    static_assert(buffer_start + buffer_stride == ByteOffset{3});
    static_assert(           2 * buffer_stride == ByteOffset{6});
    
    // Extract value
    static_assert(buffer_stride.get() == 3);
    
    // Explicit cast
    static_assert(static_cast<int>(buffer_stride) == 3);
    
    // Compile time protection
    constexpr int        element_count = 70;
    constexpr ByteOffset buffer_end    = buffer_start + element_count * buffer_stride;
    
    // > constexpr ByteOffset buffer_end = buffer_start + element_count;
    //   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //   forgot to multiply by stride, will not compile
    
    static_assert(buffer_end == ByteOffset{0 + 3 * 70});
}