#include "Camera.hpp"
#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>

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
    objectState->setAffineTransformation(getCurrentTransform());
    objectState->projectionMatrix = projectionMatrix;
}

void Camera::activateOn(Renderer *renderer)
{
    renderer->bindUniformBufferElement(1, objectState.current());
}

void Camera::perspective(float fovy, float aspectRatio, float near, float far)
{
    projectionMatrix = glm::perspective(float(fovy*M_PI/180.0), aspectRatio, near, far);
}

} // End of namespace RadiosityTest
