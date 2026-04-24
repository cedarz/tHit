#ifndef MIKKTSPACE_WRAPPER_H
#define MIKKTSPACE_WRAPPER_H

#include <vector>
#include <numbers>
#include <algorithm>
#include "scene/mesh.h"
#include <spdlog/spdlog.h>
#include <MikkTSpace/mikktspace.h>

#include <glm/glm.hpp>
// #include <glm/gtc/quaternion.hpp>
// #include <glm/gtc/type_ptr.hpp>
// #include <glm/gtc/matrix_inverse.hpp>
// #include <glm/gtc/matrix_transform.hpp>

// https://github.com/NVIDIAGameWorks/Falcor/blob/8.0/Source/Falcor/Scene/SceneBuilder.cpp#L65
class MikkTSpaceWrapper
{
public:
    static std::vector<glm::vec4> generateTangents(const Model& mesh)
    {
        //if (!mesh.normals.pData || !mesh.positions.pData ||
        //    !mesh.texCrds.pData || !mesh.pIndices) {
        if (mesh.indices.empty() || mesh.vertices.empty()) {
            spdlog::warn(
                "Can't generate tangent space. The mesh '{}' doesn't have "
                "positions/normals/texCrd/indices.",
                "mesh.name");
            return {};
        }

        // Generate new tangent space.
        SMikkTSpaceInterface mikktspace = {};
        mikktspace.m_getNumFaces = [](const SMikkTSpaceContext* pContext) {
            return ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getFaceCount();
        };
        mikktspace.m_getNumVerticesOfFace =
            [](const SMikkTSpaceContext* pContext, int32_t face) { return 3; };
        mikktspace.m_getPosition = [](const SMikkTSpaceContext* pContext,
                                      float position[], int32_t face,
                                      int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getPosition(position, face, vert);
        };
        mikktspace.m_getNormal = [](const SMikkTSpaceContext* pContext,
                                    float normal[], int32_t face,
                                    int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getNormal(normal, face, vert);
        };
        mikktspace.m_getTexCoord = [](const SMikkTSpaceContext* pContext,
                                      float texCrd[], int32_t face,
                                      int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getTexCrd(texCrd, face, vert);
        };
        mikktspace.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext,
                                         const float tangent[], float sign,
                                         int32_t face, int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->setTSpaceBasic(tangent, sign, face, vert);
        };

        mikktspace.m_setTSpace = [](const SMikkTSpaceContext* pContext, const float fvTangent[],
               const float fvBiTangent[], const float fMagS, const float fMagT,
               const tbool bIsOrientationPreserving, const int face,
               const int vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                    ->setTSpace(fvTangent, fvBiTangent, fMagS, fMagT, bIsOrientationPreserving, face, vert);
        };

        MikkTSpaceWrapper wrapper(mesh);
        SMikkTSpaceContext context = {};
        context.m_pInterface = &mikktspace;
        context.m_pUserData = &wrapper;

        if (genTangSpaceDefault(&context) == false) {
            /*FALCOR_THROW(
                "MikkTSpace failed to generate tangents for the mesh '{}'.",
                mesh.name);*/
            spdlog::error(
                "MikkTSpace failed to generate tangents for the mesh '{}'.",
                "mesh.name");
        }

        return wrapper.mTangents;
    }

    static bool generateTangents(Model& mesh)
    {
        // if (!mesh.normals.pData || !mesh.positions.pData ||
        //     !mesh.texCrds.pData || !mesh.pIndices) {
        if (mesh.indices.empty() || mesh.vertices.empty()) {
            spdlog::warn(
                "Can't generate tangent space. The mesh '{}' doesn't have "
                "positions/normals/texCrd/indices.",
                "mesh.name");
            return {};
        }

        // Generate new tangent space.
        SMikkTSpaceInterface mikktspace = {};
        mikktspace.m_getNumFaces = [](const SMikkTSpaceContext* pContext) {
            return ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getFaceCount();
        };
        mikktspace.m_getNumVerticesOfFace =
            [](const SMikkTSpaceContext* pContext, int32_t face) { return 3; };
        mikktspace.m_getPosition = [](const SMikkTSpaceContext* pContext,
                                      float position[], int32_t face,
                                      int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getPosition(position, face, vert);
        };
        mikktspace.m_getNormal = [](const SMikkTSpaceContext* pContext,
                                    float normal[], int32_t face,
                                    int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getNormal(normal, face, vert);
        };
        mikktspace.m_getTexCoord = [](const SMikkTSpaceContext* pContext,
                                      float texCrd[], int32_t face,
                                      int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->getTexCrd(texCrd, face, vert);
        };
        mikktspace.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext,
                                         const float tangent[], float sign,
                                         int32_t face, int32_t vert) {
            ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                ->setTSpaceBasic(tangent, sign, face, vert);
        };

        mikktspace.m_setTSpace =
            [](const SMikkTSpaceContext* pContext, const float fvTangent[],
               const float fvBiTangent[], const float fMagS, const float fMagT,
               const tbool bIsOrientationPreserving, const int face,
               const int vert) {
                ((MikkTSpaceWrapper*)(pContext->m_pUserData))
                    ->setTSpace(fvTangent, fvBiTangent, fMagS, fMagT,
                                bIsOrientationPreserving, face, vert);
            };

        MikkTSpaceWrapper wrapper(mesh);
        SMikkTSpaceContext context = {};
        context.m_pInterface = &mikktspace;
        context.m_pUserData = &wrapper;

        if (genTangSpaceDefault(&context) == false) {
            /*FALCOR_THROW(
                "MikkTSpace failed to generate tangents for the mesh '{}'.",
                mesh.name);*/
            spdlog::error(
                "MikkTSpace failed to generate tangents for the mesh '{}'.",
                "mesh.name");
            return false;
        }

        for (int i = 0; i < mesh.indices.size(); ++i) {
            mesh.vertices[i].Tangent = glm::vec3(wrapper.mTangents[i]);
            mesh.vertices[i].Bitangent = wrapper.mBiTangents[i];
        }

        return true;
    }

