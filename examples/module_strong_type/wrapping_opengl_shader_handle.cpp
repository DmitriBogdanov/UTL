#include "include/UTL/strong_type.hpp"

// Mock of an OpenGL API
using GLuint = unsigned int;
using GLenum = unsigned int;

GLuint glCreateShader ([[maybe_unused]] GLenum shader_type) { return 0; }
void   glCompileShader([[maybe_unused]] GLuint shader_id  ) {           }
void   glDeleteShader ([[maybe_unused]] GLuint shader_id  ) {           }

#define GL_VERTEX_SHADER 1

int main() {
    using namespace utl;
    
    // Create strongly typed wrapper around OpenGL shader handle
    // (unsigned int) with move-only semantics and RAII cleanup
    using shader_handle = strong_type::unique<GLuint, class shader_tag, strong_type::bind<&glDeleteShader>>;
    
    shader_handle shader = glCreateShader(GL_VERTEX_SHADER);
    
    // ...
    
    // Retrieve underlying value and pass it to a 'C' API
    glCompileShader(shader.get());
}