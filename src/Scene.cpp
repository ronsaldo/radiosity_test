#include "Scene.hpp"
#include "SceneObject.hpp"

namespace RadiosityTest
{

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::prepareForRendering()
{
    for(auto &object : objects)
        object->prepareForRendering();
}

void Scene::addObject(const SceneObjectPtr &newObject)
{
    objects.push_back(newObject);
    newObject->addedToScene(this);
}

void Scene::removeObject(const SceneObjectPtr &object)
{
    for(size_t i = 0; i < objects.size(); ++i)
    {
        if(objects[i] == object)
        {
            objects.erase(objects.begin() + i);
            return;
        }
    }
}

} // End of namespace RadiosityTest
