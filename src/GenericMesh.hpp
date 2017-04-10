#ifndef RADIOSITY_TEST_GERNERIC_MESH_HPP
#define RADIOSITY_TEST_GERNERIC_MESH_HPP

#include <stdint.h>
#include "Mesh.hpp"
#include "Lightmap.hpp"
#include "GenericVertex.hpp"

namespace RadiosityTest
{
/**
 * Generic mesh builder
 */
class GenericMeshBuilder
{
public:
    GenericMeshBuilder();
    ~GenericMeshBuilder();

    GenericMeshBuilder &newBaseVertex()
    {
        baseVertex = vertices.size();
        return *this;
    }

    GenericMeshBuilder &addVertex(const GenericVertex &vertex)
    {
        vertices.push_back(vertex);
        return *this;
    }

    GenericMeshBuilder &addPositionNormal(const glm::vec3 &position, const glm::vec3 &normal)
    {
        GenericVertex vertex;
        vertex.position = position;
        vertex.normal = normal;
        vertex.color = currentColor;
        return addVertex(vertex);
    }

    GenericMeshBuilder &addPositionNormalTexcoord(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texcoord)
    {
        GenericVertex vertex;
        vertex.position = position;
        vertex.normal = normal;
        vertex.texcoord = texcoord;
        vertex.color = currentColor;
        return addVertex(vertex);
    }

    GenericMeshBuilder &addIndex(uint32_t index)
    {
        indices.push_back(index + baseVertex);
        return *this;
    }

    GenericMeshBuilder &addQuad(uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4);

    GenericMeshBuilder &beginTriangles()
    {
        beginPrimitives(PrimitiveType::Triangle);
        return *this;
    }

    GenericMeshBuilder &beginPrimitives(PrimitiveType type);

    MeshPtr mesh();

    GenericMeshBuilder &addCube(const glm::vec3 &extent);
    GenericMeshBuilder &addCubeInterior(const glm::vec3 &extent);

    GenericMeshBuilder &setColor(const glm::vec4 &newColor)
    {
        currentColor = newColor;
        return *this;
    }

private:
    void finishLastSubmesh();

    std::vector<GenericVertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Submesh> submeshes;
    uint32_t baseVertex;
    glm::vec4 currentColor;
    LightmapPacker lightmapPacker;
};

} // End of namspace RadiosityTest

#endif //RADIOSITY_TEST_GERNERIC_MESH_HPP
