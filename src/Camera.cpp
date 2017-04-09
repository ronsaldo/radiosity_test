#include "Camera.hpp"
#include "Renderer.hpp"

namespace RadiosityTest
{

Camera::Camera()
{
    projectionMatrix = glm::mat4();
}

Camera::~Camera()
{
}

void Camera::renderWith(Renderer *renderer)
{
}

void Camera::prepareForRendering()
{
    objectState->setOrthoTransformation(getCurrentTransform());
    objectState->projectionMatrix = projectionMatrix;
}

void Camera::activateOn(Renderer *renderer)
{
    renderer->bindUniformBufferElement(1, objectState.current());
}

} // End of namespace RadiosityTest
