#ifndef RADIOSITY_TEST_LIGHT_HPP
#define RADIOSITY_TEST_LIGHT_HPP

#include "SceneObject.hpp"
#include "LightState.hpp"

namespace RadiosityTest
{
DECLARE_CLASS(Light);

/**
 * Light type.
 */
enum class LightType
{
    Directional = 0,
    Point,
    Spot
};

/**
 * A light.
 */
class Light : public SceneObject
{
public:
    Light();
    ~Light();

    virtual void accept(SceneVisitor *visitor) override;

    LightState currentState();

    LightType getType() const
    {
        return type;
    }

    void setType(LightType newType)
    {
        type = newType;
    }

    const glm::vec4 &getIntensity() const
    {
        return intensity;
    }

    void setIntensity(const glm::vec4 &newIntensity)
    {
        intensity = newIntensity;
    }

    const glm::vec3 &getAttenuationFactors() const
    {
        return attenuation;
    }

    void setAttenuationFactors(const glm::vec3 &newAttenuation)
    {
        attenuation = newAttenuation;
    }

    const glm::vec3 &getSpotDirection() const
    {
        return spotDirection;
    }

    void setSpotDirection(const glm::vec3 &newDirection)
    {
        spotDirection = newDirection;
    }

    void setSpotCutoff(const glm::vec2 &newCutoff)
    {
        spotCutoff = newCutoff;
    }

    float getSpotExponent() const
    {
        return spotExponent;
    }

    void setSpotExponent(float newSpotExponent)
    {
        spotExponent = newSpotExponent;
    }

private:
    LightType type;
    glm::vec4 intensity;
    glm::vec3 attenuation;
    glm::vec3 spotDirection;
    glm::vec2 spotCutoff;
    float spotExponent;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_LIGHT_HPP
