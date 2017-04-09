#include "GenericMesh.hpp"
#include "GpuBuffer.hpp"
#include "VertexSpecification.hpp"

namespace RadiosityTest
{

GenericMeshBuilder::GenericMeshBuilder()
{
    baseVertex = 0;
    currentColor = glm::vec4(1.0, 1.0, 1.0, 1.0);
}

GenericMeshBuilder::~GenericMeshBuilder()
{
}

MeshPtr GenericMeshBuilder::mesh()
{
    finishLastSubmesh();

    // Create the vertex buffer
    auto vertexBuffer = std::make_shared<GpuBuffer> ();
    vertexBuffer->setImmutableContent(sizeof(vertices[0])*vertices.size(), &vertices[0]);

    // Create the index buffer
    auto indexBuffer = std::make_shared<GpuBuffer> ();
    indexBuffer->setImmutableContent(sizeof(indices[0])*indices.size(), &indices[0]);

    // Create the vertex specification.
    auto vertexSpecification = std::make_shared<VertexSpecification> ();
    vertexSpecification->setIndexBuffer(indexBuffer);
    vertexSpecification->setVertexBuffers(1, &vertexBuffer);
    vertexSpecification->setVertexAttribute(0, 0, 3, GL_FLOAT, GL_FALSE, sizeof(GenericVertex), offsetof(GenericVertex, position));
    vertexSpecification->setVertexAttribute(0, 1, 4, GL_FLOAT, GL_FALSE, sizeof(GenericVertex), offsetof(GenericVertex, color));
    vertexSpecification->setVertexAttribute(0, 2, 2, GL_FLOAT, GL_FALSE, sizeof(GenericVertex), offsetof(GenericVertex, texcoord));
    vertexSpecification->setVertexAttribute(0, 3, 3, GL_FLOAT, GL_FALSE, sizeof(GenericVertex), offsetof(GenericVertex, normal));

    // Create the mesh object
    auto result = std::make_shared<Mesh> ();
    result->vertexBuffer = vertexBuffer;
    result->indexBuffer = indexBuffer;
    result->vertexSpecification = vertexSpecification;
    result->submeshes = submeshes;
    return result;
}

void GenericMeshBuilder::finishLastSubmesh()
{
    if(submeshes.empty())
        return;

    auto &submesh = submeshes.back();
    submesh.indexCount = indices.size() - submesh.startIndex;
}

GenericMeshBuilder &GenericMeshBuilder::beginPrimitives(PrimitiveType type)
{
    // Avoid starting a new submesh, if possible.
    if(!submeshes.empty())
    {
        auto &lastSubmesh = submeshes.back();
        if(lastSubmesh.primitiveType == type)
        {
            newBaseVertex();
            return *this;
        }
    }

    // Start a new submesh.
    finishLastSubmesh();
    Submesh submesh;
    submesh.primitiveType = type;
    submesh.startIndex = indices.size();
    submesh.indexCount = 0;
    submeshes.push_back(submesh);
    return *this;
}

GenericMeshBuilder &GenericMeshBuilder::addCube(const glm::vec3 &extent)
{
    auto halfExtent = extent*0.5f;
    auto px = halfExtent.x; auto py = halfExtent.y; auto pz = halfExtent.z;
    auto nx = -halfExtent.x; auto ny = -halfExtent.y; auto nz = -halfExtent.z;

    beginTriangles();

    // +X Right
    newBaseVertex();
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(1.0f, 0.0f, 0.0f));
    addIndex(0); addIndex(1); addIndex(2);
    addIndex(2); addIndex(3); addIndex(0);

    // -X Left
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addIndex(2); addIndex(1); addIndex(0);
    addIndex(0); addIndex(3); addIndex(2);

    // +Y Top
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addIndex(0); addIndex(1); addIndex(2);
    addIndex(2); addIndex(3); addIndex(0);

    // -Y Bottom
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addIndex(2); addIndex(1); addIndex(0);
    addIndex(0); addIndex(3); addIndex(2);

    // +Z Front
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addIndex(0); addIndex(1); addIndex(2);
    addIndex(2); addIndex(3); addIndex(0);

    // -Z Back
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addIndex(2); addIndex(1); addIndex(0);
    addIndex(0); addIndex(3); addIndex(2);

    return *this;
}

GenericMeshBuilder &GenericMeshBuilder::addCubeInterior(const glm::vec3 &extent)
{
    auto halfExtent = extent*0.5f;
    auto px = halfExtent.x; auto py = halfExtent.y; auto pz = halfExtent.z;
    auto nx = -halfExtent.x; auto ny = -halfExtent.y; auto nz = -halfExtent.z;

    beginTriangles();

    // +X Right
    newBaseVertex();
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addIndex(2); addIndex(1); addIndex(0);
    addIndex(0); addIndex(3); addIndex(2);

    // -X Left
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(1.0f, 0.0f, 0.0f));
    addIndex(0); addIndex(1); addIndex(2);
    addIndex(2); addIndex(3); addIndex(0);

    // +Y Top
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addIndex(2); addIndex(1); addIndex(0);
    addIndex(0); addIndex(3); addIndex(2);

    // -Y Bottom
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addIndex(0); addIndex(1); addIndex(2);
    addIndex(2); addIndex(3); addIndex(0);

    // +Z Front
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addIndex(2); addIndex(1); addIndex(0);
    addIndex(0); addIndex(3); addIndex(2);

    // -Z Back
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addIndex(0); addIndex(1); addIndex(2);
    addIndex(2); addIndex(3); addIndex(0);

    return *this;
}

} // End of namespace RadiosityTest
