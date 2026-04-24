#include "ShadowDepthPass.h"
#include <glad/gl.h>
#include <spdlog/spdlog.h>


ShadowDepthPass::SharedPtr ShadowDepthPass::create(const std::string& pass_name, Camera* camera)
{
    SharedPtr pShadowDepth = SharedPtr(new ShadowDepthPass(pass_name));
    pShadowDepth->shadow_camera = camera;
    return pShadowDepth;
}

ShadowDepthPass::ShadowDepthPass(const std::string& pass_name) : RenderPass(pass_name)
{
    depth_tex = gl::Texture::create2D(m_width, m_height, ResourceFormat::D24Unorm);
    depth_fbo = std::make_shared<gl::Framebuffer>();
    depth_fbo->SetAttatchments({}, depth_tex);
    shadow_depth_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/shadow_depth.vert")} });
}

void ShadowDepthPass::Execute()
{
    GLint vp[4]; glGetIntegerv(GL_VIEWPORT, vp);
    spdlog::info("viewport : {} {} {} {} ", vp[0], vp[1], vp[2], vp[3]);
    depth_fbo->use();
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    shadow_depth_shader.use();
    // view/projection transformations
    glm::mat4 projection = glm::perspective(
        glm::radians(shadow_camera->Zoom), (float)m_width / (float)m_height, 10.0f, 1000.0f);
    glm::mat4 view = shadow_camera->GetViewMatrix();
    shadow_depth_shader.setMat4("projection", projection);
    shadow_depth_shader.setMat4("view", view);
    shadow_depth_shader.setMat4("model", m_model);
    //GLint vp[4]; 
    glGetIntegerv(GL_VIEWPORT, vp);
    spdlog::info("viewport : {} {} {} {} ", vp[0], vp[1], vp[2], vp[3]);
    for (Mesh* m : m_meshes)
    {
        m->Draw(shadow_depth_shader);
    }
    //mesh.Draw(shadow_depth_shader);
    //hairmesh.Draw(shadow_depth_shader);

    depth_fbo->unuse();
}

void ShadowDepthPass::SetMeshes(std::vector<Mesh*> meshes, const glm::mat4& worldMatrix)
{
    m_meshes = meshes;
    m_model = worldMatrix;
}
