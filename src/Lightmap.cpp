#include "Lightmap.hpp"
#include "GpuTexture.hpp"
#include "Ray.hpp"
#include <string.h>
#include "Float.hpp"

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

inline bool isRightOf(const glm::vec2 &e1, const glm::vec2 &e2, const glm::vec2 &p)
{
    return edgeOrientation(e1, e2, p) < -FloatEpsilon;
}

inline uint8_t encodeColorChannel(float f)
{
    return glm::clamp(int(f*255), 0, 255);
}

inline uint32_t encodeColor(const glm::vec4 &color)
{
    auto r = encodeColorChannel(color.r);
    auto g = encodeColorChannel(color.g);
    auto b = encodeColorChannel(color.b);
    auto a = encodeColorChannel(color.a);

    return r | (g << 8) | (b << 16) | (a << 24);
}

inline bool closeTo(const glm::vec3 &a, const glm::vec3 &b)
{
    return closeTo(a.x, b.x) && closeTo(a.y, b.y) && closeTo(a.z, b.z);
}
static float rayPlaneIntersection(const Ray &ray, const glm::vec3 &normal, float distance)
{
    auto den = glm::dot(ray.direction, normal);
    if(closeTo(den, 0))
        return -1.0f;

    return (distance - glm::dot(ray.position, normal)) / den;
}

static float rayQuadIntersection(const Ray &ray, const LightmapCompactQuadSurface &quad)
{
    auto intersection = rayPlaneIntersection(ray, quad.normal, quad.distance);
    if(intersection < 0)
        return intersection;

    auto intersectionPoint = ray.position + ray.direction*intersection;
    auto projectedPoint = glm::vec2(glm::dot(quad.tangent, intersectionPoint), glm::dot(quad.bitangent, intersectionPoint));
    if(isRightOf(quad.vertices[0], quad.vertices[1], projectedPoint) ||
        isRightOf(quad.vertices[1], quad.vertices[2], projectedPoint) ||
        isRightOf(quad.vertices[2], quad.vertices[3], projectedPoint) ||
        isRightOf(quad.vertices[3], quad.vertices[0], projectedPoint))
        return -1.0f;

    return intersection;
}

Lightmap::Lightmap()
    : frontBuffer(nullptr), backBuffer(nullptr), directLightBuffer(nullptr), indirectLightBuffer(nullptr)

{
}

Lightmap::~Lightmap()
{
    delete [] frontBuffer;
    delete [] backBuffer;
    delete [] directLightBuffer;
    delete [] indirectLightBuffer;
}

void Lightmap::createBuffers()
{
    frontBuffer = new uint32_t[width*height];
    backBuffer = new uint32_t[width*height];
    directLightBuffer = new glm::vec4[width*height];
    indirectLightBuffer = new glm::vec4[width*height];
    oldIndirectLightBuffer = new glm::vec4[width*height];

    memset(frontBuffer, 0, width*height*4);
    memset(backBuffer, 0, width*height*4);

    lightmapTexture = std::make_shared<GpuTexture> (GL_TEXTURE_2D);
    lightmapTexture->setStorage2D(1, width, height, GL_RGBA8);
    //lightmapTexture->setNearestFiltering();
    lightmapTexture->setLinearFiltering();
    lightmapTexture->clampToEdge();
    lightmapTexture->uploadLevel(0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frontBuffer);
    uploadedCount = 0;
    computedCount = 0;
}

GpuTexturePtr Lightmap::getValidLightmapTexture()
{
    std::unique_lock<std::mutex> l(mutex);
    if(uploadedCount != computedCount)
    {
        lightmapTexture->uploadLevel(0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frontBuffer);
        uploadedCount = computedCount;
    }

    return lightmapTexture;
}

void Lightmap::swapBuffers()
{
    std::unique_lock<std::mutex> l(mutex);
    ++computedCount;
    std::swap(frontBuffer, backBuffer);
}

