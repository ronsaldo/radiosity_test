#ifndef RADIOSITY_TEST_GPU_BUFFER_HPP
#define RADIOSITY_TEST_GPU_BUFFER_HPP

#include "Object.hpp"
#include "GLCommon.hpp"

namespace RadiosityTest
{
DECLARE_CLASS(GpuBuffer);

/**
 * The allowable mapping usages.
 */
enum class GpuBufferMapAccess
{
    ReadOnly = 0,
    WriteOnly,
    ReadWrite
};

/**
 * I represent a buffer that resides in GPU accessible memory.
 */
class GpuBuffer: public Object
{
public:
    GpuBuffer();
    ~GpuBuffer();

    GLenum getHandle() const
    {
        return handle;
    }

    void setPersistentStorage(size_t capacity, GpuBufferMapAccess newMappingAccess);
    void setUploadedStorage(size_t capacity);
    void setImmutableContent(size_t size, const void *content);

    size_t getCapacity() const
    {
        return capacity;
    }

    void *map();
    void unmap();

private:
    GLuint handle;
    size_t capacity;

    void *mappedPointer;
    size_t mapCount;
    GpuBufferMapAccess mappingAccess;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_GPU_BUFFER_HPP
