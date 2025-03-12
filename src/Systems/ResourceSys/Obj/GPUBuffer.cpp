#include "GPUBuffer.hpp"

#include <utility>

GPUBuffer::GPUBuffer() { glGenBuffers(1, &mId); }

GPUBuffer::~GPUBuffer() { glDeleteBuffers(1, &mId); }

GPUBuffer::GPUBuffer(const GPUBuffer& other) : mSize(other.mSize) {
    glGenBuffers(1, &mId);
    if(mSize == 0) return;

    // Bind the source buffer for reading
    glBindBuffer(GL_COPY_READ_BUFFER, other.mId);
    // Allocate data for the new buffer
    glBindBuffer(GL_COPY_WRITE_BUFFER, mId);
    glBufferData(GL_COPY_WRITE_BUFFER, mSize, nullptr, GL_STATIC_DRAW);

    // Copy data
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, mSize);

    // Unbind buffers
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept : GPUBuffer() { swap(*this, other); }

GPUBuffer& GPUBuffer::operator=(GPUBuffer other) {
    // Copy-swap idiom
    swap(*this, other);
    return *this;
}

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other) noexcept {
    swap(*this, other);
    return *this;
}

void swap(GPUBuffer& first, GPUBuffer& second) noexcept {
    std::swap(first.mId, second.mId);
    std::swap(first.mCount, second.mCount);
    std::swap(first.mSize, second.mSize);
}

GLuint GPUBuffer::getId() const { return mId; }

// Returns the number of elements in buffer
size_t GPUBuffer::getCount() const { return mCount; }

// Returns the size of the contents of the buffer
size_t GPUBuffer::getSize() const { return mSize; }