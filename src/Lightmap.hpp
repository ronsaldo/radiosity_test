#ifndef RADIOSITY_TEST_LIGHTMAP_HPP
#define RADIOSITY_TEST_LIGHTMAP_HPP

#include "Object.hpp"
#include "Box2.hpp"
#include "GenericVertex.hpp"
#include "LightState.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <mutex>

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
    glm::vec3 normal;
    size_t texelIndex;
    size_t surfaceIndex;
};

/**
 * A lightmap quad surface. Used for ray casting
 */
class LightmapCompactQuadSurface
{
public:
    glm::vec2 vertices[4];

    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    float distance;
};

/**
 * A lightmap
 */
class Lightmap : public Object
{
public:
    static constexpr float Reflectivity = 0.8f;

    Lightmap();
    ~Lightmap();

    void createBuffers();
    void process(const std::vector<LightState> &lights);
    void computeRadiosityFactors();

    size_t width;
    size_t height;
    std::vector<LightmapPatch> patches;
    std::vector<LightmapCompactQuadSurface> quadSurfaces;
    std::vector<float> viewFactors;
    std::vector<float> viewFactorsDen;

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

    GpuTexturePtr getValidLightmapTexture();

private:
    bool isRayOccluded(glm::vec3 startPoint, size_t startSurfaceIndex,
        glm::vec3 endPoint, size_t endSurfaceIndex = -1);

    void computeDirectLights(const std::vector<LightState> &lights);
    void computeIndirectLightBounce();
    void swapBuffers();

    uint32_t *frontBuffer;
    uint32_t *backBuffer;
    glm::vec4 *directLightBuffer;
    glm::vec4 *indirectLightBuffer;
    glm::vec4 *oldIndirectLightBuffer;
    GpuTexturePtr lightmapTexture;
    std::mutex mutex;
    int uploadedCount;
    int computedCount;
};

struct LightmapQuadSurface
{
    glm::vec3 positions[4];
    glm::vec2 texcoords[4];
    glm::vec3 normal;
    uint32_t indices[4];
    size_t index;
};

/**
 * Lightmap packer. An utility for building a lightmap texture coordinates.
 */
class LightmapPacker
{
public:
    static constexpr size_t MaxTextureWidth = 2048;
    static constexpr float DefaultTexelScale = 0.2;

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
    void buildLightMapPatchesFor(const LightmapPtr &lightmap);
    void buildLightMapPatchesForTriangle(const LightmapPtr &lightmap,
        size_t surfaceIndex,
        const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
        const glm::vec2 &tc1, const glm::vec2 &tc2, const glm::vec2 &tc3,
        const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3
    );

    std::vector<LightmapQuadSurface> quadSurfaces;
    std::vector<bool> usedTexels;
    float texelScale;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_LIGHTMAP_PACKER_HPP
