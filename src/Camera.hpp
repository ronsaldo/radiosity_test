#ifndef RADIOSITY_TEST_CAMERA_HPP
#define RADIOSITY_TEST_CAMERA_HPP

#include "SceneObject.hpp"
#include "CameraState.hpp"

namespace RadiosityTest
{
DECLARE_CLASS(Camera);

class Camera : public SceneObject
{
public:
    Camera();
    ~Camera();

    virtual void renderWith(Renderer *renderer) override;
    virtual void prepareForRendering() override;

    virtual void accept(SceneVisitor *visitor) override;

    void activateOn(Renderer *renderer);

    void perspective(float fovy, float aspectRatio, float near, float far);

    const glm::mat4 &getProjectionMatrix() const
    {
        return projectionMatrix;
    }

    void setProjectionMatrix(const glm::mat4 &newMatrix)
    {
        projectionMatrix = newMatrix;
    }

private:
    glm::mat4 projectionMatrix;
    CameraStateBufferedObject objectState;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_CAMERA_HPP
