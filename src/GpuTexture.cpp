#include "GpuTexture.hpp"

namespace RadiosityTest
{

GpuTexture::GpuTexture(GLenum target)
    : target(target)
{
    glCreateTextures(target, 1, &handle);
}

GpuTexture::~GpuTexture()
{
    glDeleteTextures(1, &handle);
}

void GpuTexture::setStorage2D(size_t levels, size_t width, size_t height, GLenum internalFormat)
{
    glTextureStorage2D(handle, levels, internalFormat, width, height);
    glTextureParameteri(handle, GL_TEXTURE_MAX_LEVEL, levels - 1);
}

void GpuTexture::setNearestFiltering()
{
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void GpuTexture::setLinearFiltering()
{
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void GpuTexture::clampToEdge()
{
    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GpuTexture::uploadLevel(size_t level, size_t x, size_t y, size_t width, size_t height, GLenum format, GLenum type, const void *data)
{
    glTextureSubImage2D(handle, level, x, y, width, height, format, type, data);
}

} // End of namespace RadiosityTest
