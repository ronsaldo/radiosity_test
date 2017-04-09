#include "VertexSpecification.hpp"
#include "GpuBuffer.hpp"

namespace RadiosityTest
{
VertexSpecification::VertexSpecification()
{
    glCreateVertexArrays(1, &handle);
}

VertexSpecification::~VertexSpecification()
{
    glDeleteVertexArrays(1, &handle);
}

void VertexSpecification::activate()
{
    glBindVertexArray(handle);
}

void VertexSpecification::setIndexBuffer(const GpuBufferPtr &newIndexBuffer)
{
    indexBuffer = newIndexBuffer;
    glVertexArrayElementBuffer(handle, indexBuffer->getHandle());
}

void VertexSpecification::setVertexBuffers(size_t count, GpuBufferPtr newVertexBuffer[])
{
    vertexBuffers.resize(count);
    for(size_t i = 0; i < count; ++i)
        vertexBuffers[i] = newVertexBuffer[i];
}

void VertexSpecification::setVertexAttribute(size_t bufferIndex, size_t attributeIndex, size_t size, GLenum type, GLint normalized, size_t stride, size_t offset)
{
    glEnableVertexArrayAttrib(handle, attributeIndex);
    glVertexArrayVertexBuffer(handle, attributeIndex, vertexBuffers[bufferIndex]->getHandle(), offset, stride);
    glVertexArrayAttribFormat(handle, attributeIndex, size, type, normalized, 0);
}

} // End of namespace RadiosityTest
