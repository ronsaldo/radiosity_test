#ifndef RADIOSITY_TEST_VERTEX_SPECIFICATION_HPP
#define RADIOSITY_TEST_VERTEX_SPECIFICATION_HPP

#include "Object.hpp"
#include "GLCommon.hpp"
#include <vector>

namespace RadiosityTest
{
DECLARE_CLASS(VertexSpecification);
DECLARE_CLASS(GpuBuffer);

/**
 * Mesh vertex specification
 */
class VertexSpecification: public Object
{
public:
    VertexSpecification();
    ~VertexSpecification();

    void setIndexBuffer(const GpuBufferPtr &newIndexBuffer);
    void setVertexBuffers(size_t count, GpuBufferPtr newVertexBuffer[]);
    void setVertexAttribute(size_t bufferIndex, size_t attributeIndex, size_t size, GLenum type, GLint normalized, size_t stride, size_t offset);

    void activate();

private:
    GLuint handle;
    GpuBufferPtr indexBuffer;
    std::vector<GpuBufferPtr> vertexBuffers;
};

} // End of RadiosityTest

#endif //RADIOSITY_TEST_VERTEX_SPECIFICATION_HPP
