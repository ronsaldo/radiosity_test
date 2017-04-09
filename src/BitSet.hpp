#ifndef RADIOSITY_TEST_BITSET_HPP
#define RADIOSITY_TEST_BITSET_HPP

#include <vector>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <stdint.h>

namespace RadiosityTest
{
/**
 * A bit set
 */
class BitSet
{
public:

    BitSet()
        : size(0)
    {
    }

    BitSet(size_t size, bool value = false)
        : size(0)
    {
        resize(size, value);
    }

    intptr_t findAnyTrue()
    {
        return findAny(0, true);
    }

    intptr_t findAnyFalse()
    {
        return findAny(uint32_t(-1), false);
    }

    intptr_t findAny(uint32_t blockCompare, bool elementValue)
    {
        auto capacity = data.size();
        for(size_t i = 0; i < capacity; ++i)
        {
            if(data[i] != blockCompare)
            {
                auto startIndex = i*32;
                auto endIndex = std::min(startIndex + 32, size);
                for(size_t j = startIndex; j != endIndex; ++j)
                {
                    if(get(j) == elementValue)
                        return j;
                }

                return -1;
            }
        }

        return -1;
    }

    void resize(size_t newSize, bool value = false)
    {
        size = newSize;
        data.resize((size + 31) / 32, value ? -1 : 0);
    }

    bool get(size_t index)
    {
        assert(index < size);

        size_t elementIndex = index >> 5;
        size_t bitIndex = index & 31;
        return ((data[elementIndex] >> bitIndex) & 1) != 0;
    }

    void set(size_t index, bool value)
    {
        assert(index < size);

        size_t elementIndex = index >> 5;
        size_t bitIndex = index & 31;
        uint32_t bit = 1u << bitIndex;
        if(value)
            data[elementIndex] |= bit;
        else
            data[elementIndex] &= ~bit;
    }

private:
    std::vector<uint32_t> data;
    size_t size;
};
} // End of namespace namespace RadiosityTest

#endif // RADIOSITY_TEST_BITSET_HPP
