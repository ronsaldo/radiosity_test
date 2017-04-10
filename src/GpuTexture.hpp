#ifndef RADIOSITY_TEST_GPU_TEXTURE_HPP
#define RADIOSITY_TEST_GPU_TEXTURE_HPP

#include "Object.hpp"
#include "GLCommon.hpp"

namespace RadiosityTest
{
DECLARE_CLASS(GpuTexture);

class GpuTexture : public Object
{
public:
    GpuTexture(GLenum target);
    ~GpuTexture();

    void setStorage2D(size_t levels, size_t width, size_t height, GLenum internalFormat);
    void uploadLevel(size_t level, size_t x, size_t y, size_t width, size_t height, GLenum format, GLenum type, const void *data);
    void setNearestFiltering();
    void setLinearFiltering();
    void clampToEdge();

    GLenum getTarget() const
    {
        return target;
    }

    GLuint getHandle() const
    {
        return handle;
    }


private:
    GLenum target;
    GLuint handle;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_GPU_TEXTURE_HPP
