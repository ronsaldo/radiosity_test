#include "Lightmap.hpp"
#include "GpuTexture.hpp"
#include <random>
#include <string.h>

namespace RadiosityTest
{
static glm::vec3 Directions[] = {
    glm::vec3(1.0, 0.0, 0.0),
    glm::vec3(-1.0, 0.0, 0.0),
    glm::vec3(0.0, 1.0, 0.0),
    glm::vec3(0.0, -1.0, 0.0),
    glm::vec3(0.0, 0.0, 1.0),
    glm::vec3(0.0, 0.0, -1.0),

    // Duplicated to avoid module
    glm::vec3(1.0, 0.0, 0.0),
    glm::vec3(-1.0, 0.0, 0.0),
    glm::vec3(0.0, 1.0, 0.0),
    glm::vec3(0.0, -1.0, 0.0),
    glm::vec3(0.0, 0.0, 1.0),
    glm::vec3(0.0, 0.0, -1.0),
};

static int findBestMatchingDirection(glm::vec3 &vector)
{
    int bestIndex = 0;
    float bestMatch = -FP_INFINITE;
    for(int i = 0; i < 6; ++i)
    {
        float lambda = glm::dot(vector, Directions[i]);
        if(lambda > bestMatch)
        {
            bestIndex = i;
            bestMatch = lambda;
        }
    }

    return bestIndex;
}

static glm::vec2 projectToPlanes(const glm::vec3 &position, const glm::vec3 &u, const glm::vec3 &v)
{
    return glm::vec2(glm::dot(position, u), glm::dot(position, v));
}

inline size_t roundToNextPowerOfTwo(size_t value)
{
    size_t result = 1;
    while(result < value)
        result <<= size_t(1);
    return result;
}

inline float edgeOrientation(const glm::vec2 &e1, const glm::vec2 &e2, const glm::vec2 &p)
{
    auto u = e2 - e1;
    auto v = p - e1;
    return u.x * v.y -  u.y * v.x;
}

Lightmap::Lightmap()
    : frontBuffer(nullptr), backBuffer(nullptr)
{
}

Lightmap::~Lightmap()
{
    delete [] frontBuffer;
    delete [] backBuffer;
}

void Lightmap::createBuffers()
{
    frontBuffer = new uint32_t[width*height];
    backBuffer = new uint32_t[width*height];
    memset(frontBuffer, 0, width*height*4);
    memset(backBuffer, 0, width*height*4);

    /*std::mt19937 random;
    random.seed(0);

    for(size_t i = 0; i < width*height; ++i)
    {
        uint8_t gray = random() & 0xFF;
        uint32_t color = gray | (gray << 8) | (gray << 16) | (0xFF << 24);
        frontBuffer[i] = backBuffer[i] = color;
    }
    */
    for(auto &patch : patches)
    {
        auto lightDistance = glm::length(glm::vec3(0.0, 1.0, 0.0) - patch.position);
        auto lightAmount = glm::clamp(1.0f - lightDistance / 3, 0.0f, 1.0f);
        uint8_t gray = glm::clamp(int(lightAmount *255), 0, 255);
        uint32_t color = gray | (gray << 8) | (gray << 16) | (0xFF << 24);
        frontBuffer[patch.texelIndex] = backBuffer[patch.texelIndex] = color;
    }

    lightmapTexture = std::make_shared<GpuTexture> (GL_TEXTURE_2D);
    lightmapTexture->setStorage2D(1, width, height, GL_RGBA8);
    lightmapTexture->setNearestFiltering();
    //lightmapTexture->setLinearFiltering();
    lightmapTexture->clampToEdge();
    lightmapTexture->uploadLevel(0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frontBuffer);
}

LightmapPacker::LightmapPacker()
{
    texelScale = DefaultTexelScale;
}

LightmapPacker::~LightmapPacker()
{
}

void LightmapPacker::addQuadSurface(
    const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3, const glm::vec3 &p4,
    uint32_t i1, uint32_t i2, uint32_t i3, uint32_t i4
)
{
    auto u = p2 - p1;
    auto v = p4 - p1;
    auto normal = glm::normalize(glm::cross(u, v));
    auto normalDirection = findBestMatchingDirection(normal);
    auto uDirection = Directions[normalDirection + 2];
    auto vDirection = Directions[normalDirection + 4];

    auto tc1 = projectToPlanes(p1, uDirection, vDirection);
    auto tc2 = projectToPlanes(p2, uDirection, vDirection);
    auto tc3 = projectToPlanes(p3, uDirection, vDirection);
    auto tc4 = projectToPlanes(p4, uDirection, vDirection);

    LightmapQuadSurface surface;
    surface.normal = normal;
    surface.positions[0] = p1; surface.positions[1] = p2; surface.positions[2] = p3; surface.positions[3] = p4;
    surface.texcoords[0] = tc1; surface.texcoords[1] = tc2; surface.texcoords[2] = tc3; surface.texcoords[3] = tc4;
    surface.indices[0] = i1; surface.indices[1] = i2; surface.indices[2] = i3; surface.indices[3] = i4;
    quadSurfaces.push_back(surface);
}

struct SurfaceWithAreaAndBox
{
    SurfaceWithAreaAndBox(float area = 0, Box2 box = Box2(), LightmapQuadSurface *surface = nullptr)
        : area(area), box(box), surface(surface)
    {
    }

