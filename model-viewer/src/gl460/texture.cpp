#include <gl460/texture.h>
#include <gl460/gl_enum_string_helper.h>

#include <glad/gl.h>

#include <cassert>
#include <stdexcept>

#include <spdlog/spdlog.h>

gl::Texture::SharedPtr gl::Texture::create2D(uint32_t width, uint32_t height, ResourceFormat format, uint32_t arraySize, uint32_t mipLevels, const void* pInitData, ResourceBindFlags bindFlags)
{
    Texture::SharedPtr ptex = SharedPtr(new Texture(width, height, 1, arraySize, mipLevels, 0, format, TexType::Texture2D, bindFlags));
    glTextureStorage2D(ptex->m_api_handle, 1, getResourceInternalFormat(format), width, height);
    glTextureParameteri(ptex->m_api_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(ptex->m_api_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /*glGenerateTextureMipmap(ptex->m_api_handle);
    int maxlevel = -1;
    glGetTextureParameteriv(ptex->m_api_handle, GL_TEXTURE_MAX_LEVEL, &maxlevel);
    glGetTextureParameteriv(ptex->m_api_handle, GL_TEXTURE_MAX_LOD, &maxlevel);
    if (depth) {
        glTextureParameteri(ptex->m_api_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(ptex->m_api_handle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }*/

    if (pInitData) {
        bool autoGenMips = mipLevels == kMaxPossible;
        glTextureSubImage2D(ptex->m_api_handle, (autoGenMips ? ptex->m_mip_levels : 0), 0, 0, ptex->m_width, ptex->m_height, getResourceDataFormat(ptex->m_format), getResourceDataType(ptex->m_format), pInitData);
        if (autoGenMips)
        {
            glGenerateTextureMipmap(ptex->m_api_handle);
        }
    }
    return ptex;
}

gl::Texture::Texture(uint32_t width, uint32_t height, uint32_t depth, uint32_t arraySize, uint32_t mipLevels, uint32_t sampleCount, ResourceFormat format, TexType type, ResourceBindFlags bindFlags)
    : m_width(width), m_height(height), m_depth(depth), m_mip_levels(mipLevels), m_sample_count(sampleCount), m_array_size(arraySize), m_format(format)
{
    if (m_mip_levels == kMaxPossible)
    {
        uint32_t dims = width | height | depth;
        m_mip_levels = static_cast<uint32_t>(log(dims)) + 1;
    }
    glCreateTextures(getResourceDimension(type), 1, &m_api_handle);
    //glGenTextures(1, &m_api_handle);
    //glBindTexture(GL_TEXTURE_2D, m_api_handle);

    //glPixelStorei(GL_PACK_ALIGNMENT, 1);
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //spdlog::info("gl enum: {}", string_gl_enum(dtype.type()));
    //spdlog::info("gl enum: {}", string_gl_enum(internal_format));
}

gl::Texture::Texture(int width,
                     int height,
                     int components,
                     bool depth,
                     const gl::PixelType& dtype,
                     const void* data,
                     int samples,
                     int alignment)
    : m_width(width),
      m_height(height),
      m_components(components),
      m_sample_count(samples),
      is_depth(depth),
      m_dtype(dtype),
      m_mip_levels(0) {
    if (components < 1 || components > 4) {
        throw std::invalid_argument("Components must be 1, 2, 3 or 4");
    } else if (samples & (samples - 1)) {
        throw std::invalid_argument("The number of samples is invalid");
    } else if (data != nullptr && samples) {
        throw std::invalid_argument("Multisample textures are not writable directly");
    } else if (alignment != 1 && alignment != 2 && alignment != 4 && alignment != 8) {
        throw std::invalid_argument("Alignment must be 1, 2, 4 or 8");
    } else if (depth && dtype != gl::PixelType::f32) {
        throw std::invalid_argument("Depth buffer only supports dtype gl::f32");
    }

    size_t expected_size = static_cast<size_t>(width) * static_cast<size_t>(components) * dtype.size();
    expected_size = (expected_size + static_cast<size_t>(alignment) - 1) / static_cast<size_t>(alignment) *
                    static_cast<size_t>(alignment);
    expected_size = expected_size * static_cast<size_t>(height);

    int pixel_type = dtype.type();
    auto [base_format, internal_format] = dtype.format(components);
    GLenum texture_target = m_sample_count ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    glCreateTextures(texture_target, 1, &m_api_handle);

    if (samples) {
        glTextureStorage2DMultisample(m_api_handle, samples, depth ? GL_DEPTH_COMPONENT24 : internal_format, width, height,
                                      true);
    } else {
        glPixelStorei(GL_PACK_ALIGNMENT, alignment);
        glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        spdlog::info("gl enum: {}", string_gl_enum(dtype.type()));
        spdlog::info("gl enum: {}", string_gl_enum(internal_format));

        glTextureStorage2D(m_api_handle, 1, depth ? GL_DEPTH_COMPONENT24 : internal_format, width, height);
        glTextureParameteri(m_api_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_api_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateTextureMipmap(m_api_handle);
        int maxlevel = -1;
        glGetTextureParameteriv(m_api_handle, GL_TEXTURE_MAX_LEVEL, &maxlevel);
        glGetTextureParameteriv(m_api_handle, GL_TEXTURE_MAX_LOD, &maxlevel);

        if (depth) {
            glTextureParameteri(m_api_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(m_api_handle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        }

        if (data) {
            glTextureSubImage2D(m_api_handle, 0, 0, 0, width, height, base_format, pixel_type, data);
        }
    }
}

void gl::Texture::reset() noexcept {
    if (this->operator bool()) {
        glDeleteTextures(1, &m_api_handle);
        m_api_handle = INVALID;
    }
}

void gl::Texture::swap(gl::Texture& other) noexcept {
    std::swap(m_api_handle, other.m_api_handle);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_components, other.m_components);
    std::swap(m_sample_count, other.m_sample_count);
    std::swap(m_depth, other.m_depth);
    std::swap(m_dtype, other.m_dtype);
    std::swap(m_mip_levels, other.m_mip_levels);
}

gl::Texture::Texture(gl::Texture&& other) noexcept {
    swap(other);
}

gl::Texture& gl::Texture::operator=(gl::Texture&& other) noexcept {
    swap(other);
    return *this;
}

void gl::Texture::write(const void* data, size_t size, int level, int alignment) {
    assert(this->operator bool());

    if (alignment != 1 && alignment != 2 && alignment != 4 && alignment != 8) {
        throw std::invalid_argument("Alignment must be 1, 2, 4 or 8");
    } else if (level > m_mip_levels) {
        throw std::invalid_argument("Invalid level");
    } else if (m_sample_count) {
        throw std::logic_error("Multisample textures are not writable directly");
    }

    int width = m_width / (1 << level);
    int height = m_height / (1 << level);

    width = width > 1 ? width : 1;
    height = height > 1 ? height : 1;

    spdlog::info("type: {}", string_gl_enum(getResourceDataType(m_format)));
    spdlog::info("format: {}", string_gl_enum(getResourceDataFormat(m_format)));

    glPixelStorei(GL_PACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    glTextureSubImage2D(m_api_handle, 0, 0, 0, width, height, getResourceDataFormat(m_format), getResourceDataType(m_format), data);
    glTextureParameteri(m_api_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_api_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void gl::Texture::use(unsigned slot) {
    assert(this->operator bool());

    glBindTextureUnit(slot, m_api_handle);
}
