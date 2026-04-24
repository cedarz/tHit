#include "mesh.h"

#include <glad/gl.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "asset_manager.h"
#include "gl460/ShaderProgram.h"
#include "utils/utils.h"

#include <string>
#include <vector>
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"
using namespace std;

unsigned int TextureFromFile(const string& filename, //const string &directory,
                             bool gamma)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    //auto img = read_png_file(filename.c_str(), width, height, nrComponents);

    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

    std::vector<glm::vec3> rgbs;
    int idx = 0;
    for (int h = 0; h < height; ++h) {
        //std::cout << h << std::endl;
        for (int w = 0; w < width; ++w) {
            float r = data[(width * h + w) * nrComponents + 0] / 255.0;
            float g = data[(width * h + w) * nrComponents + 1] / 255.0;
            float b = data[(width * h + w) * nrComponents + 2] / 255.0;
            float a = data[(width * h + w) * nrComponents + 3] / 255.0;
            rgbs.push_back(glm::vec3(r, g, b));
        }
    }

    std::vector<uint8_t> ret;  //(width * height * channel);// 3);

    //int idx = 0;
    //for (int h = 0; h < height; ++h) {
    //    for (int w = 0; w < width; ++w) {
    //        //std::cout << (width * h + w) << std::endl;
    //        //ret.push_back(data[(width * h + w) * nrComponents + 0]);
    //        //ret.push_back(data[(width * h + w) * nrComponents + 1]);
    //        //ret.push_back(data[(width * h + w) * nrComponents + 2]);
    //        ret.push_back(data[idx * nrComponents + 0]);
    //        ret.push_back(data[idx * nrComponents + 1]);
    //        ret.push_back(data[idx * nrComponents + 2]);
    //        //ret.push_back(data[idx * nrComponents + 3]);

    //        idx++;
    //    }
    //}
    //nrComponents = 3;
    int req = 4;


    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void Model::UpLoad()
{
    //this->vertices = vertices;
    //this->indices = indices;
    //this->textures = textures;
    albedo = TextureFromFile(AssetManager::GetAssetPath("testball/Sphere_BaseColor.png"), true);
    normal = TextureFromFile(AssetManager::GetAssetPath("testball/Sphere_Normal.png"), false);
    mra = TextureFromFile(AssetManager::GetAssetPath("testball/Sphere_mra.png"), false);
    //albedo = TextureFromFile(AssetManager::GetAssetPath("testball/Sphere_mra.png"), false);

    /*albedo = TextureFromFile(AssetManager::GetAssetPath("Earth_baseColor.png"), true);
    normal = TextureFromFile(AssetManager::GetAssetPath("earth/textures/Earth_normal.png"), false);
    mra = TextureFromFile(AssetManager::GetAssetPath("earth/textures/Earth_metallicRoughness.png"), false);*/

    // hair
    albedo = TextureFromFile(AssetManager::GetAssetPath("hair002/textures/HAIRa_Diffuse.png"), true);
    normal = TextureFromFile(AssetManager::GetAssetPath("hair002/textures/normal_1.png"), false);

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
}

void Model::Draw(ShaderProgram& shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    //for(unsigned int i = 0; i < textures.size(); i++)
    //{
    //    glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
    //    // retrieve texture number (the N in diffuse_textureN)
    //    string number;
    //    string name = textures[i].type;
    //    if(name == "texture_diffuse")
    //        number = std::to_string(diffuseNr++);
    //    else if(name == "texture_specular")
    //        number = std::to_string(specularNr++); // transfer unsigned int to string
    //    else if(name == "texture_normal")
    //        number = std::to_string(normalNr++); // transfer unsigned int to string
    //     else if(name == "texture_height")
    //        number = std::to_string(heightNr++); // transfer unsigned int to string

    //    // now set the sampler to the correct texture unit
    //    glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
    //    // and finally bind the texture
    //    glBindTexture(GL_TEXTURE_2D, textures[i].id);
    //}

    //glActiveTexture(GL_TEXTURE0);
    //glUniform1i(glGetUniformLocation(shader.ID, "albedoMap"), 0);
    //    // and finally bind the texture
    //glBindTexture(GL_TEXTURE_2D, albedo);

    //glActiveTexture(GL_TEXTURE1);
    //glUniform1i(glGetUniformLocation(shader.ID, "normalMap"), 1);
    //// and finally bind the texture
    //glBindTexture(GL_TEXTURE_2D, normal);

    //glActiveTexture(GL_TEXTURE2);
    //glUniform1i(glGetUniformLocation(shader.ID, "mraMap"), 2);
    //// and finally bind the texture
    //glBindTexture(GL_TEXTURE_2D, mra);
    if (indices.empty()) return;
    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void Model::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}

void HairMesh::UpLoad()
{
    setupMesh();
}

void HairMesh::Draw(ShaderProgram& shader)
{
    if (VAO == 0) return;
    glBindVertexArray(VAO);

    float lineWidth[2];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidth);
    glDrawElements(GL_LINES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void HairMesh::setupMesh()
{
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
    glBindVertexArray(0);
}
