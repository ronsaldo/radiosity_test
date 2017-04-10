#ifndef RADIOSITY_TEST_LIGHTMAP_BUILD_PROCESS_HPP
#define RADIOSITY_TEST_LIGHTMAP_BUILD_PROCESS_HPP

#include "Object.hpp"
#include "LightState.hpp"
#include <thread>
#include <mutex>
#include <vector>

namespace RadiosityTest
{
DECLARE_CLASS(LightmapBuildProcess);
DECLARE_CLASS(Scene);
DECLARE_CLASS(Lightmap);

/**
 * A process that takes care of building lightmaps.
 */
class LightmapBuildProcess : public Object
{
public:
    LightmapBuildProcess();
    ~LightmapBuildProcess();

    void start();
    void shutdown();

    void setScene(const ScenePtr &newScene)
    {
        theScene = newScene;
    }

    const ScenePtr &getScene() const
    {
        return theScene;
    }

private:
    void threadProcess();
    void processScene(const ScenePtr &scene);

    std::thread thread;
    std::mutex mutex;
    ScenePtr theScene;
    std::vector<LightState> currentLights;
    std::vector<LightmapPtr> pendingLightmaps;
    bool running;
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_LIGHTMAP_BUILD_PROCESS_HPP
