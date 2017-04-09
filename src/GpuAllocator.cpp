#include "GpuAllocator.hpp"

namespace RadiosityTest
{

GpuAllocator::GpuAllocator(size_t elementSize, size_t capacity, GpuBufferMapAccess access)
    : elementSize(elementSize), capacity(capacity)
{
    usedElements.resize(capacity);
    buffer = std::make_shared<GpuBuffer> ();
    buffer->setPersistentStorage(capacity * elementSize, access);
    data = reinterpret_cast<uint8_t*> (buffer->map());
    usedElements.resize(capacity);
}

GpuAllocator::~GpuAllocator()
{
}

void *GpuAllocator::allocate()
{
    auto element = usedElements.findAnyFalse();
    if(element < 0)
        return nullptr;

    assert(!usedElements.get(element));
    usedElements.set(element, true);
    return data + (element*elementSize);
}

void GpuAllocator::free(void *element)
{
    if(!element)
        return;

    auto elementIndex = (reinterpret_cast<uint8_t*> (element) - data) / elementSize;
    assert(elementIndex < capacity);
    assert(usedElements.get(elementIndex));
    usedElements.set(elementIndex, false);
}

} // End of namespace RadiosityTest
