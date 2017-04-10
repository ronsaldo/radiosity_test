#ifndef RADIOSITY_TEST_RAY_HPP
#define RADIOSITY_TEST_RAY_HPP

#include <glm/glm.hpp>
#include <math.h>

namespace RadiosityTest
{
/**
 * A ray
 */
struct Ray
{
    Ray() {}
    Ray(const glm::vec3 &position, const glm::vec3 &direction, float maxDistance = INFINITY)
        : position(position), direction(direction), maxDistance(maxDistance) {}

    static Ray fromEndPoints(const glm::vec3 &start, const glm::vec3 &end)
    {
        auto delta = end - start;
        auto length = glm::length(delta);
        return Ray(start, delta / length, length);
    }

    glm::vec3 position;
    glm::vec3 direction;
    float maxDistance;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_RAY_HPP