    bool operator<(const SurfaceWithAreaAndBox &o) const
    {
        return area < o.area;
    }

    float area;
    Box2 box;
    LightmapQuadSurface *surface;
};

LightmapPtr LightmapPacker::buildLightMap()
{
    // Sort the surfaces according to their area.
    std::vector<SurfaceWithAreaAndBox> sortedSurfaces(quadSurfaces.size());
    for(size_t i = 0; i < quadSurfaces.size(); ++i)
    {
        auto &surface = quadSurfaces[i];
        Box2 box;
        for(int j = 0; j < 4; ++j)
            box.insertPoint(surface.texcoords[j]);
        sortedSurfaces[i] = SurfaceWithAreaAndBox(box.area(), box, &surface);
    }
    std::sort(sortedSurfaces.begin(), sortedSurfaces.end());

    // Position the surfaces.
    size_t column = 0;
    size_t row = 0;
    size_t currentHeight = 0;
    size_t maxWidth = 0;

    for(auto &surfaceWithAreaAndBox : sortedSurfaces)
    {
        auto &box = surfaceWithAreaAndBox.box;
        auto surface = surfaceWithAreaAndBox.surface;

        auto rawExtent = box.extent();
        size_t width = ceil(rawExtent.x / texelScale);
        size_t height = ceil(rawExtent.x / texelScale);
        //printf("Extent: %zu %zu\n", width, height);

        // Can we use this column?
        if(width + column + 1 > MaxTextureWidth)
        {
            maxWidth = std::max(maxWidth, column + 1);
            column = 0;
            row += currentHeight + 1;
            currentHeight = 0;
        }

        // Expand the column
        currentHeight = std::max(height, currentHeight);

        // Compute the integer texture coordinate.
        glm::vec2 texcoordOffset = glm::vec2(column, row);
        for(size_t i = 0; i < 4; ++i)
        {
            surface->texcoords[i] = (surface->texcoords[i] - box.min) / texelScale + texcoordOffset;
            //printf("tc: %f %f\n", surface->texcoords[i].x, surface->texcoords[i].y);
        }

        // Consume the cell
        column += width + 1;
    }

    maxWidth = std::max(maxWidth, column + 1);
    if(currentHeight > 0)
        row += currentHeight;

    // Compute the actual light map width and height.
    auto lightmapWidth = roundToNextPowerOfTwo(maxWidth);
    auto lightmapHeight = roundToNextPowerOfTwo(row);

    // Create the lightmap.
    auto lightmap = std::make_shared<Lightmap> ();
    lightmap->width = lightmapWidth;
    lightmap->height = lightmapHeight;

    buildLightMapPatchesFor(lightmap);
    printf("Patch count %zu\n", lightmap->patches.size());

    // Normalize the lightmap texture coordinates
    auto texcoordScale = glm::vec2(1.0f / lightmapWidth, 1.0f / lightmapHeight);
    //auto texcoordScale = glm::vec2(1.0f/maxWidth, 1.0f/column);
    for(auto &surface : quadSurfaces)
    {
        for(int i = 0; i < 4; ++i)
        {
            surface.texcoords[i] = (surface.texcoords[i] + 0.5f) * texcoordScale;
            //printf("tc: %f %f\n", surface.texcoords[i].x, surface.texcoords[i].y);
        }
    }

    // Create the lightmap buffers
    lightmap->createBuffers();

    return lightmap;
}

void LightmapPacker::buildLightMapPatchesFor(const LightmapPtr &lightmap)
{
    usedTexels.resize(lightmap->width*lightmap->height);

    // Create the lightmap patches.
    for(auto &surface : quadSurfaces)
    {
        buildLightMapPatchesForTriangle(lightmap, surface.index,
            surface.positions[0], surface.positions[1], surface.positions[2],
            surface.texcoords[0], surface.texcoords[1], surface.texcoords[2],
            surface.normal, surface.normal, surface.normal);
        buildLightMapPatchesForTriangle(lightmap, surface.index,
            surface.positions[2], surface.positions[3], surface.positions[0],
            surface.texcoords[2], surface.texcoords[3], surface.texcoords[0],
            surface.normal, surface.normal, surface.normal);
    }
}

void LightmapPacker::buildLightMapPatchesForTriangle(const LightmapPtr &lightmap,
    size_t surfaceIndex,
    const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3,
    const glm::vec2 &tc1, const glm::vec2 &tc2, const glm::vec2 &tc3,
    const glm::vec3 &n1, const glm::vec3 &n2, const glm::vec3 &n3
)
{
    // Rasterization algorithm taken from: https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/
    if(edgeOrientation(tc1, tc2, tc3) < 0)
    {
        return buildLightMapPatchesForTriangle(lightmap, surfaceIndex,
            p3, p2, p1,
            tc3, tc2, tc1,
            n3, n2, n1);
    }

    // Triangle rasterization.
    Box2 box;
    box.insertPoint(tc1);
    box.insertPoint(tc2);
    box.insertPoint(tc3);

    int minX = box.min.x;
    int minY = box.min.y;
    int maxX = ceil(box.max.x);
    int maxY = ceil(box.max.y);
    if(maxX >= intptr_t(lightmap->width))
        maxX = lightmap->width - 1;
    if(maxY >= intptr_t(lightmap->height))
        maxY = lightmap->height - 1;

    glm::vec2 f12(tc1.y - tc2.y, tc2.x - tc1.x);
    glm::vec2 f23(tc2.y - tc3.y, tc3.x - tc2.x);
    glm::vec2 f31(tc3.y - tc1.y, tc1.x - tc3.x);

    auto minP = glm::vec2(minX, minY);
    auto rowW1 = edgeOrientation(tc2, tc3, minP);
    auto rowW2 = edgeOrientation(tc3, tc1, minP);
    auto rowW3 = edgeOrientation(tc1, tc2, minP);

    for(int ty = minY; ty <= maxY; ++ty)
    {
        auto w1 = rowW1;
        auto w2 = rowW2;
        auto w3 = rowW3;

        for(int tx = minX; tx <= maxX; ++tx)
        {
            if(w1 >= 0 && w2 >= 0 && w3 >= 0)
            {
                size_t texelIndex = ty*lightmap->width + tx;
                if(!usedTexels[texelIndex])
                {
                    // Mark the texel as used.
                    usedTexels[texelIndex] = true;
                    auto normFactor = w1 + w2 + w3;
                    auto pw1 = w1 / normFactor;
                    auto pw2 = w2 / normFactor;
                    auto pw3 = w3 / normFactor;

                    LightmapPatch patch;
                    patch.texelIndex = texelIndex;
                    patch.normal = glm::normalize(n1*pw1 + n2*pw2 + n3*pw3);
                    patch.position = p1*pw1 + p2*pw2 + p3*pw3;
                    patch.surfaceIndex = surfaceIndex;
                    lightmap->patches.push_back(patch);
                }
            }

            w1 += f23.x;
            w2 += f31.x;
            w3 += f12.x;
        }

        rowW1 += f23.y;
        rowW2 += f31.y;
        rowW3 += f12.y;
    }
}

} // End of namespace RadiosityTest
