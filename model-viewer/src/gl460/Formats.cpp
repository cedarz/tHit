#include "gl460/Formats.h"

GLenum getResourceDimension(TexType type)
{
    switch (type)
    {
    case TexType::Buffer:
        break;
    case TexType::Texture1D:
        break;
    case TexType::Texture2D:
        return GL_TEXTURE_2D;
        break;
    case TexType::Texture3D:
        break;
    case TexType::TextureCube:
        break;
    case TexType::Texture2DMultisample:
        break;
    default:
        break;
    }

    return GL_TEXTURE_2D;
}

FormatDesc getResourceFormatDesc(ResourceFormat format)
{
    FormatDesc desc;
    switch (format)
    {
    case ResourceFormat::Unknown:
        break;
    case ResourceFormat::R8Unorm:
        break;
    case ResourceFormat::R8Snorm:
        break;
    case ResourceFormat::R16Unorm:
        break;
    case ResourceFormat::R16Snorm:
        break;
    case ResourceFormat::RG8Unorm:
        break;
    case ResourceFormat::RG8Snorm:
        break;
    case ResourceFormat::RG16Unorm:
        break;
    case ResourceFormat::RG16Snorm:
        break;
    case ResourceFormat::RGB16Unorm:
        break;
    case ResourceFormat::RGB16Snorm:
        break;
    case ResourceFormat::R24UnormX8:
        break;
    case ResourceFormat::RGB5A1Unorm:
        break;
    case ResourceFormat::RGBA8Unorm:
        desc.internal_format = GL_RGBA8;
        desc.format = GL_RGBA;
        desc.type = GL_UNSIGNED_BYTE;
        break;
    case ResourceFormat::RGBA8Snorm:
        break;
    case ResourceFormat::RGB10A2Unorm:
        break;
    case ResourceFormat::RGB10A2Uint:
        break;
    case ResourceFormat::RGBA16Unorm:
        break;
    case ResourceFormat::RGBA8UnormSrgb:
        break;
    case ResourceFormat::R16Float:
        break;
    case ResourceFormat::RG16Float:
        break;
    case ResourceFormat::RGB16Float:
        break;
    case ResourceFormat::RGBA16Float:
        break;
    case ResourceFormat::R32Float:
        break;
    case ResourceFormat::R32FloatX32:
        break;
    case ResourceFormat::RG32Float:
        break;
    case ResourceFormat::RGB32Float:
        break;
    case ResourceFormat::RGBA32Float:
        break;
    case ResourceFormat::R11G11B10Float:
        break;
    case ResourceFormat::RGB9E5Float:
        break;
    case ResourceFormat::R8Int:
        break;
    case ResourceFormat::R8Uint:
        break;
    case ResourceFormat::R16Int:
        break;
    case ResourceFormat::R16Uint:
        break;
    case ResourceFormat::R32Int:
        break;
    case ResourceFormat::R32Uint:
        desc.internal_format = GL_R32UI;
        desc.format = GL_RED_INTEGER;
        desc.type = GL_UNSIGNED_INT;
        break;
    case ResourceFormat::RG8Int:
        break;
    case ResourceFormat::RG8Uint:
        break;
    case ResourceFormat::RG16Int:
        break;
    case ResourceFormat::RG16Uint:
        break;
    case ResourceFormat::RG32Int:
        break;
    case ResourceFormat::RG32Uint:
        break;
    case ResourceFormat::RGB16Int:
        break;
    case ResourceFormat::RGB16Uint:
        break;
    case ResourceFormat::RGB32Int:
        break;
    case ResourceFormat::RGB32Uint:
        break;
    case ResourceFormat::RGBA8Int:
        break;
    case ResourceFormat::RGBA8Uint:
        break;
    case ResourceFormat::RGBA16Int:
        break;
    case ResourceFormat::RGBA16Uint:
        break;
    case ResourceFormat::RGBA32Int:
        break;
    case ResourceFormat::RGBA32Uint:
        break;
    case ResourceFormat::BGRA8Unorm:
        break;
    case ResourceFormat::BGRA8UnormSrgb:
        break;
    case ResourceFormat::BGRX8Unorm:
        break;
    case ResourceFormat::BGRX8UnormSrgb:
        break;
    case ResourceFormat::Alpha8Unorm:
        break;
    case ResourceFormat::Alpha32Float:
        break;
    case ResourceFormat::R5G6B5Unorm:
        break;
    case ResourceFormat::D32Float:
        break;
    case ResourceFormat::D16Unorm:
        break;
    case ResourceFormat::D32FloatS8X24:
        break;
    case ResourceFormat::D24UnormS8:
        break;
    case ResourceFormat::D24Unorm:
        desc.internal_format = GL_DEPTH_COMPONENT24;
        desc.format = GL_DEPTH_COMPONENT;
        desc.type = GL_UNSIGNED_INT;
        break;
    case ResourceFormat::BC1Unorm:
        break;
    case ResourceFormat::BC1UnormSrgb:
        break;
    case ResourceFormat::BC2Unorm:
        break;
    case ResourceFormat::BC2UnormSrgb:
        break;
    case ResourceFormat::BC3Unorm:
        break;
    case ResourceFormat::BC3UnormSrgb:
        break;
    case ResourceFormat::BC4Unorm:
        break;
    case ResourceFormat::BC4Snorm:
        break;
    case ResourceFormat::BC5Unorm:
        break;
    case ResourceFormat::BC5Snorm:
        break;
    case ResourceFormat::BC6HS16:
        break;
    case ResourceFormat::BC6HU16:
        break;
    case ResourceFormat::BC7Unorm:
        break;
    case ResourceFormat::BC7UnormSrgb:
        break;
    case ResourceFormat::Count:
        break;
    default:
        break;
    }
    return desc;
}

GLint getResourceInternalFormat(ResourceFormat format)
{
    return getResourceFormatDesc(format).internal_format;
}

GLenum getResourceDataFormat(ResourceFormat format)
{
    return getResourceFormatDesc(format).format;
}

GLenum getResourceDataType(ResourceFormat format)
{
    return getResourceFormatDesc(format).type;
}
