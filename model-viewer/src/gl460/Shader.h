#ifndef SHADER_H
#define SHADER_H
#include <glad/gl.h>

#include <string>

class Shader {
public:
    enum class Type : GLenum {
        Vertex = GL_VERTEX_SHADER,      /**< Vertex shader */
        TessellationControl = GL_TESS_CONTROL_SHADER,
        TessellationEvaluation = GL_TESS_EVALUATION_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Compute = GL_COMPUTE_SHADER,
        Fragment = GL_FRAGMENT_SHADER   /**< Fragment shader */
    };

    explicit Shader(Type type);
    ~Shader();

    GLuint id() const { return id_; }
    Type type() const { return type_; }
    void SetSource(const std::string& source) { shader_source_ = source; }
    void SetFile(const std::string& filename);
    bool Compile();
    bool CheckCompile();

private:

    GLuint id_;
    Type type_;
    std::string shader_source_;
};
#endif // !SHADER_H
