#ifndef RADIOSITY_TEST_GERNERIC_MESH_HPP
#define RADIOSITY_TEST_GERNERIC_MESH_HPP

#include <glm/glm.hpp>
#include <stdint.h>
#include "Mesh.hpp"

namespace RadiosityTest
{

/**
 * Generic vertex
 */
struct GenericVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 texcoord;
};

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

    GenericMeshBuilder &beginTriangles()
    {
        beginPrimitives(PrimitiveType::Triangle);
        return *this;
    }

    GenericMeshBuilder &beginPrimitives(PrimitiveType type);

    MeshPtr mesh();

    GenericMeshBuilder &addCube(const glm::vec3 &extent);

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
};

} // End of namspace RadiosityTest

#endif //RADIOSITY_TEST_GERNERIC_MESH_HPP
