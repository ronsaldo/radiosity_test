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

    // Build the light map.
    auto lightmap = lightmapPacker.buildLightMap();
    lightmapPacker.applyTexcoordsTo(&vertices[0]);

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
    result->lightmap = lightmap;
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

GenericMeshBuilder &GenericMeshBuilder::addQuad(uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4)
{
    addIndex(i1), addIndex(i2); addIndex(i3);
    addIndex(i3); addIndex(i4); addIndex(i1);

    auto &v1 = vertices[i1 + baseVertex];
    auto &v2 = vertices[i2 + baseVertex];
    auto &v3 = vertices[i3 + baseVertex];
    auto &v4 = vertices[i4 + baseVertex];

    auto &p1 = v1.position;
    auto &p2 = v2.position;
    auto &p3 = v3.position;
    auto &p4 = v4.position;

    lightmapPacker.addQuadSurface(p1, p2, p3, p4,
        i1 + baseVertex, i2 + baseVertex, i3 + baseVertex, i4 + baseVertex);
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
    addQuad(0, 1, 2, 3);

    // -X Left
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(-1.0f, 0.0f, 0.0f));
    addQuad(3, 2, 1, 0);

    // +Y Top
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addQuad(0, 1, 2, 3);

    // -Y Bottom
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addQuad(3, 2, 1, 0);

    // +Z Front
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, 0.0f, 1.0f));
    addQuad(0, 1, 2, 3);

    // -Z Back
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, 0.0f, -1.0f));
    addQuad(3, 2, 1, 0);

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
    addQuad(3, 2, 1, 0);

    // -X Left
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(1.0f, 0.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(1.0f, 0.0f, 0.0f));
    addQuad(0, 1, 2, 3);

    // +Y Top
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, -1.0f, 0.0f));
    addQuad(3, 2, 1, 0);

    // -Y Bottom
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, 1.0f, 0.0f));
    addQuad(0, 1, 2, 3);

    // +Z Front
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, ny, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(px, py, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addPositionNormal(glm::vec3(nx, py, pz), glm::vec3(0.0f, 0.0f, -1.0f));
    addQuad(3, 2, 1, 0);

    // -Z Back
    newBaseVertex();
    addPositionNormal(glm::vec3(nx, ny, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, ny, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(px, py, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addPositionNormal(glm::vec3(nx, py, nz), glm::vec3(0.0f, 0.0f, 1.0f));
    addQuad(0, 1, 2, 3);

    return *this;
}

} // End of namespace RadiosityTest
