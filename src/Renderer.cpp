#include "Renderer.hpp"
#include "GpuProgram.hpp"
#include "GpuTexture.hpp"
#include "GenericMesh.hpp"
#include "VertexSpecification.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"
#include "Camera.hpp"
#include <string.h>

namespace RadiosityTest
{
static size_t frameBufferingIndex = 0;

size_t getCurrentFrameBufferingIndex()
{
    return frameBufferingIndex;
}

static const GLenum primitiveTypeMap[] = {
    GL_POINTS,
    GL_LINES,
    GL_TRIANGLES,
};

Renderer::Renderer()
{
    memset(fences, 0, sizeof(fences));
    lightMapFilter = LightMapFilter::Linear;
}

Renderer::~Renderer()
{
}

bool Renderer::initialize()
{
    if(!createPrograms())
        return false;
    return true;
}

bool Renderer::createPrograms()
{
    colorProgram = std::make_shared<GpuProgram> ();
    colorProgram->attachVertexFromFile("data/shaders/genericVertex.vert")
        .attachFragmentFromFile("data/shaders/color.frag")
        .link();

    normalProgram = std::make_shared<GpuProgram> ();
    normalProgram->attachVertexFromFile("data/shaders/genericVertex.vert")
        .attachFragmentFromFile("data/shaders/normal.frag")
        .link();

    lightmapProgram = std::make_shared<GpuProgram> ();
    lightmapProgram->attachVertexFromFile("data/shaders/genericVertex.vert")
        .attachFragmentFromFile("data/shaders/lightmap.frag")
        .link();

    currentProgram = lightmapProgram;
    return colorProgram->isValid();
}

void Renderer::useProgram(const GpuProgramPtr &program)
{
    program->activate();
}

void Renderer::renderScene(const CameraPtr &camera, const ScenePtr &scene)
{
    // Use sRGB
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    useProgram(currentProgram);
    camera->activateOn(this);
    {
        std::unique_lock<std::mutex> (scene->getMutex());
        for(auto &sceneObject : scene->getObjects())
            drawSceneObject(sceneObject);

    }
}

void Renderer::beginFrame()
{
    auto &fence = fences[frameBufferingIndex];
    if(fence)
    {
        GLenum waitResult = GL_UNSIGNALED;
        while(waitResult != GL_ALREADY_SIGNALED && waitResult != GL_CONDITION_SATISFIED)
        {
            waitResult = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
        }

        glDeleteSync(fence);
        fence = 0;
    }
}

void Renderer::endFrame()
{
    fences[frameBufferingIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    frameBufferingIndex = (frameBufferingIndex + 1) % 3;
}

void Renderer::drawSceneObject(const SceneObjectPtr &sceneObject)
{
    sceneObject->renderWith(this);
}

void Renderer::bindUniformBufferRange(const GpuBufferPtr &buffer, GLuint binding, size_t offset, size_t size)
{
    //printf("Bind uniform %p, %d %zu:%zu\n", buffer.get(), binding, offset, size);
    glBindBufferRange(GL_UNIFORM_BUFFER, binding, buffer->getHandle(), offset, size);
}

void Renderer::drawMesh(const MeshPtr &mesh)
{
    if(mesh->lightmap)
    {
        glActiveTexture(GL_TEXTURE1);
        auto lightmap = mesh->lightmap->getValidLightmapTexture();
        if(lightMapFilter == LightMapFilter::Linear)
            lightmap->setLinearFiltering();
        else
            lightmap->setNearestFiltering();

        glBindTexture(GL_TEXTURE_2D, lightmap->getHandle());
    }

    mesh->vertexSpecification->activate();
    for(auto &submesh : mesh->submeshes)
    {
        GLenum primitiveMode = primitiveTypeMap[(int)submesh.primitiveType];
        glDrawElements(primitiveMode, submesh.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*> (submesh.startIndex*4));
    }
}

} // End of namespace RadiosityTest
