#ifndef RADIOSITY_TEST_SCENE_HPP
#define RADIOSITY_TEST_SCENE_HPP

#include "Object.hpp"
#include <vector>

namespace RadiosityTest
{
DECLARE_CLASS(Scene);
DECLARE_CLASS(SceneObject);

class Scene: public Object
{
public:
    typedef std::vector<SceneObjectPtr> SceneObjects;

    Scene();
    ~Scene();

    void prepareForRendering();
    void addObject(const SceneObjectPtr &newObject);
    void removeObject(const SceneObjectPtr &object);

    const SceneObjects &getObjects() const
    {
        return objects;
    }
private:
    SceneObjects objects;
};

} // namespace RadiosityTest

#endif //RADIOSITY_TEST_SCENE_HPP
