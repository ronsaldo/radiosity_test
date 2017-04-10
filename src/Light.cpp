#include "Light.hpp"

namespace RadiosityTest
{

Light::Light()
{
    type = LightType::Point;
    attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
    intensity = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    spotCutoff = glm::vec2(90.0, 90.0); // 90 degrees
    spotExponent = 1.0f;
}

Light::~Light()
{
}

void Light::accept(SceneVisitor *visitor)
{
    return visitor->visitLight(this);
}

LightState Light::currentState()
{
    LightState state;
    state.intensity = intensity;
    state.attenuation = attenuation;
    if(type == LightType::Directional)
        state.position = glm::vec4(getOrientation()*glm::vec3(0, 0, 1), 0.0);
    else
        state.position = glm::vec4(getPosition(), 1.0);

    if(type == LightType::Spot)
    {
        state.spotDirection = getOrientation()*glm::vec3(0, 0, 1);
        state.spotCutoff = glm::cos(spotCutoff*float(M_PI/180.0));
        state.spotExponent = spotExponent;
    }
    else
    {
        state.spotCutoff = glm::vec2(-1.0, -1.0);
        state.spotExponent = 0.0f;
    }
    return state;
}

} // End of namespace RadiosityTest
