#ifndef BUFFER_H
#define BUFFER_H

#include "gl460/image_format.h"
#include <glad/gl.h>

#include <glm/glm.hpp>

#include <exception>
#include <functional>
#include <vector>

namespace gl {

class Buffer {
public:
    Buffer();
    Buffer(uint32_t size, const void* data = nullptr);
    template <typename T>
    Buffer(const std::vector<T>& data);
    

    ~Buffer() noexcept;

    operator bool() const noexcept { return m_handle != INVALID; }

    
    const gl::PixelType& dtype() const noexcept { return m_dtype; }

    gl::Handle native_handle() const noexcept { return m_handle; }

    template <typename T>
    void write(const std::vector<T>& data) {
        write(data.data(), sizeof(T) * data.size());
    }

    void write(const void* data, size_t size);

    void immutableStroage();

    void bind(GLenum target);
    void bindIndexed(GLenum target, GLuint index);

private:

    GLenum m_target;


    static constexpr gl::Handle INVALID = 0xFFFFFFFF;

    gl::Handle m_handle{INVALID};
    int m_width;
    int m_height;
    int m_components;
    int m_samples;
    bool m_depth;
    int m_max_level;
    std::reference_wrapper<const gl::PixelType> m_dtype{gl::PixelType::i8};
};

template<typename T>
inline Buffer::Buffer(const std::vector<T>& data) : Buffer(sizeof(T) * data.size(), data.data()) {}

}  // namespace gl

#endif /* TEXTURE_H */
