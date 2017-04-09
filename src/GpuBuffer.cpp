#include "GpuBuffer.hpp"

namespace RadiosityTest
{
GpuBuffer::GpuBuffer()
    : capacity(0), mappedPointer(nullptr), mapCount(0)
{
    glCreateBuffers(1, &handle);
}

GpuBuffer::~GpuBuffer()
{
    if(mappedPointer)
    {
        glUnmapNamedBuffer(handle);
    }

    glDeleteBuffers(1, &handle);
}

void *GpuBuffer::map()
{
    ++mapCount;
    if(mapCount == 1)
    {
        GLenum mapMode;
        switch(mappingAccess)
        {
        default:
        case GpuBufferMapAccess::ReadOnly:
            mapMode = GL_READ_ONLY;
            break;
        case GpuBufferMapAccess::WriteOnly:
            mapMode = GL_WRITE_ONLY;
            break;
        case GpuBufferMapAccess::ReadWrite:
            mapMode = GL_READ_WRITE;
            break;
        }

        mappedPointer = glMapNamedBuffer(handle, mapMode);
    }
    return mappedPointer;
}

void GpuBuffer::unmap()
{
    assert(mapCount > 0);
    if(--mapCount == 0)
    {
        glUnmapNamedBuffer(handle);
        mappedPointer = nullptr;
    }
}

void GpuBuffer::setPersistentStorage(size_t newCapacity, GpuBufferMapAccess newMappingAccess)
{
    GLbitfield flags = GL_MAP_PERSISTENT_BIT |  GL_MAP_COHERENT_BIT;
    mappingAccess = newMappingAccess;
    capacity = newCapacity;

    switch(newMappingAccess)
    {
    default:
    case GpuBufferMapAccess::ReadOnly:
        flags |= GL_MAP_READ_BIT;
        break;
    case GpuBufferMapAccess::WriteOnly:
        flags |= GL_MAP_WRITE_BIT;
        break;
    case GpuBufferMapAccess::ReadWrite:
        flags |= GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
        break;
    }

    glNamedBufferStorage(handle, capacity, nullptr, flags);
}

void GpuBuffer::setUploadedStorage(size_t capacity)
{
    glNamedBufferStorage(handle, capacity, nullptr,  GL_DYNAMIC_STORAGE_BIT);
}

void GpuBuffer::setImmutableContent(size_t size, const void *content)
{
    capacity = size;
    glNamedBufferStorage(handle, size, content, 0);
}

} // End of namespace RadiosityTest
