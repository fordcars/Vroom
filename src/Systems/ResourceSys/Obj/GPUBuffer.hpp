#pragma once

#include <glad/glad.h>

#include <vector>

// Simple OpenGL buffer wrapper for RAII
class GPUBuffer {
public:
    GPUBuffer();
    template <typename T>
    GPUBuffer(GLenum target, const std::vector<T>& data,
              GLenum usageHint = GL_STATIC_DRAW)
        : GPUBuffer() {
        setData(target, data, usageHint);
    }

    ~GPUBuffer();
    GPUBuffer(const GPUBuffer& other);
    GPUBuffer(GPUBuffer&& other) noexcept;
    GPUBuffer& operator=(GPUBuffer other);
    GPUBuffer& operator=(GPUBuffer&& other) noexcept;
    friend void swap(GPUBuffer& first, GPUBuffer& second) noexcept;

    GLuint getId() const;
    size_t getCount() const;
    size_t getSize() const;

    template <typename T>
    void setData(GLenum target, const std::vector<T>& data,
                 GLenum usageHint = GL_STATIC_DRAW) {
        if(data.empty()) return;

        glBindBuffer(target, mId);
        glBufferData(target, data.size() * sizeof(T), data.data(), usageHint);

        mCount = data.size();
        mSize = data.size() * sizeof(T);
    }

private:
    GLuint mId = 0;
    size_t mCount = 0;
    size_t mSize = 0;
};