/*
 * Type adapters for loader compatibility
 * Maps GLSL-PathTracer types to glm types
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <map>

// Material enums
    enum AlphaMode {
        Opaque,
        Blend,
        Mask
    };
    
    enum MediumType {
        None,
        Absorb,
        Scatter,
        Emissive
    };
    
    enum LightType {
        RectLight,
        SphereLight,
        DistantLight
    };
    
    // Material structure
    struct Material {
        Material() {
            baseColor = glm::vec3(1.0f, 1.0f, 1.0f);
            anisotropic = 0.0f;
            emission = glm::vec3(0.0f, 0.0f, 0.0f);
            metallic = 0.0f;
            roughness = 0.5f;
            subsurface = 0.0f;
            specularTint = 0.0f;
            sheen = 0.0f;
            sheenTint = 0.0f;
            clearcoat = 0.0f;
            clearcoatGloss = 0.0f;
            specTrans = 0.0f;
            ior = 1.5f;
            mediumType = None;
            mediumDensity = 0.0f;
            mediumColor = glm::vec3(1.0f, 1.0f, 1.0f);
            mediumAnisotropy = 0.0f;
            baseColorTexId = -1;
            metallicRoughnessTexID = -1;
            normalmapTexID = -1;
            emissionmapTexID = -1;
            opacity = 1.0f;
            alphaMode = Opaque;
            alphaCutoff = 0.0f;
        }
        
        glm::vec3 baseColor;
        float anisotropic;
        glm::vec3 emission;
        float metallic;
        float roughness;
        float subsurface;
        float specularTint;
        float sheen;
        float sheenTint;
        float clearcoat;
        float clearcoatGloss;
        float specTrans;
        float ior;
        MediumType mediumType;
        float mediumDensity;
        glm::vec3 mediumColor;
        float mediumAnisotropy;
        int baseColorTexId;
        int metallicRoughnessTexID;
        int normalmapTexID;
        int emissionmapTexID;
        float opacity;
        AlphaMode alphaMode;
        float alphaCutoff;
    };
    
    // Light structure
    struct Light {
        glm::vec3 position;
        glm::vec3 emission;
        glm::vec3 u;
        glm::vec3 v;
        float radius;
        float area;
        LightType type;
    };
    
    // Render options
    struct RenderOptions {
        RenderOptions() {
            renderResolution = glm::ivec2(1280, 720);
            windowResolution = glm::ivec2(1280, 720);
            uniformLightCol = glm::vec3(0.3f, 0.3f, 0.3f);
            backgroundCol = glm::vec3(1.0f, 1.0f, 1.0f);
            tileWidth = 100;
            tileHeight = 100;
            maxDepth = 2;
            maxSpp = -1;
            RRDepth = 2;
            texArrayWidth = 2048;
            texArrayHeight = 2048;
            enableRR = true;
            enableTonemap = true;
            enableAces = false;
            openglNormalMap = true;
            enableEnvMap = false;
            enableUniformLight = false;
            hideEmitters = false;
            enableBackground = false;
            transparentBackground = false;
            independentRenderSize = false;
            enableRoughnessMollification = false;
            enableVolumeMIS = false;
            envMapIntensity = 1.0f;
            envMapRot = 0.0f;
            roughnessMollificationAmt = 0.0f;
        }
        
        glm::ivec2 renderResolution;
        glm::ivec2 windowResolution;
        glm::vec3 uniformLightCol;
        glm::vec3 backgroundCol;
        int tileWidth;
        int tileHeight;
        int maxDepth;
        int maxSpp;
        int RRDepth;
        int texArrayWidth;
        int texArrayHeight;
        bool enableRR;
        bool enableTonemap;
        bool enableAces;
        bool openglNormalMap;
        bool enableEnvMap;
        bool enableUniformLight;
        bool hideEmitters;
        bool enableBackground;
        bool transparentBackground;
        bool independentRenderSize;
        bool enableRoughnessMollification;
        bool enableVolumeMIS;
        float envMapIntensity;
        float envMapRot;
        float roughnessMollificationAmt;
    };
    
    // Mesh instance
    struct MeshInstance {
        std::string name;
        int meshID;
        glm::mat4 transform;
        int materialID;
        
        MeshInstance(const std::string& n, int mId, const glm::mat4& xform, int matId)
            : name(n), meshID(mId), transform(xform), materialID(matId) {}
    };

