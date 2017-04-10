#include "LightmapBuildProcess.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"
#include "Mesh.hpp"
#include "Lightmap.hpp"
#include "Light.hpp"

namespace RadiosityTest
{
LightmapBuildProcess::LightmapBuildProcess()
    : running(false)
{
}

LightmapBuildProcess::~LightmapBuildProcess()
{
}

void LightmapBuildProcess::start()
{
    std::unique_lock<std::mutex> l(mutex);
    if(running)
        return;

    running = true;
    std::thread t([=]{
        threadProcess();
    });
    thread.swap(t);
}

void LightmapBuildProcess::shutdown()
{
    {
        std::unique_lock<std::mutex> l(mutex);
        running = false;
    }
    thread.join();
}

void LightmapBuildProcess::threadProcess()
{
    for(;;)
    {
        ScenePtr currentScene;
        {
            std::unique_lock<std::mutex> l(mutex);
            if(!running)
                break;

            currentScene = theScene;
        }

        processScene(currentScene);
        pendingLightmaps.clear();
        currentLights.clear();
    }
}

void LightmapBuildProcess::processScene(const ScenePtr &scene)
{
    class Visitor : public SceneVisitor
    {
    public:
        Visitor(LightmapBuildProcess *self)
            : self(self) {}

        void visitSceneMeshObject(SceneMeshObject *object)
        {
            auto &mesh = object->getMesh();
            if(mesh && mesh->lightmap)
                self->pendingLightmaps.push_back(mesh->lightmap);
        }

        void visitLight(Light *light)
        {
            self->currentLights.push_back(light->currentState());
        }

        LightmapBuildProcess *self;
    };

    if(!scene)
        return;

    {
        std::unique_lock<std::mutex> (scene->getMutex());
        Visitor visitor(this);
        for(auto &object : scene->getObjects())
        {
            object->accept(&visitor);
        }
    }

    for(auto &lightmap : pendingLightmaps)
        lightmap->process(currentLights);
}

} // End of namespace RadiosityTest