void Lightmap::process(const std::vector<LightState> &lights)
{
    computeDirectLights(lights);
    computeIndirectLightBounce();

    for(size_t i = 0; i < width*height; ++i)
    {
        backBuffer[i] = encodeColor(directLightBuffer[i] + indirectLightBuffer[i]);
    }

    swapBuffers();
}

void Lightmap::computeDirectLights(const std::vector<LightState> &lights)
{
    // Direct lights
    for(auto &patch : patches)
    {
        glm::vec4 lightColor;
        for(auto &light : lights)
        {
            if(light.position.w == 1.0)
            {
                // Are we in shadow?
                if(isRayOccluded(patch.position, patch.surfaceIndex, light.position))
                    continue;
            }

            auto lightDir = glm::vec3(light.position) - patch.position*light.position.w;
            auto lightDistance = glm::length(lightDir);
            auto L = lightDir / lightDistance;
            auto NdotL = std::max(glm::dot(L, patch.normal), 0.0f);
            if(NdotL > 0.0f)
            {
                auto spotFactor = 1.0f;
                if(light.spotCutoff.x > -0.5)
                {
                    auto NdotS = glm::dot(light.spotDirection, L);
                    spotFactor = glm::pow(glm::smoothstep(light.spotCutoff.x, light.spotCutoff.y, NdotS), light.spotExponent);
                }

                auto lightAttenuation = spotFactor / (light.attenuation.x + light.attenuation.y*lightDistance + light.attenuation.z*(lightDistance*lightDistance));
                auto lightAmount = NdotL*lightAttenuation;
                lightColor += lightAmount*light.intensity;
            }
        }

        directLightBuffer[patch.texelIndex] = lightColor;
    }
}

void Lightmap::computeIndirectLightBounce()
{
    auto columns = patches.size();
    auto row = 0;
    std::swap(oldIndirectLightBuffer, indirectLightBuffer);
    //memset(oldIndirectLightBuffer, 0, sizeof(oldIndirectLightBuffer[0])*width*height);
    memset(indirectLightBuffer, 0, sizeof(indirectLightBuffer[0])*width*height);
    for(size_t i = 0; i < patches.size(); ++i, row += columns)
    {
        auto &sourcePatch = patches[i];
        auto sourceNormalizationFactor = viewFactorsDen[i];

        for(size_t j = i + 1; j < patches.size(); ++j)
        {
            auto &destPatch = patches[j];
            auto destNormalizationFactor = viewFactorsDen[i];

            size_t index = row + j;
            auto factor = viewFactors[index];

            auto sourceFactor = factor*sourceNormalizationFactor;
            auto destFactor = factor*destNormalizationFactor;

            indirectLightBuffer[sourcePatch.texelIndex] += (oldIndirectLightBuffer[destPatch.texelIndex] + directLightBuffer[destPatch.texelIndex])*sourceFactor;
            indirectLightBuffer[destPatch.texelIndex] += (oldIndirectLightBuffer[sourcePatch.texelIndex] + directLightBuffer[sourcePatch.texelIndex])*destFactor;
        }
    }
}

