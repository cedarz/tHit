#include "gl460/Shader.h"
#include "utils/utils.h"

#include <iostream>

namespace {
std::string ShaderName(const Shader::Type type) {
    switch(type) {
        case Shader::Type::Vertex:                  return "vertex";
        case Shader::Type::Geometry:                return "geometry";
        case Shader::Type::TessellationControl:     return "tessellation control";
        case Shader::Type::TessellationEvaluation:  return "tessellation evaluation";
        case Shader::Type::Compute:                 return "compute";
        case Shader::Type::Fragment:                return "fragment";
    }
}
}

Shader::Shader(Type type) {
    id_ = glCreateShader(GLenum(type));
}

// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteShader.xhtml
// glDeleteShader frees the memory and invalidates the name associated with the shader object specified by shader. 
// This command effectively undoes the effects of a call to glCreateShader. If a shader object to be deleted is 
// attached to a program object, it will be flagged for deletion, but it will not be deleted until it is no longer 
// attached to any program object, for any rendering context(i.e., it must be detached from wherever it was 
// attached before it will be deleted).A value of 0 for shader will be silently ignored.
Shader::~Shader() {
    if (!id_) return;
    glDeleteShader(id_);
}

void Shader::SetFile(const std::string& filename)
{
    SetSource(read_file(filename));
}

bool Shader::Compile()
{
    const char* pointer = shader_source_.c_str();
    glShaderSource(id_, 1, &pointer, nullptr);
    glCompileShader(id_);
    return CheckCompile();
}

bool Shader::CheckCompile()
{
    GLint success, logLength;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &logLength);

    if (!success)
    {
        std::string infoLog;
        infoLog.reserve(std::size_t(logLength));
        glGetShaderInfoLog(id_, logLength, nullptr, infoLog.data());
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << ShaderName(type_) << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    return success;
}

