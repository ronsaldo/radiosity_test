#ifndef RADIOSITY_LIGHT_STATE_HPP
#define RADIOSITY_LIGHT_STATE_HPP

#include <glm/glm.hpp>

namespace RadiosityTest
{

/**
 * Light source state.
 */
struct LightState
{
    glm::vec4 position;
    glm::vec4 intensity;
    glm::vec3 attenuation;
    glm::vec3 spotDirection;
    glm::vec2 spotCutoff;
    float spotExponent;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_LIGHT_STATE_HPP
