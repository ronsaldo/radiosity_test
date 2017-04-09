#ifndef RADIOSITY_TEST_SCENE_OBJECT_HPP
#define RADIOSITY_TEST_SCENE_OBJECT_HPP

#include "Object.hpp"
#include "GpuAllocator.hpp"
#include "ObjectState.hpp"
#include "CameraState.hpp"
#include <glm/glm.hpp>

namespace RadiosityTest
{
DECLARE_CLASS(Scene)
DECLARE_CLASS(SceneObject);
DECLARE_CLASS(Mesh);
DECLARE_CLASS(Renderer);

/**
 * Scene object
 */
class SceneObject : public Object,
    public std::enable_shared_from_this<SceneObject>
{
public:
    SceneObject();
    ~SceneObject();

    virtual void addedToScene(Scene *newScene);
    virtual void prepareForRendering();
    virtual void renderWith(Renderer *renderer);

    Scene *getScene() const
    {
        return scene;
    }

    const glm::vec3 &getPosition() const
    {
        return position;
    }

    const glm::mat3 &getOrientation() const
    {
        return orientation;
    }

    const MeshPtr &getRenderable() const
    {
        return renderable;
    }

    void setRenderable(const MeshPtr &newRenderable)
    {
        renderable = newRenderable;
    }

    glm::mat4 getCurrentTransform()
    {
        return glm::mat4();
    }

private:
    glm::vec3 position;
    glm::mat3 orientation;
    MeshPtr renderable;
    Scene *scene;
};

/**
 * Scene mesh object
 */
class SceneMeshObject : public SceneObject
{
public:
    SceneMeshObject();
    ~SceneMeshObject();

    virtual void renderWith(Renderer *renderer) override;
    virtual void prepareForRendering() override;

    const MeshPtr &getMesh() const
    {
        return mesh;
    }

    void setMesh(const MeshPtr &newMesh)
    {
        mesh = newMesh;
    }

private:
    MeshPtr mesh;
    ObjectStateBufferedObject objectState;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_SCENE_OBJECT_HPP
