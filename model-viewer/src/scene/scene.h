/*
 * Simplified Scene class for loader compatibility
 * Stores loaded scene data that can be converted to the project's Model format
 */

#pragma once

#include "loader_types.h"
#include <string>
#include <vector>
#include <memory>
#include <scene/mesh.h>
#include <aabb.h>
#include <camera.h>
#include <gl460/buffer.h>

struct MeshVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

class CMesh
{
   public:
    CMesh() = default;
    //CMesh(const CMesh&) = delete;
    //CMesh& operator=(const CMesh&) = delete;
    CMesh(CMesh&& other)
        : m_Vertices(other.m_Vertices), m_Indices(other.m_Indices), aabb(other.aabb), m_VAO(other.m_VAO), m_VBO(other.m_VBO), m_EBO(other.m_EBO) {
        other.m_VAO = -1;
        other.m_VBO = -1;
        other.m_EBO = -1;
    }
    CMesh& operator=(CMesh&&) = default;
    CMesh(const std::vector<MeshVertex>& verts, const std::vector<uint32_t>& indices, const AABB& bb);
    ~CMesh();
    void BuildVAO();
    AABB aabb;

    void draw() {
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

   private:
    std::vector<MeshVertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
    GLuint m_VAO = -1;
    GLuint m_VBO = -1;
    GLuint m_EBO = -1;

};


struct TextureInfo
{
    std::string file_name;
    GLuint tex_id;
    GLuint64 tex_handle;
    void Load();
};

class Scene {
public:
    Scene();
    ~Scene();

    void BuildGpuResources();
    
    // Add methods matching GLSL-PathTracer Scene API
    int AddMesh(const std::string& filename);
    int AddMesh(const std::vector<MeshVertex>& verts, const std::vector<uint32_t>& indices, AABB aabb);
    int AddTexture(const std::string& filename);
    int AddMaterial(const Material& material);
    int AddMeshInstance(const MeshInstance& meshInstance);
    int AddLight(const Light& light);
    void AddCamera(const glm::vec3& eye, const glm::vec3& lookat, float fov);
    void AddEnvMap(const std::string& filename);
    void AddModel(const std::string& filename, const glm::mat4& transform, int matID);
    
    // Data storage
    std::vector<CMesh> meshes;
    std::vector<TextureInfo> textures;  // Store texture file paths
    std::vector<GLuint64> texture_handles; // bindless texture https://ktstephano.github.io/rendering/opengl/bindless
    std::vector<Material> materials;
    std::vector<MeshInstance> meshInstances;
    std::vector<Light> lights;
    std::unique_ptr<gl::Buffer> materialBuffer; // uniform buffer
    
    // Camera data
    struct CameraData {
        glm::vec3 position;
        glm::vec3 lookAt;
        float fov;
        float aperture;
        float focalDist;
    } camera;
    
    // Environment map
    std::string envMapFile;
    
    // Render options
    RenderOptions renderOptions;

public:
    AABB sceneAABB;
    //Camera camera;
    //Camera shadow_camera;
    
private:
    int nextMeshId;
    int nextTextureId;
};
