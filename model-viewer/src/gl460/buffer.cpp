#include <gl460/buffer.h>
#include <gl460/gl_enum_string_helper.h>

#include <glad/gl.h>

#include <cassert>
#include <stdexcept>

#include <spdlog/spdlog.h>

gl::Buffer::~Buffer() noexcept
{
    glDeleteBuffers(1, &m_handle);
}

gl::Buffer::Buffer()
{
    glCreateBuffers(1, &m_handle);
}

gl::Buffer::Buffer(uint32_t size, const void* data)
{
    glCreateBuffers(1, &m_handle);
    glNamedBufferStorage(m_handle, size, data, GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
}

void gl::Buffer::write(const void* data, size_t size)
{
    //glBufferSubData()
    glNamedBufferSubData(m_handle, 0, size, data);
}

void gl::Buffer::bindIndexed(GLenum target, GLuint index)
{
    glBindBufferBase(target, index, m_handle);
}
