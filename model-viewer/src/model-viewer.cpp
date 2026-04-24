#include <filesystem>
#include "trackball_viewer.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include "scene/mesh.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <hair/hair.h>
#include "hair/cyHairFile.h"
//#include "hairmesh.h"
#include <mikktspace_wrapper.h>
#include <assimp/version.h>
#include <spdlog/spdlog.h>

#include "arcball.h"
#include "aabb.h"
#include "asset_manager.h"
#include "gl460/framebuffer.h"
#include "gl460/texture.h"
#include "gl460/buffer.h"
#include "gl460/gl_enum_string_helper.h"
#include "gl460/ShaderProgram.h";
#include "gfx/pipeline.h"

#include "renderpass/ShadowDepthPass.h"
#include "scene/loader.h"
#include "scene/scene.h"


 glm::vec3 lightPositions[] = {
    glm::vec3(94.26411, 264.5282, 92.26411)
};
glm::vec3 lightColors[] = {
    glm::vec3(0.430, 0.380, 0.280),
};

struct ListNode
{
    //float color[3];
    uint32_t color; // packed color
    float depth;
    unsigned next;
};

class ModelViewer : public TrackballViewer
{
   public:
    ModelViewer(uint32_t width, uint32_t height)
        : TrackballViewer("ModelViewer", width, height, false),
          arcball(glm::vec3(0.0f, 0.0f, 0.0f), 1.f)
    {
        depth_tex = gl::Texture::create2D(this->width(), this->height(), ResourceFormat::D24Unorm);
        depth_fbo = std::make_shared<gl::Framebuffer>();
        depth_fbo->SetAttatchments({}, depth_tex);

        color_tex = std::make_shared<gl::Texture>(this->width(), this->height(), 4, false, gl::PixelType::f8, nullptr, 0, 1);
        color_fbo = std::make_shared<gl::Framebuffer>();
        color_fbo->SetAttatchments({ color_tex }, depth_tex);

        glCreateVertexArrays(1, &vao);
        MAX_LIST_NODE = width * height;

        shadow_depth_pass = ShadowDepthPass::create("shadow_depth", &shadow_camera);
    }

    Model debugmesh;
    Model mesh;
    Hair hair;
    HairMesh hairmesh;
    ShaderProgram shader;
    ShaderProgram hair_shader;
    ShaderProgram depth_shader;
    ShaderProgram resolve_ppll_shader;
    ShaderProgram shadow_depth_shader;
    ShaderProgram debug_shader;
    ShaderProgram pbr_shader;

    std::shared_ptr<RenderPass> shadow_depth_pass;

    Camera camera;
    Camera shadow_camera;
    ShaderProgram test_shader;
    AABB aabb;
    ArcBall arcball;
    glm::mat4 model = glm::mat4(1.0f);

    std::shared_ptr<gl::Texture> depth_tex;
    std::shared_ptr<gl::Framebuffer> depth_fbo;

    std::shared_ptr<gl::Texture> color_tex;
    std::shared_ptr<gl::Framebuffer> color_fbo;

    GLuint vao;
    std::shared_ptr<gl::Texture> ppll_tex;
    GLuint* clear_ppll_tex_data;
    std::shared_ptr<gl::Buffer> atomic_count;
    std::shared_ptr<gl::Buffer> ppll_nodes;
    size_t MAX_LIST_NODE;
    uint32_t node_count = 0;

    Scene loadedScene;

public:

