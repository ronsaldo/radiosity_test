#ifndef RADIOSITY_TEST_GENERIC_VERTEX_HPP
#define RADIOSITY_TEST_GENERIC_VERTEX_HPP

#include <glm/glm.hpp>

namespace RadiosityTest
{
/**
 * Generic vertex
 */
struct GenericVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 texcoord;
};

} // End of namespace namespace RadiosityTest

#endif //RADIOSITY_TEST_GENERIC_VERTEX_HPP
