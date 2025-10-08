#include "include/UTL/log.hpp"

using namespace utl;

// Custom type
struct Vec3 { double x, y, z; };

// Extend formatter to support 'Vec3'
template <>
struct log::Formatter<Vec3> {
    template <class Buffer>
    void operator()(Buffer& buffer, const Vec3& vec) {
        Formatter<const char*>{}(buffer, "Vec3{");
        Formatter<     double>{}(buffer, vec.x  );
        Formatter<const char*>{}(buffer, ", "   );
        Formatter<     double>{}(buffer, vec.y  );
        Formatter<const char*>{}(buffer, ", "   );
        Formatter<     double>{}(buffer, vec.z  );
        Formatter<const char*>{}(buffer, "}"    );
    }
};

int main() {
    // Test
    assert(log::stringify(Vec3{1, 2, 3}) == "Vec3{1, 2, 3}");
}