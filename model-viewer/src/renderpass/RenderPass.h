#ifndef Render_Pass_H
#define Render_Pass_H
#include <glad/gl.h>
#include <string>
#include <vector>
#include <scene/mesh.h>

class RenderPass {
public:
    RenderPass(const std::string& pass_name) : m_pass_name(pass_name) {}
    virtual ~RenderPass();

    virtual void Execute() = 0;
    virtual void SetMeshes(std::vector<Mesh*> meshes, const glm::mat4& worldMatrix) = 0;
    virtual GLuint GetOutputHandle() const = 0;

private:
    std::string m_pass_name;
};

#endif // !Render_Pass_H


