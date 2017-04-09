#ifndef RADIOSITY_TEST_OBJECT_STATE_HPP
#define RADIOSITY_TEST_OBJECT_STATE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "GpuAllocator.hpp"

namespace RadiosityTest
{

/**
 * Object state
 */
struct ObjectState
{
    glm::mat4 transformation;
    glm::mat4 inverseTransformation;

    glm::mat4 normalTransformation;
    glm::mat4 inverseNormalTransformation;

    void setTransformation(const glm::mat4 &newTransformation)
    {
        transformation = newTransformation;
        inverseTransformation = glm::inverse(transformation);
        buildNormalMatrices();
    }

    void setAffineTransformation(const glm::mat4 &newTransformation)
    {
        transformation = newTransformation;
        inverseTransformation = glm::affineInverse(transformation);
        buildNormalMatrices();
    }

    void buildNormalMatrices()
    {
        normalTransformation = glm::transpose(glm::mat3(inverseTransformation));
        inverseNormalTransformation = glm::transpose(glm::mat3(transformation));
    }
};

typedef GpuUniformObjectAllocator<ObjectState, 30000> ObjectStateAllocator;
typedef ObjectStateAllocator::BufferedObject ObjectStateBufferedObject;

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_OBJECT_STATE_HPP
