#ifndef Depth_Pre_Pass_H
#define Depth_Pre_Pass_H
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "renderpass/RenderPass.h"
#include "gl460/ShaderProgram.h"
#include "asset_manager.h"
#include "gl460/framebuffer.h"
#include "gl460/texture.h"
#include "camera.h"
#include "scene/mesh.h"

class DepthPrePass : public RenderPass {
public:
    using SharedPtr = std::shared_ptr<DepthPrePass>;
    static SharedPtr create(const std::string& pass_name, Camera* camera);
public:
    DepthPrePass(const std::string& pass_name);
    virtual ~DepthPrePass() = default;


    void Execute() override;
    void SetMeshes(std::vector<Mesh*> meshes, const glm::mat4& worldMatrix) override;

    GLuint GetOutputHandle() const override { return depth_tex->getApiHandle(); }

private:
    std::string m_pass_name;

    std::vector<Mesh*> m_meshes;
    glm::mat4 m_model;

    ShaderProgram depth_shader;
    std::shared_ptr<gl::Framebuffer> depth_fbo;
    std::shared_ptr <gl::Texture> depth_tex;
    Camera* camera;
    int m_width = 1280;// 1024;
    int m_height = 720;// 1024;
};

#endif // !Depth_Pre_Pass_H


