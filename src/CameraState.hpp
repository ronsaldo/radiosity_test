#ifndef RADIOSITY_TEST_CAMERA_STATE_HPP
#define RADIOSITY_TEST_CAMERA_STATE_HPP

#include "ObjectState.hpp"

namespace RadiosityTest
{

/**
 * State of a camera.
 */
struct CameraState : ObjectState
{
    glm::mat4 projectionMatrix;
};

typedef GpuUniformObjectAllocator<CameraState, 1000> CameraStateAllocator;
typedef CameraStateAllocator::BufferedObject CameraStateBufferedObject;

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_CAMERA_STATE_HPP