    void Update()
    {
        glm::mat4 T(1.0);
        T = glm::translate(T, aabb.center());

        // render the loaded model
        glm::mat4 T1 = glm::mat4(1.0f);
        T1 = glm::translate(T1, -aabb.center()); // glm::vec3(0.0f, 0.0f,0.0f));  // translate it down so it's at the center of the scene
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));  // it's a bit too big for our scene, so scale it down
        glm::mat4 rot = glm::inverse(arcball.getTransformation());

        glm::mat4 T2 = glm::mat4(1.0f);
        T2 = glm::translate(T2, aabb.center());

        model = T2 * rot * T1;

        shadow_depth_pass->SetMeshes({ &mesh, &hairmesh }, model);
    }

    void CreateHairBuffers()
    {
        ppll_tex = gl::Texture::create2D(width(), height(), ResourceFormat::R32Uint);
        atomic_count = std::make_shared<gl::Buffer>(sizeof(uint32_t));
        ppll_nodes = std::make_shared<gl::Buffer>(width() * height() * 64 * sizeof(ListNode));

        clear_ppll_tex_data = new GLuint[MAX_LIST_NODE];
        memset(clear_ppll_tex_data, 0xFF, MAX_LIST_NODE * sizeof(GLuint));
        ppll_tex->write(clear_ppll_tex_data, MAX_LIST_NODE * sizeof(GLuint));

        atomic_count->write(&node_count, sizeof(uint32_t));

        glBindImageTexture(0, ppll_tex->native_handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
        atomic_count->bindIndexed(GL_ATOMIC_COUNTER_BUFFER, 0);
        ppll_nodes->bindIndexed(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void shadow_depth()
    {
        depth_fbo->use();
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_BLEND);
        shadow_depth_shader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(shadow_camera.Zoom), (float)width() / (float)height(), 10.0f, 1000.0f);
        glm::mat4 view = shadow_camera.GetViewMatrix();
        shadow_depth_shader.setMat4("projection", projection);
        shadow_depth_shader.setMat4("view", view);
        shadow_depth_shader.setMat4("model", model);

        mesh.Draw(shadow_depth_shader);
        hairmesh.Draw(shadow_depth_shader);

        //test_shader.unUse();
        depth_fbo->unuse();
    }

    void draw_depth()
    {
        depth_fbo->use();
        //glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glDisable(GL_BLEND);
        glClearDepth(1.0);
        glClear(GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)width() / (float)height(), 10.0f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        //model = glm::inverse(model);
        //model = glm::affineInverse(model);
        shader.setMat4("model", model);
        shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.setVec3("lightPositions[0]", lightPositions[0]);
        shader.setVec3("lightColors[0]", lightColors[0]);

        shader.setVec3("camPos", camera.Position);
        mesh.Draw(shader);
        //hairmesh.Draw(shader);

        glm::vec4 pp{ -2.78896, 77.66232, 46.29383, 1.0 };
        glm::vec4 pp_model = view * model * pp;
        glm::vec4 pp_clip = projection * view * model * pp;

        draw_hair(hair_shader);

        depth_fbo->unuse();
    }

    void visualize_buffer()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, width(), height());

        depth_shader.use();
        
        glBindTextureUnit(0, depth_tex->native_handle());
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        //depth_shader.unUse();
    }

    void draw_hair(ShaderProgram& shader)
    {
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //color_fbo->use();

        //atomic_count->bindIndexed(GL_ATOMIC_COUNTER_BUFFER, 0);
        //GLuint* c = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
        //std::cout << *c << " " << width() * height() * 512 << std::endl;
        //glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

        ppll_tex->write(clear_ppll_tex_data, MAX_LIST_NODE * sizeof(GLuint));
        atomic_count->write(&node_count, sizeof(uint32_t));

        float strand_width = 3.0;
        // don't forget to enable shader before setting uniforms
        shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width() / (float)height(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        //model = glm::inverse(model);
        //model = glm::affineInverse(model);
        shader.setMat4("model", model);
        shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.setVec3("lightPositions[0]", lightPositions[0]);
        shader.setVec3("lightColors[0]", lightColors[0]);

        shader.setVec2("windowSize", glm::vec2(width(), height()));
        shader.setFloat("strandWdith", strand_width);
        shader.setFloat("u_NearPlane", 0.01);
        shader.setFloat("u_FarPlane", 1000.0);

        shader.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth_tex->native_handle());
        shader.setInt("depthMap", 3);

        glm::mat4 bias{
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
        };
        glm::mat4 lightP = glm::perspective(
            glm::radians(shadow_camera.Zoom), (float)width() / (float)height(), 10.0f, 1000.0f);
        glm::mat4 lightVP = bias * lightP * shadow_camera.GetViewMatrix();
        shader.setMat4("lightVP", lightVP);

        glLineWidth(strand_width);
        glDisable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_FALSE);
        hairmesh.Draw(shader);
        //glDisable(GL_BLEND);
        //color_fbo->unuse();
        //return;
        // resolve ppll
        //glPushGroupMarkerEXT 
        resolve_ppll_shader.use();
        //glBindTextureUnit(0, color_tex->native_handle());
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_ONE);
        //glDisable(GL_BLEND);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        resolve_ppll_shader.unUse();
    }

    void draw_hair(const std::string& drawMode)
    {
        Update();

        //draw_depth();
        //visualize_buffer();
        //draw_hair(depth_shader);
        shadow_depth();
        //shadow_depth_pass->Execute();
        //visualize_buffer();
        //return;
        
        //return;
        // draw mesh
        //renderer_.draw(projection_matrix_, modelview_matrix_, drawMode);
        //glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        //color_fbo->use();

        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glClearDepth(1.0);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //color_fbo->use();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   

        // don't forget to enable shader before setting uniforms
        shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)width() / (float)height(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        //model = glm::inverse(model);
        //model = glm::affineInverse(model);
        shader.setMat4("model", model);
        shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.setVec3("lightPositions[0]", lightPositions[0]);
        shader.setVec3("lightColors[0]", lightColors[0]);

        shader.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth_tex->native_handle());
        shader.setInt("depthMap", 3);

        glm::mat4 bias{
            0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.0,
            0.5, 0.5, 0.5, 1.0
        };
        glm::mat4 lightP = glm::perspective(
            glm::radians(shadow_camera.Zoom), (float)width() / (float)height(), 10.0f, 1000.0f);
        glm::mat4 lightVP = bias * lightP * shadow_camera.GetViewMatrix();
        shader.setMat4("lightVP", lightVP);

        mesh.Draw(shader);
        //color_fbo->unuse();
        //hairmesh.Draw(shader);
        draw_debug(debug_shader);

        draw_hair(hair_shader);
        //color_fbo->unuse();
    }

    void draw(const std::string& drawMode)
    {
        Update();

        // draw_depth();
        // visualize_buffer();
        shadow_depth();
        // shadow_depth_pass->Execute();
        // visualize_buffer();
        // return;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ShaderProgram& shader = pbr_shader;
        shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width() / (float)height(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // model = glm::inverse(model);
        // model = glm::affineInverse(model);
        shader.setMat4("model", model);
        shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.setVec3("lightPositions[0]", lightPositions[0]);
        shader.setVec3("lightColors[0]", lightColors[0]);

        shader.setVec3("camPos", camera.Position);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depth_tex->native_handle());
        shader.setInt("depthMap", 3);

        glm::mat4 bias{0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
        glm::mat4 lightP = glm::perspective(glm::radians(shadow_camera.Zoom), (float)width() / (float)height(), 10.0f, 1000.0f);
        glm::mat4 lightVP = bias * lightP * shadow_camera.GetViewMatrix();
        shader.setMat4("lightVP", lightVP);

        //mesh.Draw(shader);
        for (auto& mi : loadedScene.meshInstances) {

            CMesh& mesh = loadedScene.meshes[mi.meshID];
            glm::mat4 transform = mi.transform;
            Material mat = loadedScene.materials[mi.materialID];
            shader.setMat4("model", model * transform);
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(shader.id(), "albedoMap"), 0);
            glBindTexture(GL_TEXTURE_2D, loadedScene.textures[mat.baseColorTexId].tex_id);

            glActiveTexture(GL_TEXTURE1);
            glUniform1i(glGetUniformLocation(shader.id(), "normalMap"), 1);
            glBindTexture(GL_TEXTURE_2D, loadedScene.textures[mat.normalmapTexID].tex_id);

            glActiveTexture(GL_TEXTURE2);
            glUniform1i(glGetUniformLocation(shader.id(), "mraMap"), 2);
            glBindTexture(GL_TEXTURE_2D, loadedScene.textures[mat.metallicRoughnessTexID].tex_id);

            mesh.draw();


        }

    }


    void LoadShaderFile() { 
        shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/shader.vert")},
            {Shader::Type::Fragment, AssetManager::GetAssetPath("shaders/lambert.frag")} });

        pbr_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/shader.vert")},
            {Shader::Type::Fragment, AssetManager::GetAssetPath("shaders/shader.frag")} });
        hair_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/hair.vert")},
            {Shader::Type::Fragment, AssetManager::GetAssetPath("shaders/hair.frag")} });
        depth_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/BufferVisualisation.vert")},
            {Shader::Type::Fragment, AssetManager::GetAssetPath("shaders/BufferVisualisation.frag")} });
        resolve_ppll_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/resolve_ppll_blend.vert")},
            {Shader::Type::Fragment, AssetManager::GetAssetPath("shaders/resolve_ppll_blend.frag")} });
        debug_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/debug.vert")},
            {Shader::Type::Fragment, AssetManager::GetAssetPath("shaders/debug.frag")} });
        shadow_depth_shader.load({ {Shader::Type::Vertex, AssetManager::GetAssetPath("shaders/shadow_depth.vert")} });

        shadow_depth_shader.use();

        int count;
        GLsizei length = 0;
        GLint size = 0;
        GLenum type;
        GLchar name[256];

        glGetProgramiv(shadow_depth_shader.id(), GL_ACTIVE_ATTRIBUTES, &count);
        printf("Active Attributes: %d\n", count);
        for (int i = 0; i < count; i++) {
            glGetActiveAttrib(shadow_depth_shader.id(), (GLuint)i, 256, &length,
                              &size, &type, name);

            printf("Attribute #%d Type: %u Name: %s\n", i, type, name);
        }

        glGetProgramiv(shadow_depth_shader.id(), GL_ACTIVE_UNIFORMS, &count);
        printf("Active Uniforms: %d\n", count);

        for (int i = 0; i < count; i++) {
            glGetActiveUniform(shadow_depth_shader.id(), (GLuint)i, 256, &length,
                               &size, &type, name);

            printf("Uniform #%d Type: %s Name: %s\n", i, string_gl_enum(type), name);
        }
        shadow_depth_shader.unUse();
    }

    void ResetCamera()
    {
        camera = Camera(aabb.center() + glm::vec3(0.0, 0.0, 1.66 * aabb.bounding_radius()));
        shadow_camera = Camera(lightPositions[0], aabb.center(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void scroll(double xoffset, double yoffset) {
        float d = (float)yoffset * 0.12 * aabb.bounding_radius();
        camera.ZoomIn(-d);
    }

    void motion(double xpos, double ypos)
    {
        if (last_point_ok_) {
            // zoom
            if (right_mouse_pressed() ||
                (left_mouse_pressed() && shift_pressed())) {
                //zoom(xpos, ypos);
                float dy = ypos - last_point_2d_[1];
                float h = height();
                //translate(glm::vec3(0.0, 0.0, radius_ * dy * 3.0 / h));
                float zooming = dy * aabb.bounding_radius() * 3.0 / h;
                camera.ZoomIn(zooming);
            }

            // translation
            else if (middle_mouse_pressed() ||
                     (left_mouse_pressed() && alt_pressed())) {
                //translation(xpos, ypos);
                float dx = xpos - last_point_2d_[0];
                float dy = ypos - last_point_2d_[1];

                float z = fabsf(camera.Position.z - aabb.center().z);
                const float hh = z * tan(glm::radians(camera.Zoom) * 0.5f);
                const float hw = hh * width() / height();

                glm::vec2 dndc = glm::vec2(1.0, -1.0) * glm::vec2(dx, dy) * 2.0f / glm::vec2(width(), height());
                camera.Move(dndc.x * hw, dndc.y * hh);
            }

            // rotation
            else if (left_mouse_pressed()) {
                //rotation(xpos, ypos);
                auto mappoint = [this](const glm::ivec2& in) {
                    float s = min(width(), height()) - 1.0;
                    float px = (in.x * 2.0 - width() + 1) / s;
                    float py = -(in.y * 2.0 - height() + 1) / s;
                    return glm::vec2(px, py);
                };
                //map_to_sphere()
                std::cout << last_point_2d_.x << " " << last_point_2d_.y
                          << " : " << xpos << " " << ypos << std::endl;
                arcball.beginDrag(mappoint(last_point_2d_));
                arcball.drag(mappoint(glm::vec2(xpos, ypos)));
            }
        }

        //if (left_mouse_pressed()) {
        //    //rotation(xpos, ypos);
        //    arcball.beginDrag(glm::vec2(last_point_2d_) * glm::vec2(2.0, 2.0) /
        //                      glm::vec2(width(), height()) - glm::vec2(1.0));
        //    arcball.drag(glm::vec2(xpos, ypos) * glm::vec2(2.0, 2.0) /
        //                      glm::vec2(width(), height()) - glm::vec2(1.0));
        //}
        

        // remember points
        last_point_2d_ = glm::ivec2(xpos, ypos);
        

        last_point_ok_ = map_to_sphere(last_point_2d_, last_point_3d_);
    }

    bool LoadHair(const std::string& filename)
    {
        hair.read(filename.c_str());
        hair.compute_tangents();

        for (int i = 0; i < hair.strands.size(); ++i)
        {
            //std::cout << i << " : " << hair.strands[i].size() << std::endl;
            for (int j = 0; j < hair.strands[i].size() - 1; ++j)
            {
                Vertex vert;
                vert.Position = hair.strands[i][j];
                vert.Tangent = hair.tangents[i][j];

                hairmesh.indices.push_back(hairmesh.vertices.size());
                hairmesh.vertices.push_back(vert);
                aabb.push(vert.Position);

                vert.Position = hair.strands[i][j + 1];
                vert.Tangent = hair.tangents[i][j + 1];

                hairmesh.indices.push_back(hairmesh.vertices.size());
                hairmesh.vertices.push_back(vert);
                if (j == hair.strands[i].size() - 1)
                {
                    aabb.push(vert.Position);
                }
            }

        }
        hairmesh.UpLoad();

        return true;
    }

    void LoadHairFile()
    {
        cy::HairFile cyHair;
        cyHair.LoadFromFile(AssetManager::GetAssetPath("cyHair/ponytail.hair").c_str());
        // https://www.cemyuksel.com/cyCodeBase/soln/using_hair_files.html

        int hairCount = cyHair.GetHeader().hair_count;
        int pointCount = cyHair.GetHeader().point_count;
        int segment = cyHair.GetHeader().d_segments;
        spdlog::info("Number of hair strands = {}", hairCount);
        spdlog::info("Number of hair points = {}", pointCount);
        spdlog::info("Number of hair segment = {}", segment);
        //float* dirs = new float[pointCount * 3];
        //cyHair.FillDirectionArray(dirs);
        float* points = cyHair.GetPointsArray();
        float* tangents = cyHair.GetTangetsArray();
        const unsigned short* segments = cyHair.GetSegmentsArray();
        /*for (int i = 0; i < hairCount; ++i) {
            int segs = segments[i] + 1;
        }*/
        if (!segments)
        {
            for (int i = 0; i < hairCount; ++i)
            {
                for (int j = 0; j < cyHair.GetHeader().d_segments; ++j) {
                    Vertex vert;
                    int idx = (i * (cyHair.GetHeader().d_segments + 1) + j) * 3;
                    vert.Position = glm::vec3(points[idx] / 10.0, points[idx + 1] / 10.0, points[idx + 2] / 10.0);
                    vert.Tangent = glm::vec3(tangents[idx], tangents[idx + 1], tangents[idx + 2]);

                    hairmesh.indices.push_back(hairmesh.vertices.size());
                    hairmesh.vertices.push_back(vert);
                    aabb.push(vert.Position);

                    idx = (i * (cyHair.GetHeader().d_segments + 1) + j + 1) * 3;
                    vert.Position = glm::vec3(points[idx] / 10.0, points[idx + 1] / 10.0, points[idx + 2] / 10.0);
                    vert.Tangent = glm::vec3(tangents[idx], tangents[idx + 1], tangents[idx + 2]);

                    hairmesh.indices.push_back(hairmesh.vertices.size());
                    hairmesh.vertices.push_back(vert);
                    if (j == cyHair.GetHeader().d_segments)
                    {
                        aabb.push(vert.Position);
                    }
                }
            }
        }
        hairmesh.UpLoad();

    }

    bool LoadFromFile(const std::string& filename)
    {
        //name = filename;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        
        // Extract directory path for .mtl file search
        std::string mtlBaseDir = filename.substr(0, filename.find_last_of("/\\"));
        if (!mtlBaseDir.empty()) {
            mtlBaseDir += "/";
        }
        
        // New API: LoadObj(attrib, shapes, materials, warn, err, filename, mtl_basedir, triangulate, default_vcols_fallback)
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                    filename.c_str(), mtlBaseDir.c_str(), true);

        if (!ret) {
            printf("Unable to load model\n");
            return false;
        }

        /*std::ofstream fout(AssetManager::GetAssetPath("cyHair/ponytail/ponytailscale.obj"));
        for (int i = 0; i < attrib.vertices.size(); i += 3) {
            std::string s = fmt::format("v {} {} {}", attrib.vertices[i + 0] / 10.0, attrib.vertices[i + 1] / 10.0, attrib.vertices[i + 2] / 10.0);
            fout << s << std::endl;
        }
        for (int i = 0; i < attrib.normals.size(); i += 3) {
            std::string s = fmt::format("vn {} {} {}", attrib.normals[i + 0], attrib.normals[i + 1], attrib.normals[i + 2]);
            fout << s << std::endl;
        }
        for (int i = 0; i < attrib.texcoords.size(); i += 2) {
            std::string s = fmt::format("vt {} {}", attrib.texcoords[i + 0], attrib.texcoords[i + 1]);
            fout << s << std::endl;
        }*/

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;

            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

                /*tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];
                std::string sf = fmt::format("f {}/{}/{} {}/{}/{} {}/{}/{}", 
                    idx0.vertex_index + 1, idx0.texcoord_index + 1, idx0.normal_index + 1,
                    idx1.vertex_index + 1, idx1.texcoord_index + 1, idx1.normal_index + 1,
                    idx2.vertex_index + 1, idx2.texcoord_index + 1, idx2.normal_index + 1
                    );
                fout << sf << std::endl;*/

                // Loop over vertices in the face.
                for (size_t v = 0; v < 3; v++) {
                    // access to vertex
                    tinyobj::index_t idx =
                        shapes[s].mesh.indices[index_offset + v];
                    tinyobj::real_t vx =
                        attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy =
                        attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz =
                        attrib.vertices[3 * idx.vertex_index + 2];
                    tinyobj::real_t nx =
                        attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny =
                        attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz =
                        attrib.normals[3 * idx.normal_index + 2];

                    tinyobj::real_t tx, ty;

                    if (!attrib.texcoords.empty()) {
                        tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                        ty = 1.0 - attrib.texcoords[2 * idx.texcoord_index + 1];
                    } else {
                        if (v == 0)
                            tx = ty = 0;
                        else if (v == 1)
                            tx = 0, ty = 1;
                        else
                            tx = ty = 1;
                    }

                    Vertex vert;
                    vert.Position = glm::vec3(vx, vy, vz);
                    vert.Normal = glm::vec3(nx, ny, nz);
                    vert.TexCoords = glm::vec2(tx, ty);
                    mesh.indices.push_back(mesh.vertices.size());
                    mesh.vertices.push_back(vert);

                    aabb.push(vert.Position);
                }

                index_offset += 3;
            }
        }

        auto T = MikkTSpaceWrapper::generateTangents(mesh);
        //mesh.vertices
        //fout.close();
        mesh.UpLoad();
        return true;
    }

    void draw_debug(ShaderProgram& debug_shader)
    {
        // don't forget to enable shader before setting uniforms
        debug_shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom), (float)width() / (float)height(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        debug_shader.setMat4("projection", projection);
        debug_shader.setMat4("view", view);
        debug_shader.setMat4("model", model);
        debug_shader.setVec3("Translation", lightPositions[0]);

        mesh.Draw(debug_shader);
    }

    void UpdateCamera(Scene* loadedScene, Camera& camera) {
        if (!loadedScene) return;

        const auto& camData = loadedScene->camera;

        // Create a new camera with position and lookAt
        //camera = Camera(camData.position, camData.lookAt, glm::vec3(0.0f, 1.0f, 0.0f));
        //camera.Zoom = camData.fov;

        aabb = loadedScene->sceneAABB;
        float r = aabb.bounding_radius();
        camera = Camera(aabb.center() + glm::vec3(0.0, 0.0, aabb.bounding_radius()));

        int count = std::min(static_cast<int>(loadedScene->lights.size()), 1);

        for (int i = 0; i < count; i++) {
            const auto& light = loadedScene->lights[i];
            lightPositions[i] = light.position;
            lightColors[i] = light.emission;
        }

        if (!loadedScene->lights.empty()) {
            shadow_camera = Camera(loadedScene->lights[0].position, aabb.center(), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        shadow_camera = Camera(lightPositions[0] * 100.0f, aabb.center(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    bool LoadSceneFromFile(const std::string& filename)
    {
        //Scene loadedScene;
        RenderOptions renderOptions;
        
        if (!LoadScene(filename, &loadedScene, renderOptions)) {
            printf("Failed to load scene file: %s\n", filename.c_str());
            return false;
        }
        
        // Update camera from loaded scene
        UpdateCamera(&loadedScene, camera);
        
        return true;
    }

    //void runImGui() override
    //{
    //    ImGui::Begin("UI");

    //    static char modelPath[100] = {"assets/sponza/sponza.obj"};
    //    ImGui::InputText("Model", modelPath, 100);
    //    if (ImGui::Button("Load Model")) {
    //        scene.setModel({std::string(CMAKE_SOURCE_DIR) + "/" + modelPath});
    //    }

    //    ImGui::Separator();

    //    ImGui::InputFloat("FOV", &camera.fov);
    //    ImGui::InputFloat("Movement Speed", &camera.movement_speed);
    //    ImGui::InputFloat("Look Around Speed", &camera.look_around_speed);

    //    if (ImGui::Button("Reset Camera")) { camera.reset(); }

    //    ImGui::Separator();

    //    ImGui::Combo(
    //        "Layer Type", reinterpret_cast<int *>(&layerType),
    //        "Position\0Normal\0TexCoords\0Tangent\0dndu\0dndv\0Diffuse\0Sp"
    //        "ecular\0Ambient\0Emis"
    //        "sive\0Height\0NormalMap\0Shininess\0Displacement\0Light\0\0");

    //    ImGui::End();
    //}
};

#include "scene/scene.h"
int main()
{
    spdlog::info("Assimp Version: {}.{}.{}", aiGetVersionMajor(), aiGetVersionMinor(), aiGetVersionRevision());
    ModelViewer app(1280, 720);
    app.LoadSceneFromFile(AssetManager::GetAssetPath("pbr.scene"));
    
    //app.LoadFromFile(AssetManager::GetAssetPath("testball/ball.obj"));
    app.LoadShaderFile();
    //app.LoadFromFile(AssetManager::GetAssetPath("hair002/hair002_norm.obj"));

    //app.LoadFromFile(AssetManager::GetAssetPath("USCHairSalon/head_model.obj"));
    //app.LoadHair();
    //app.LoadDebugFile(AssetManager::GetAssetPath("testball/ball.obj"));

#if 0 // draw hair
    app.LoadFromFile(AssetManager::GetAssetPath("cyHair/ponytail/ponytailscale.obj"));
    app.LoadHairFile();
    app.LoadShaderFile();
    app.CreateHairBuffers();
#endif

    //app.ResetCamera();

    app.run();

    return 0;
}