#include "SceneObject.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace RadiosityTest
{
SceneObject::SceneObject()
    : scene(nullptr)
{
}

SceneObject::~SceneObject()
{
}

void SceneObject::addedToScene(Scene *newScene)
{
    if(scene && scene != newScene)
        scene->removeObject(shared_from_this());

    scene = newScene;
}

void SceneObject::prepareForRendering()
{
}

void SceneObject::renderWith(Renderer *renderer)
{
}

glm::mat4 SceneObject::getCurrentTransform() const
{
    glm::mat4 matrix(orientation);
    matrix[3][0] = position.x;
    matrix[3][1] = position.y;
    matrix[3][2] = position.z;
    matrix[3][3] = 1.0f;
    return matrix;
}

SceneMeshObject::SceneMeshObject()
{
}

SceneMeshObject::~SceneMeshObject()
{
}

void SceneMeshObject::renderWith(Renderer *renderer)
{
    renderer->bindUniformBufferElement(0, objectState.current());
    if(mesh)
        renderer->drawMesh(mesh);
}

void SceneMeshObject::prepareForRendering()
{
    objectState->setAffineTransformation(getCurrentTransform());
}

} // End of namespace RadiosityTest
