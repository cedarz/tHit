#ifndef Shadow_Depth_Pass_H
#define Shadow_Depth_Pass_H
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

class ShadowPass : public RenderPass {
public:
    using SharedPtr = std::shared_ptr<ShadowPass>;
    static SharedPtr create(const std::string& pass_name, Camera* camera);
public:
    ShadowPass(const std::string& pass_name);
    virtual ~ShadowPass() = default;

    
    void Execute() override;
    void SetMeshes(std::vector<Mesh*> meshes, const glm::mat4& worldMatrix) override;

    GLuint GetOutputHandle() const override { return shadow_depth_tex->getApiHandle(); }

private:
    std::string m_pass_name;
    
    std::vector<Mesh*> m_meshes;
    glm::mat4 m_model;
    ShaderProgram visibility_shader;
    std::shared_ptr<gl::Framebuffer> visibility_fbo;

    ShaderProgram shadow_depth_shader;
    std::shared_ptr<gl::Framebuffer> shadow_depth_fbo;
    std::shared_ptr <gl::Texture> shadow_depth_tex;
    Camera* shadow_camera;
    int m_width = 1280;// 1024;
    int m_height = 720;// 1024;
};

#endif // !Shadow_Depth_Pass_H


