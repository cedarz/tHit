#ifndef MESH_H
#define MESH_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "asset_manager.h"
#include "gl460/ShaderProgram.h"

#include <string>
#include <vector>
#include <iostream>

using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	//weights from each bone
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

unsigned int TextureFromFile(const string& filename, bool gamma);


class Mesh {
public:
    virtual void UpLoad() = 0;
    virtual void Draw(ShaderProgram& shader) = 0;
private:
    virtual void setupMesh() = 0;
};

class Model : public Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO = 0;
    unsigned int albedo;
    unsigned int normal;
    unsigned int mra;
    glm::vec3 getNormal(int32_t face, int32_t vert) const
    {
        int vid = indices[face * 3 + vert];
        return vertices[vid].Normal;
    }

    glm::vec2 getTexCrd(int32_t face, int32_t vert) const
    {
        int vid = indices[face * 3 + vert];
        return vertices[vid].TexCoords;
    }

    void UpLoad() override;
    void Draw(ShaderProgram& shader) override;
    

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh() override;
   
};

class HairMesh : public Mesh {
public:
    // mesh Data
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    //vector<Texture>      textures;
    unsigned int VAO = 0;
    unsigned int albedo;
    unsigned int normal;
    unsigned int mra;

    glm::vec3 getNormal(int32_t face, int32_t vert) const
    {
        int vid = indices[face * 3 + vert];
        return vertices[vid].Normal;
    }

    glm::vec2 getTexCrd(int32_t face, int32_t vert) const
    {
        int vid = indices[face * 3 + vert];
        return vertices[vid].TexCoords;
    }

    void UpLoad() override;
    void Draw(ShaderProgram& shader) override;

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh() override;
    
};
#endif
