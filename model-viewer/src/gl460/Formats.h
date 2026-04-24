#ifndef Formats_H
#define Formats_H
#include <cstdint>
#include <utility>
#include <glad/gl.h>

enum class TexType : uint32_t
{
    Buffer,                 ///< Buffer. Can be bound to all shader-stages
    Texture1D,              ///< 1D texture. Can be bound as render-target, shader-resource and UAV
    Texture2D,              ///< 2D texture. Can be bound as render-target, shader-resource and UAV
    Texture3D,              ///< 3D texture. Can be bound as render-target, shader-resource and UAV
    TextureCube,            ///< Texture-cube. Can be bound as render-target, shader-resource and UAV
    Texture2DMultisample,   ///< 2D multi-sampled texture. Can be bound as render-target, shader-resource and UAV
};

enum class ResourceBindFlags : uint32_t
{
    None = 0x0,             ///< The resource will not be bound the pipeline. Use this to create a staging resource
    Vertex = 0x1,           ///< The resource will be bound as a vertex-buffer
    Index = 0x2,            ///< The resource will be bound as a index-buffer
    Constant = 0x4,         ///< The resource will be bound as a constant-buffer
    StreamOutput = 0x8,     ///< The resource will be bound to the stream-output stage as an output buffer
    ShaderResource = 0x10,  ///< The resource will be bound as a shader-resource
    UnorderedAccess = 0x20, ///< The resource will be bound as an UAV
    RenderTarget = 0x40,    ///< The resource will be bound as a render-target
    DepthStencil = 0x80,    ///< The resource will be bound as a depth-stencil buffer
    IndirectArg = 0x100,    ///< The resource will be bound as an indirect argument buffer
    Shared = 0x200,    ///< The resource will be shared with a different adapter. Mostly useful for sharing resoures with CUDA
    AccelerationStructure = 0x80000000,  ///< The resource will be bound as an acceleration structure

    AllColorViews = ShaderResource | UnorderedAccess | RenderTarget,
    AllDepthViews = ShaderResource | DepthStencil
};

// "Difference between format and internalformat"
// https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat
struct FormatDesc
{
    GLint internal_format; // internalFormat defines the format that OpenGL should use to store the data internally.
    GLenum format;
    GLenum type;
};

enum class ResourceFormat : uint32_t
{
    Unknown,
    R8Unorm,
    R8Snorm,
    R16Unorm,
    R16Snorm,
    RG8Unorm,
    RG8Snorm,
    RG16Unorm,
    RG16Snorm,
    RGB16Unorm,
    RGB16Snorm,
    R24UnormX8,
    RGB5A1Unorm,
    RGBA8Unorm,
    RGBA8Snorm,
    RGB10A2Unorm,
    RGB10A2Uint,
    RGBA16Unorm,
    RGBA8UnormSrgb,
    R16Float,
    RG16Float,
    RGB16Float,
    RGBA16Float,
    R32Float,
    R32FloatX32,
    RG32Float,
    RGB32Float,
    RGBA32Float,
    R11G11B10Float,
    RGB9E5Float,
    R8Int,
    R8Uint,
    R16Int,
    R16Uint,
    R32Int,
    R32Uint,
    RG8Int,
    RG8Uint,
    RG16Int,
    RG16Uint,
    RG32Int,
    RG32Uint,
    RGB16Int,
    RGB16Uint,
    RGB32Int,
    RGB32Uint,
    RGBA8Int,
    RGBA8Uint,
    RGBA16Int,
    RGBA16Uint,
    RGBA32Int,
    RGBA32Uint,

    BGRA8Unorm,
    BGRA8UnormSrgb,

    BGRX8Unorm,
    BGRX8UnormSrgb,
    Alpha8Unorm,
    Alpha32Float,
    R5G6B5Unorm,

    // Depth-stencil
    D32Float,
    D16Unorm,
    D32FloatS8X24,
    D24UnormS8,
    D24Unorm,

    // Compressed formats
    BC1Unorm,   // DXT1
    BC1UnormSrgb,
    BC2Unorm,   // DXT3
    BC2UnormSrgb,
    BC3Unorm,   // DXT5
    BC3UnormSrgb,
    BC4Unorm,   // RGTC Unsigned Red
    BC4Snorm,   // RGTC Signed Red
    BC5Unorm,   // RGTC Unsigned RG
    BC5Snorm,   // RGTC Signed RG
    BC6HS16,
    BC6HU16,
    BC7Unorm,
    BC7UnormSrgb,

    Count
};

GLenum getResourceDimension(TexType type);
FormatDesc getResourceFormatDesc(ResourceFormat format);
GLint getResourceInternalFormat(ResourceFormat format);
GLenum getResourceDataFormat(ResourceFormat format);
GLenum getResourceDataType(ResourceFormat format);

#endif // !Formats_H