private:
    MikkTSpaceWrapper(const Model& mesh) : mMesh(mesh)
    {
        //FALCOR_ASSERT(mesh.indexCount > 0);
        //mTangents.resize(mesh.indexCount, glm::vec4(0));
        mTangents.resize(mesh.indices.size(), glm::vec4(0));

        mBiTangents.resize(mesh.indices.size(), glm::vec3(0));

        //FALCOR_ASSERT_EQ(mesh.indexCount, mMesh.faceCount * 3);
        //mPositions.resize(mMesh.faceCount * 3);
        mPositions.resize(mMesh.indices.size());
        /*switch (mMesh.positions.frequency) {
            case Mesh::AttributeFrequency::Constant: {
                std::fill_n(mPositions.begin(), mPositions.size(),
                            mMesh.positions.pData[0]);
                break;
            }
            case Mesh::AttributeFrequency::Uniform: {
                for (uint32_t i = 0; i < mMesh.faceCount; ++i)
                    std::fill_n(mPositions.begin() + i * 3, 3,
                                mMesh.positions.pData[i]);
                break;
            }
            case Mesh::AttributeFrequency::Vertex: {
                FALCOR_ASSERT_EQ(mesh.indexCount, mPositions.size());
                for (size_t fvarIdx = 0; fvarIdx < mPositions.size(); ++fvarIdx)
                    mPositions[fvarIdx] =
                        mMesh.positions.pData[mMesh.pIndices[fvarIdx]];
                break;
            }
            case Mesh::AttributeFrequency::FaceVarying: {
                memcpy(mPositions.data(), mMesh.positions.pData,
                       mPositions.size() * sizeof(glm::vec3));
                break;
            }
            default:
                FALCOR_UNREACHABLE();
        }*/

        // vertex size == indices size
        // Mesh::AttributeFrequency::FaceVarying
        for (size_t fvarIdx = 0; fvarIdx < mPositions.size(); ++fvarIdx) {
            mPositions[fvarIdx] = mMesh.vertices[fvarIdx].Position;
        }
    }
    const Model& mMesh;
    std::vector<glm::vec4> mTangents;
    std::vector<glm::vec3> mBiTangents;
    std::vector<glm::vec3> mPositions;

    int32_t getFaceCount() const { 
        //return (int32_t)mMesh.faceCount; 
        return (int32_t)mMesh.indices.size() / 3;
    }
    void getPosition(float position[], int32_t face, int32_t vert) const
    {
        //FALCOR_ASSERT_LT(size_t(face) * 3 + vert, mPositions.size());
        memcpy(position, mPositions.data() + (face * 3 + vert), sizeof(glm::vec3));
    }
    void getNormal(float normal[], int32_t face, int32_t vert)
    {
        *reinterpret_cast<glm::vec3*>(normal) = mMesh.getNormal(face, vert);
    }
    void getTexCrd(float texCrd[], int32_t face, int32_t vert)
    {
        *reinterpret_cast<glm::vec2*>(texCrd) = mMesh.getTexCrd(face, vert);
    }

    void setTSpaceBasic(const float tangent[], float sign, int32_t face,
                    int32_t vert)
    {
        glm::vec3 T = *reinterpret_cast<const glm::vec3*>(tangent);
        mTangents[face * 3 + vert] = glm::vec4(normalize(T), sign);
    }

    void setTSpace(const float fvTangent[], const float fvBiTangent[],
                   const float fMagS, const float fMagT,
                   const tbool bIsOrientationPreserving, const int face,
                   const int vert)
    {
        glm::vec3 T = glm::vec3(fvTangent[0], fvTangent[1], fvTangent[2]);
        mTangents[face * 3 + vert] = glm::vec4(normalize(T), 0.0);

        glm::vec3 B = glm::vec3(fvBiTangent[0], fvBiTangent[1], fvBiTangent[2]);
        mBiTangents[face * 3 + vert] = normalize(B);
    }
};

#endif
