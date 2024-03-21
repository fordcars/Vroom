#pragma once

#include <vector>
#include <glad/glad.h>

// Simple OpenGL buffer wrapper for RAII
class GPUBuffer {
public:
    GPUBuffer();
    ~GPUBuffer();
    GPUBuffer(const GPUBuffer& other);
    GPUBuffer(GPUBuffer&& other) noexcept;
    GPUBuffer& operator=(GPUBuffer other);
    GPUBuffer& operator=(GPUBuffer&& other) noexcept;
    friend void swap(GPUBuffer& first, GPUBuffer& second) noexcept;

    GLuint getId() const;
    size_t getSize() const;

    template<typename T>
    void setData(const std::vector<T>& data) {
        if(data.empty()) return;
        mSize = data.size() * sizeof(T);

        glBindBuffer(GL_ARRAY_BUFFER, mId);
        glBufferData(
            GL_ARRAY_BUFFER,
            mSize,
            data.data(),
            GL_STATIC_DRAW
        );
    }

private:
    GLuint mId = 0;
    size_t mSize = 0;
};