void Lightmap::computeRadiosityFactors()
{
    // TODO: Use sparse matrices.
    viewFactors.resize(patches.size()*patches.size());
    auto columns = patches.size();
    auto occlusionCount = 0;
    auto visibleCount = 0;

    for(size_t i = 0; i < patches.size(); ++i)
    {
        auto &sourcePatch = patches[i];
        for(size_t j = i + 1; j < patches.size(); ++j)
        {
            auto &destPatch = patches[j];

            // Compute the visibility factor
            if(closeTo(destPatch.position, sourcePatch.position))
                continue;

            auto patchDirection = glm::normalize(destPatch.position - sourcePatch.position);
            auto destPatchVisibilityFactor = glm::dot(-patchDirection, destPatch.normal);
            if(destPatchVisibilityFactor < 0)
                continue;

            auto sourcePatchVisibilityFactor = glm::dot(patchDirection, sourcePatch.normal);
            if(sourcePatchVisibilityFactor < 0)
                continue;

            auto visibilityFactor = destPatchVisibilityFactor*sourcePatchVisibilityFactor;

            // Discard the patches that are occluded
            if(isRayOccluded(sourcePatch.position, sourcePatch.surfaceIndex, destPatch.position, destPatch.surfaceIndex))
            {
                ++occlusionCount;
                continue;
            }

            ++visibleCount;
            viewFactors[i*columns + j] = viewFactors[j*columns + i] = visibilityFactor;
        }
    }

    // Compute the view factor normalization constants
    size_t rowStart = 0;
    viewFactorsDen.resize(patches.size());
    for(size_t i = 0; i < patches.size(); ++i, rowStart += columns)
    {
        float viewFactorSum = 0;
        for(size_t j = 0; j < columns; ++j)
            viewFactorSum += viewFactors[rowStart + j];
        //printf("vf %f\n", viewFactorSum);
        if(!closeTo(viewFactorSum, 0.0f))
            viewFactorsDen[i] = Reflectivity / viewFactorSum;
    }

    printf("Visible: %d Occluded patches: %d\n", visibleCount, occlusionCount);
    auto f = fopen("radFactors.bin", "w");
    fwrite(&viewFactors[0], 4*viewFactors.size(), 1, f);
    fclose(f);
}

bool Lightmap::isRayOccluded(glm::vec3 startPoint, size_t startSurfaceIndex,
    glm::vec3 endPoint, size_t endSurfaceIndex)
{
    auto ray = Ray::fromEndPoints(startPoint, endPoint);
    for(size_t i = 0; i < quadSurfaces.size(); ++i)
    {
        if(i == startSurfaceIndex || i == endSurfaceIndex)
            continue;

        auto &surface = quadSurfaces[i];
        auto result = rayQuadIntersection(ray, surface);
        if(result > 0.0 && result < ray.maxDistance)
            return true;
    }

    return false;
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
    surface.index = quadSurfaces.size();
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

    // Pass the surfaces.
    lightmap->quadSurfaces.resize(quadSurfaces.size());
    for(size_t i = 0; i < quadSurfaces.size(); ++i)
    {
        auto &source = quadSurfaces[i];
        auto &dest = lightmap->quadSurfaces[i];

        // TBN matrix basis
        dest.normal = source.normal;
        dest.distance = 0.0f;
        for(int i = 0; i < 4; ++i)
            dest.distance += glm::dot(dest.normal, source.positions[i])*0.25f;

        auto normalDirection = findBestMatchingDirection(dest.normal);
        dest.tangent = Directions[normalDirection + 2];
        dest.bitangent = Directions[normalDirection + 4];

        // Gram-Schmidth orthogonalization
        dest.tangent = glm::normalize(dest.tangent - glm::dot(dest.tangent, dest.normal)*dest.normal);
        dest.bitangent = glm::normalize(dest.bitangent - glm::dot(dest.bitangent, dest.normal)*dest.normal - glm::dot(dest.bitangent, dest.tangent)*dest.tangent);

        for(int i = 0; i < 4; ++i)
        {
            auto &p = source.positions[i];
            dest.vertices[i] = glm::vec2(glm::dot(dest.tangent, p), glm::dot(dest.bitangent, p));
        }

        //printf("Surface plane: %f %f %f , %f\n", dest.normal.x, dest.normal.y, dest.normal.z, dest.distance);
        //printf("Surface tangent: %f %f %f\n", dest.tangent.x, dest.tangent.y, dest.tangent.z);
        //printf("Surface bitangent: %f %f %f\n", dest.bitangent.x, dest.bitangent.y, dest.bitangent.z);
    }

    // Compute the radiosity factors.
    lightmap->computeRadiosityFactors();

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
