#ifndef RADIOSITY_TEST_GPU_ALLOCATOR_HPP
#define RADIOSITY_TEST_GPU_ALLOCATOR_HPP

#include "GpuBuffer.hpp"
#include "BitSet.hpp"
#include <stdint.h>
#include <algorithm>
#include <vector>

namespace RadiosityTest
{
size_t getCurrentFrameBufferingIndex();

/**
 * Gpu memory mapped heap
 */
class GpuAllocator
{
public:
    GpuAllocator(size_t elementSize, size_t capacity, GpuBufferMapAccess access);
    ~GpuAllocator();

    void *allocate();
    void free(void *element);
    size_t getCapacity() const
    {
        return capacity;
    }

    size_t pointerToBufferOffset(const void *pointer) const
    {
        return reinterpret_cast<const uint8_t*> (pointer) - data;
    }

    const GpuBufferPtr &getBufferObject() const
    {
        return buffer;
    }

private:
    GpuBufferPtr buffer;
    size_t elementSize;
    size_t capacity;
    uint8_t *data;
    BitSet usedElements;
};

template<typename T, typename A>
class GpuUniquePtr
{
public:
    typedef A Allocator;
    typedef GpuUniquePtr<T, A> SelfType;

    GpuUniquePtr(T *pointer = nullptr)
        : pointer(pointer) {}
    GpuUniquePtr(SelfType &&o)
    {
        pointer = o.pointer;
        o.pointer = nullptr;
    }

    ~GpuUniquePtr()
    {
        free();
    }

    T *get() const
    {
        return pointer;
    }

    void reset(T *newPointer)
    {
        free();
        pointer = newPointer;
    }

    T *release()
    {
        auto result = pointer;
        pointer = nullptr;
        return result;
    }

    void free()
    {
        if(pointer)
            Allocator::get().free(pointer);
        pointer = nullptr;
    }

    void swap(SelfType &o)
    {
        std::swap(pointer, o.pointer);
    }

    const GpuBufferPtr &getBufferObject() const
    {
        return Allocator::get().getBufferObject();
    }

    size_t getBufferOffset() const
    {
        return Allocator::get().pointerToBufferOffset(pointer);
    }

    size_t getElementSize() const
    {
        return Allocator::ElementSize;
    }

private:
    GpuUniquePtr(const SelfType &o)
    {}

    SelfType &operator=(const SelfType &o)
    {
        return *this;
    }

    T *pointer;
};

template<typename ET, typename A>
class GpuBufferedObject
{
public:
    typedef ET ElementType;
    typedef A Allocator;
    typedef GpuBufferedObject<ET, A> SelfType;
    typedef GpuUniquePtr<ET, A> UniquePointer;

    GpuBufferedObject()
    {
        pointers[0].reset(Allocator::get().allocate());
        pointers[1].reset(Allocator::get().allocate());
        pointers[2].reset(Allocator::get().allocate());
    }

    ~GpuBufferedObject()
    {
    }

    const UniquePointer &current() const
    {
        return pointers[getCurrentFrameBufferingIndex()];
    }

    ET *get() const
    {
        return current().get();
    }

    ET *operator->() const
    {
        return current().get();
    }

    UniquePointer pointers[3];
};

template<typename ET, size_t C, GpuBufferMapAccess MA, size_t A>
class GpuObjectAllocator
{
public:
    typedef ET ElementType;
    typedef GpuObjectAllocator<ET, C, MA, A> SelfType;
    typedef GpuUniquePtr<ET, SelfType> UniquePtr;
    typedef GpuBufferedObject<ET, SelfType> BufferedObject;

    static constexpr size_t Capacity = C;
    static constexpr size_t Alignment = A;
    static constexpr GpuBufferMapAccess MemoryAccess = MA;
    static constexpr size_t ElementSize = (sizeof(ElementType) + Alignment - 1) & (-Alignment);

    GpuObjectAllocator()
        : allocator(ElementSize, Capacity, MemoryAccess)
    {
    }

    ElementType *allocate()
    {
        return reinterpret_cast<ElementType*> (allocator.allocate());
    }

    void free(ElementType *element)
    {
        allocator.free(element);
    }

    UniquePtr &&makeUnique()
    {
        return UniquePtr(allocate());
    }

    size_t pointerToBufferOffset(const ElementType *pointer)
    {
        return allocator.pointerToBufferOffset(pointer);
    }

    const GpuBufferPtr &getBufferObject() const
    {
        return allocator.getBufferObject();
    }

    static GpuObjectAllocator &get()
    {
        if(!singleton)
            singleton = new SelfType();
        return *singleton;
    }

private:
    static SelfType *singleton;
    GpuAllocator allocator;
};

template<typename ET, size_t C, GpuBufferMapAccess MA, size_t A>
GpuObjectAllocator<ET, C, MA, A> *GpuObjectAllocator<ET, C, MA, A>::singleton = nullptr;

template<typename ET, size_t C>
using GpuUniformObjectAllocator = GpuObjectAllocator<ET, C, GpuBufferMapAccess::WriteOnly, 256>;

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_GPU_ALLOCATOR_HPP
