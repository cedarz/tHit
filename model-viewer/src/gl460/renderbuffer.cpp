#include "gl460/renderbuffer.h"

#include <glad/gl.h>
#include <stdexcept>

gl::Renderbuffer::Renderbuffer(int width,
                               int height,
                               int components,
                               bool depth,
                               const gl::PixelType& dtype,
                               int samples)
    : m_width(width),
      m_height(height),
      m_components(components),
      m_depth(depth),
      m_dtype(dtype),
      m_samples(samples) {
    if (components < 1 || components > 4) {
        throw std::invalid_argument("Components must be 1, 2, 3 or 4");
    } else if (samples & (samples - 1)) {
        throw std::invalid_argument("The number of samples is invalid");
    } else if (depth && dtype != gl::PixelType::f32) {
        throw std::invalid_argument("Depth buffer only supports dtype gl::f32");
    }

    glCreateRenderbuffers(1, &m_handle);

    auto format = depth ? GL_DEPTH_COMPONENT24 : dtype.format(components).first;

    if (samples == 0) {
        glNamedRenderbufferStorage(m_handle, format, width, height);
    } else {
        glNamedRenderbufferStorageMultisample(m_handle, samples, format, width, height);
    }
}

void gl::Renderbuffer::reset() noexcept {
    if (this->operator bool()) {
        glDeleteRenderbuffers(1, &m_handle);
        m_handle = INVALID;
    }
}

void gl::Renderbuffer::swap(gl::Renderbuffer& other) noexcept {
    std::swap(m_handle, other.m_handle);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    std::swap(m_components, other.m_components);
    std::swap(m_depth, other.m_depth);
    std::swap(m_dtype, other.m_dtype);
    std::swap(m_samples, other.m_samples);
}

gl::Renderbuffer::Renderbuffer(gl::Renderbuffer&& other) noexcept {
    swap(other);
}

gl::Renderbuffer& gl::Renderbuffer::operator=(
    gl::Renderbuffer&& other) noexcept {
    swap(other);
    return *this;
}

gl::Renderbuffer::operator bool() const noexcept {
    return m_handle != INVALID;
}

int gl::Renderbuffer::width() const noexcept {
    return m_width;
}

/**
 * The height of the render buffer.
 */
int gl::Renderbuffer::height() const noexcept {
    return m_height;
}

/**
 * The number of components per pixel.
 */
int gl::Renderbuffer::components() const noexcept {
    return m_components;
}

/**
 * Determine whether this render buffer is a depth buffer.
 */
bool gl::Renderbuffer::is_depth_buffer() const noexcept {
    return m_depth;
}

/**
 * The samples of the render buffer.
 */
int gl::Renderbuffer::samples() const noexcept {
    return m_samples;
}

/**
 * The data type of the buffer.
 */
const gl::PixelType& gl::Renderbuffer::dtype() const noexcept {
    return m_dtype;
}

gl::Handle gl::Renderbuffer::native_handle() const noexcept {
    return m_handle;
}
