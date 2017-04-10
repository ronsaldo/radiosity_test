#ifndef RADIOSITY_TEST_BOX2_HPP
#define RADIOSITY_TEST_BOX2_HPP

#include <glm/vec2.hpp>
#include <algorithm>
#include <math.h>

namespace RadiosityTest
{

class Box2
{
public:
    Box2()
        : min(FP_INFINITE, FP_INFINITE), max(-FP_INFINITE, -FP_INFINITE) {}
    Box2(float minX, float minY, float maxX, float maxY)
        : min(minX, minY), max(maxX, maxY) {}

    void insertPoint(const glm::vec2 &point)
    {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
    }

    void insertBox(const Box2 &box)
    {
        min.x = std::min(min.x, box.min.x);
        min.y = std::min(min.y, box.min.y);
        max.x = std::max(max.x, box.max.x);
        max.y = std::max(max.y, box.max.y);
    }

    glm::vec2 extent() const
    {
        return max - min;
    }

    float area() const
    {
        auto ex = extent();
        return ex.x * ex.y;
    }

    glm::vec2 min, max;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_BOX2_HPP
