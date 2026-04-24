#include "scene.h"
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>

#include <tinyobjloader/tiny_obj_loader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <mikktspace_wrapper.h>

Scene::Scene() : nextMeshId(0), nextTextureId(0) {
    camera.position = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.fov = 45.0f;
    camera.aperture = 0.0f;
    camera.focalDist = 1.0f;
}

Scene::~Scene() {
}

void Scene::BuildGpuResources()
{
    materialBuffer = std::make_unique<gl::Buffer>(materials);
}

int Scene::AddMesh(const std::string& filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> tinyMaterials;
    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &warn, &err,
        filename.c_str(), nullptr, true);

    if (!ret) {
        printf("Unable to load model: %s\nError: %s\n", filename.c_str(), err.c_str());
        return -1;
    }

    if (!warn.empty()) {
        printf("Warning while loading %s: %s\n", filename.c_str(), warn.c_str());
    }

    if (!err.empty()) {
        printf("Error while loading %s: %s\n", filename.c_str(), err.c_str());
    }

    std::vector<MeshVertex> meshVerts;
    std::vector<uint32_t> meshIndices;
    AABB meshAabb;

    // Process each shape and create mesh instances
    for (size_t s = 0; s < shapes.size(); s++) {
        const auto& shape = shapes[s];
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            // Process each face (assuming triangles after triangulation)
            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                // Get vertex position
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

                // Get normal
                tinyobj::real_t nx = 0.0f, ny = 0.0f, nz = 0.0f;
                if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                    nx = attrib.normals[3 * idx.normal_index + 0];
                    ny = attrib.normals[3 * idx.normal_index + 1];
                    nz = attrib.normals[3 * idx.normal_index + 2];
                }

                // Get texture coordinates
                tinyobj::real_t tx = 0.0f, ty = 0.0f;
                if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    ty = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]; // Flip V coordinate
                }

                MeshVertex vert;
                vert.Position = glm::vec3(vx, vy, vz);
                vert.Normal = glm::vec3(nx, ny, nz);
                vert.TexCoords = glm::vec2(tx, ty);
                vert.Tangent = glm::vec3(0.0f); // Will be calculated later if needed

                meshAabb.push(vert.Position);

                meshVerts.push_back(vert);
                meshIndices.push_back(static_cast<uint32_t>(meshVerts.size() - 1));
            }
            index_offset += 3;
        }
    }

    return AddMesh(meshVerts, meshIndices, meshAabb);
    //return nextMeshId++;
}

int Scene::AddMesh(const std::vector<MeshVertex>& verts, const std::vector<uint32_t>& indices, AABB aabb) {
    if (verts.empty() || indices.empty()) {
        return -1;
    }
    
    meshes.emplace_back(verts, indices, aabb);
    meshes.back().BuildVAO();
    return nextMeshId++;
}

int Scene::AddTexture(const std::string& filename) {

    int id = -1;
    // Check if texture was already loaded
    for (int i = 0; i < textures.size(); i++) {
        if (textures[i].file_name == filename) {
            return i; // textures[i].tex_id;
        }
    }

    TextureInfo ti;
    ti.file_name = filename;
    ti.Load();
    textures.push_back(ti);
    texture_handles.push_back(ti.tex_handle);
    return nextTextureId++;
}

int Scene::AddMaterial(const Material& material) {
    materials.push_back(material);
    return static_cast<int>(materials.size() - 1);
}

int Scene::AddMeshInstance(const MeshInstance& meshInstance) {
    meshInstances.push_back(meshInstance);
    AABB bbox = meshes[meshInstance.meshID].aabb;
    glm::vec3 TMin = meshInstance.transform * glm::vec4(bbox.m_min, 1.0);
    glm::vec3 TMax = meshInstance.transform * glm::vec4(bbox.m_max, 1.0);
    bbox.m_min = glm::vec3(std::min(TMin.x, TMax.x), std::min(TMin.y, TMax.y), std::min(TMin.z, TMax.z));
    bbox.m_max = glm::vec3(std::max(TMin.x, TMax.x), std::max(TMin.y, TMax.y), std::max(TMin.z, TMax.z));
    sceneAABB.push(bbox);
    return static_cast<int>(meshInstances.size() - 1);
}

int Scene::AddLight(const Light& light) {
    lights.push_back(light);
    return static_cast<int>(lights.size() - 1);
}

void Scene::AddCamera(const glm::vec3& eye, const glm::vec3& lookat, float fov) {
    camera.position = eye;
    camera.lookAt = lookat;
    camera.fov = fov;
}

void Scene::AddEnvMap(const std::string& filename) {
    envMapFile = filename;
    renderOptions.enableEnvMap = true;
}

