#include "SceneObject.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"

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
    objectState->setOrthoTransformation(getCurrentTransform());
}

} // End of namespace RadiosityTest
