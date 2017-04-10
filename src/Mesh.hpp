#ifndef RADIOSITY_TEST_MESH_HPP
#define RADIOSITY_TEST_MESH_HPP

#include "Object.hpp"
#include <vector>

namespace RadiosityTest
{
DECLARE_CLASS(GpuBuffer);
DECLARE_CLASS(Mesh);
DECLARE_CLASS(VertexSpecification);
DECLARE_CLASS(Lightmap);

/**
 * A primitive type
 */
enum class PrimitiveType
{
    Point=0,
    Line,
    Triangle,
};

/**
 * A submesh
 */
struct Submesh
{
    PrimitiveType primitiveType;
    size_t indexCount;
    size_t startIndex;
};

/**
 * A mesh
 */
class Mesh : public Object
{
public:
    GpuBufferPtr vertexBuffer;
    GpuBufferPtr indexBuffer;
    VertexSpecificationPtr vertexSpecification;
    LightmapPtr lightmap;
    std::vector<Submesh> submeshes;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_MESH_HPP
