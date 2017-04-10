#ifndef RADIOSITY_TEST_RENDERER_HPP
#define RADIOSITY_TEST_RENDERER_HPP

#include "Object.hpp"
#include "GLCommon.hpp"

namespace RadiosityTest
{
DECLARE_CLASS(Renderer);
DECLARE_CLASS(Scene);
DECLARE_CLASS(SceneObject);
DECLARE_CLASS(GpuProgram);
DECLARE_CLASS(GpuBuffer);
DECLARE_CLASS(Mesh);
DECLARE_CLASS(Camera);

class Renderer: public Object
{
public:
    Renderer();
    ~Renderer();

    bool initialize();
    void beginFrame();
    void renderScene(const CameraPtr &camera, const ScenePtr &scene);
    void endFrame();

    void drawSceneObject(const SceneObjectPtr &sceneObject);
    void drawMesh(const MeshPtr &mesh);

    void bindUniformBufferRange(const GpuBufferPtr &buffer, GLuint binding, size_t offset, size_t size);

    template<typename T>
    void bindUniformBufferElement(GLuint binding, const T &pointer)
    {
        bindUniformBufferRange(pointer.getBufferObject(), binding, pointer.getBufferOffset(), pointer.getElementSize());
    }

    void useProgram(const GpuProgramPtr &program);

private:
    bool createPrograms();

    GpuProgramPtr colorProgram;
    GpuProgramPtr lightmapProgram;
    GpuProgramPtr currentProgram;
    GLsync fences[3];

};

size_t getCurrentFrameBufferingIndex();

} // End of namespace MonsterPhysics

#endif //RADIOSITY_TEST_RENDERER_HPP