void Scene::AddModel(const std::string& filename, const glm::mat4& transform, int matID)
{
    // Extract directory path for .mtl file search and texture resolution
    std::string path = filename.substr(0, filename.find_last_of("/\\"));
    std::string mtlBaseDir;
    if (!path.empty()) {
        path += "/";
        mtlBaseDir = path;
    } else {
        mtlBaseDir = "./";
    }
    
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> tinyMaterials;
    std::string warn;
    std::string err;
    
    // Pass mtlBaseDir to LoadObj so it can find .mtl files
    // New API: LoadObj(attrib, shapes, materials, warn, err, filename, mtl_basedir, triangulate, default_vcols_fallback)
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &warn, &err,
                                filename.c_str(), mtlBaseDir.c_str(), true);
    
    if (!ret) {
        printf("Unable to load model: %s\nError: %s\n", filename.c_str(), err.c_str());
        return;
    }
    
    if (!warn.empty()) {
        printf("Warning while loading %s: %s\n", filename.c_str(), warn.c_str());
    }
    
    if (!err.empty()) {
        printf("Error while loading %s: %s\n", filename.c_str(), err.c_str());
    }
    
    printf("Loaded %zu materials from %s (mtl search dir: %s)\n", 
           tinyMaterials.size(), filename.c_str(), mtlBaseDir.c_str());
    
    // Map to store material ID by material name/index
    std::map<int, int> materialIdMap; // tinyobj material index -> Scene material ID
    
    // Convert tinyobj materials to Scene materials
    for (size_t i = 0; i < tinyMaterials.size(); i++) {
        const auto& tinyMat = tinyMaterials[i];
        Material mat;
        
        // Base color
        mat.baseColor = glm::vec3(tinyMat.diffuse[0], tinyMat.diffuse[1], tinyMat.diffuse[2]);
        
        // Metallic and roughness (if available in specular)
        // tinyobj doesn't have direct metallic/roughness, so we'll use defaults
        mat.metallic = 0.0f;
        mat.roughness = 0.5f;
        
        // Emission
        mat.emission = glm::vec3(tinyMat.emission[0], tinyMat.emission[1], tinyMat.emission[2]);
        
        // IOR
        mat.ior = tinyMat.ior;
        
        // Opacity
        mat.opacity = tinyMat.dissolve;
        if (mat.opacity < 1.0f) {
            mat.alphaMode = AlphaMode::Blend;
        }
        
        // Textures
        if (!tinyMat.diffuse_texname.empty()) {
            std::string texPath = path + tinyMat.diffuse_texname;
            mat.baseColorTexId = AddTexture(texPath);
        }
        
        if (!tinyMat.normal_texname.empty()) {
            std::string texPath = path + tinyMat.normal_texname;
            mat.normalmapTexID = AddTexture(texPath);
        }
        
        if (!tinyMat.emissive_texname.empty()) {
            std::string texPath = path + tinyMat.emissive_texname;
            mat.emissionmapTexID = AddTexture(texPath);
        }
        
        // Add material to scene
        int matId = AddMaterial(mat);
        materialIdMap[static_cast<int>(i)] = matId;
    }
    
    // Process each shape and create mesh instances
    for (size_t s = 0; s < shapes.size(); s++) {
        const auto& shape = shapes[s];
        
        // Convert shape to MeshVertex and indices
        std::vector<MeshVertex> meshVerts;
        std::vector<uint32_t> meshIndices;
        AABB meshAabb;
        
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            // Process each face (assuming triangles after triangulation)
            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                
                // Get vertex position
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                
                // Get normal
                tinyobj::real_t nx = 0.0f, ny = 0.0f, nz = 0.0f;
                if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                    nx = attrib.normals[3 * idx.normal_index + 0];
                    ny = attrib.normals[3 * idx.normal_index + 1];
                    nz = attrib.normals[3 * idx.normal_index + 2];
                }
                
                // Get texture coordinates
                tinyobj::real_t tx = 0.0f, ty = 0.0f;
                if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    ty = 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]; // Flip V coordinate
                }
                
                MeshVertex vert;
                vert.Position = glm::vec3(vx, vy, vz);
                vert.Normal = glm::vec3(nx, ny, nz);
                vert.TexCoords = glm::vec2(tx, ty);
                vert.Tangent = glm::vec3(0.0f); // Will be calculated later if needed

                meshAabb.push(vert.Position);
                
                meshVerts.push_back(vert);
                meshIndices.push_back(static_cast<uint32_t>(meshVerts.size() - 1));
            }
            index_offset += 3;
        }
        
        // Add mesh to scene
        int meshId = AddMesh(meshVerts, meshIndices, meshAabb);
        if (meshId < 0) {
            printf("Failed to add mesh for shape %s\n", shape.name.c_str());
            continue;
        }
        
        // Get material ID for this shape
        int materialId = 0; // Default to first material
        if (matID > 0) {
            materialId = matID;
        }
        else if(!shape.mesh.material_ids.empty() && shape.mesh.material_ids[0] >= 0) {
            int tinyMatId = shape.mesh.material_ids[0];
            if (materialIdMap.find(tinyMatId) != materialIdMap.end()) {
                materialId = materialIdMap[tinyMatId];
            }
        }
        
        // Create instance name from shape name or generate one
        std::string instanceName = shape.name;
        if (instanceName.empty()) {
            instanceName = filename.substr(filename.find_last_of("/\\") + 1);
            if (shapes.size() > 1) {
                instanceName += "_" + std::to_string(s);
            }
        }
 
        // Create and add mesh instance
        MeshInstance instance(instanceName, meshId, transform, materialId);
        AddMeshInstance(instance);
    }
    
    printf("Loaded model: %s (%zu shapes, %zu materials)\n", 
           filename.c_str(), shapes.size(), tinyMaterials.size());
}

CMesh::CMesh(const std::vector<MeshVertex>& verts, const std::vector<uint32_t>& indices, const AABB& bb) 
    : m_Vertices(verts), m_Indices(indices), aabb(bb) {
}

CMesh::~CMesh() {
    if (m_VAO != static_cast<GLuint>(-1)) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = static_cast<GLuint>(-1);
    }
    if (m_VBO != static_cast<GLuint>(-1)) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = static_cast<GLuint>(-1);
    }
    if (m_EBO != static_cast<GLuint>(-1)) {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = static_cast<GLuint>(-1);
    }
}

void CMesh::BuildVAO()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(MeshVertex), &m_Vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, Tangent));

    glBindVertexArray(0);
}

void TextureInfo::Load()
{
    tex_id = TextureFromFile(file_name, true);
    tex_handle = glGetTextureHandleARB(tex_id);
}
