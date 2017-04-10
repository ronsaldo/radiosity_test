#ifndef RADIOSITY_TEST_LIGHTMAP_HPP
#define RADIOSITY_TEST_LIGHTMAP_HPP

#include "Object.hpp"
#include "Box2.hpp"
#include "GenericVertex.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace RadiosityTest
{
DECLARE_CLASS(LightmapPatch);
DECLARE_CLASS(Lightmap);
DECLARE_CLASS(LightmapPacker);
DECLARE_CLASS(GpuTexture);

/**
 * A lightmap patch
 */
class LightmapPatch
{
public:
    glm::vec3 position;
    glm::ivec2 texcoord;
};

/**
 * A lightmap
 */
class Lightmap : public Object
{
public:
    Lightmap();
    ~Lightmap();

    void createBuffers();

    size_t width;
    size_t height;
    std::vector<LightmapPatch> patches;

    uint32_t *getFrontBuffer() const
    {
        return frontBuffer;
    }

    uint32_t *getBackBuffer() const
    {
        return backBuffer;
    }

    GpuTexturePtr getLightmapTexture() const
    {
        return lightmapTexture;
    }

private:
    uint32_t *frontBuffer;
    uint32_t *backBuffer;
    GpuTexturePtr lightmapTexture;
};

struct LightmapQuadSurface
{
    glm::vec3 positions[4];
    glm::vec2 texcoords[4];
    glm::vec2 normal;
    uint32_t indices[4];
};

/**
 * Lightmap packer. An utility for building a lightmap texture coordinates.
 */
class LightmapPacker
{
public:
    static constexpr size_t MaxTextureWidth = 2048;
    static constexpr float DefaultTexelScale = 0.1;

    LightmapPacker();
    ~LightmapPacker();

    void addQuadSurface(
        const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4,
        uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4
    );

    LightmapPtr buildLightMap();

    void applyTexcoordsTo(GenericVertex *vertices)
    {
        for(auto &surface : quadSurfaces)
        {
            for(int i = 0; i < 4; ++i)
                vertices[surface.indices[i]].texcoord = surface.texcoords[i];
        }
    }

private:
    std::vector<LightmapQuadSurface> quadSurfaces;
    float texelScale;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_LIGHTMAP_PACKER_HPP
