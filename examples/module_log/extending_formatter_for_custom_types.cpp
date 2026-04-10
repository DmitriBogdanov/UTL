#include "include/UTL/log.hpp"

using namespace utl;

// Custom type
struct vec3 { double x, y, z; };

// Extend formatter to support 'vec3'
template <>
struct log::formatter<vec3> {
    template <class Buffer>
    void operator()(Buffer& buffer, const vec3& vec) {
        formatter<const char*>{}(buffer, "vec3{");
        formatter<     double>{}(buffer, vec.x  );
        formatter<const char*>{}(buffer, ", "   );
        formatter<     double>{}(buffer, vec.y  );
        formatter<const char*>{}(buffer, ", "   );
        formatter<     double>{}(buffer, vec.z  );
        formatter<const char*>{}(buffer, "}"    );
    }
};

int main() {
    // Test
    assert(log::stringify(vec3{1, 2, 3}) == "vec3{1, 2, 3}");
}