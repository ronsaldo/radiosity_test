#include "SDL.h"
#include "SDL_main.h"
#include "GLCommon.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"
#include "Camera.hpp"
#include "GenericMesh.hpp"
#include "Light.hpp"
#include "LightmapBuildProcess.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace RadiosityTest;

static SDL_Window *window;
static SDL_GLContext glContext;
static bool quitting = false;
static int screenWidth = 640;
static int screenHeight = 480;

static void processEvents();
static void render();

static RendererPtr renderer;
static ScenePtr scene;
static CameraPtr camera;
static LightmapBuildProcessPtr lightmapProcess;
static LightPtr spotLight;

static glm::vec3 cameraVelocity;
static glm::vec2 cameraAngularVelocity;
static glm::vec2 cameraAngle;

static constexpr float CameraSlowSpeed = 1.0f;
static constexpr float CameraFastSpeed = 3.0f;
static float cameraSpeed = CameraSlowSpeed;

static void onKeyDown(const SDL_KeyboardEvent &event)
{
    switch(event.keysym.sym)
    {
    case SDLK_ESCAPE:
        quitting = true;
        break;
    case SDLK_a:
        cameraVelocity.x = -1.0f;
        break;
    case SDLK_d:
        cameraVelocity.x = 1.0f;
        break;
    case SDLK_w:
        cameraVelocity.z = -1.0f;
        break;
    case SDLK_s:
        cameraVelocity.z = 1.0f;
        break;
    case SDLK_SPACE:
        cameraVelocity.y = 1.0f;
        break;
    case SDLK_LCTRL:
        cameraVelocity.y = -1.0f;
        break;
    case SDLK_LEFT:
        cameraAngularVelocity.y = 1.0f;
        break;
    case SDLK_RIGHT:
        cameraAngularVelocity.y = -1.0f;
        break;
    case SDLK_UP:
        cameraAngularVelocity.x = 1.0f;
        break;
    case SDLK_DOWN:
        cameraAngularVelocity.x = -1.0f;
        break;
    case SDLK_LSHIFT:
        cameraSpeed = CameraFastSpeed;
        break;
    case SDLK_1:
        renderer->useColorProgram();
        break;
    case SDLK_2:
        renderer->useNormalProgram();
        break;
    case SDLK_3:
        renderer->setLightMapFilter(LightMapFilter::Nearest);
        renderer->useLightMapProgram();
        break;
    case SDLK_4:
        renderer->setLightMapFilter(LightMapFilter::Linear);
        renderer->useLightMapProgram();
        break;
    }
}

static void onKeyUp(const SDL_KeyboardEvent &event)
{
    switch(event.keysym.sym)
    {
    case SDLK_ESCAPE:
        quitting = true;
        break;
    case SDLK_a:
        if(cameraVelocity.x < 0)
            cameraVelocity.x = 0;
        break;
    case SDLK_d:
        if(cameraVelocity.x > 0)
            cameraVelocity.x = 0;
        break;
    case SDLK_w:
        if(cameraVelocity.z < 0)
            cameraVelocity.z = 0;
        break;
    case SDLK_s:
        if(cameraVelocity.z > 0)
            cameraVelocity.z = 0;
        break;
    case SDLK_SPACE:
        if(cameraVelocity.y > 0)
            cameraVelocity.y = 0;
        break;
    case SDLK_LCTRL:
        if(cameraVelocity.y < 0)
            cameraVelocity.y = 0;
        break;
    case SDLK_LEFT:
        if(cameraAngularVelocity.y > 0)
            cameraAngularVelocity.y = 0.0f;
        break;
    case SDLK_RIGHT:
        if(cameraAngularVelocity.y < 0)
            cameraAngularVelocity.y = 0.0f;
        break;
    case SDLK_UP:
        if(cameraAngularVelocity.x > 0)
            cameraAngularVelocity.x = 0.0f;
        break;
    case SDLK_DOWN:
        if(cameraAngularVelocity.x < 0)
            cameraAngularVelocity.x = 0.0f;
        break;
    case SDLK_LSHIFT:
        cameraSpeed = CameraSlowSpeed;
        break;
    }
}

static void processEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
        case SDL_KEYDOWN:
            onKeyDown(event.key);
            break;
        case SDL_KEYUP:
            onKeyUp(event.key);
            break;
        case SDL_QUIT:
            quitting = true;
            break;
        }
    }
}

static void update(float delta)
{
    cameraAngle += cameraAngularVelocity*delta;
    glm::mat3 cameraOrientation = glm::rotate(glm::mat4(), cameraAngle.y, glm::vec3(0,1,0)) * glm::rotate(glm::mat4(), cameraAngle.x, glm::vec3(1,0,0));

    camera->setOrientation(cameraOrientation);
    camera->setPosition(camera->getPosition() + cameraOrientation*(cameraVelocity*delta*cameraSpeed));
}

static void render()
{
    if(renderer)
    {
        renderer->beginFrame();
        scene->prepareForRendering();

        renderer->renderScene(camera, scene);
        renderer->endFrame();
    }

    SDL_GL_SwapWindow(window);
}

static void createScene()
{
    scene = std::make_shared<Scene> ();

    // Create the camera
    camera = std::make_shared<Camera> ();
    camera->perspective(60.0, float(screenWidth) / float(screenHeight), 0.1f, 1000.0f);
    camera->setPosition(glm::vec3(0.0, 0.60, 1.25));
    scene->addObject(camera);

    // Create the static geometry
    {
        auto staticGeometry = std::make_shared<SceneMeshObject> ();
        staticGeometry->setMesh(GenericMeshBuilder()

            // Add the walls
            .identity()
            .translate(0, 1.0, 0)
            .addCubeInterior(glm::vec3(4.0, 2.0, 4.0))

            // Add a cube
            .identity()
            .translate(0, 0.30, 0)
            .addCube(glm::vec3(0.6, 0.6, 0.6))
            .mesh()
        );
        scene->addObject(staticGeometry);
    }

    // Create the light
    {
        spotLight = std::make_shared<Light> ();
        spotLight->setType(LightType::Spot);
        spotLight->setPosition(glm::vec3(0.0f, 1.5f, -0.5f));
        spotLight->lookDown();
        spotLight->setSpotCutoff(glm::vec2(70, 60));
        spotLight->setAttenuationFactors(glm::vec3(1.0f, 0.0f, 3.0f));
        scene->addObject(spotLight);
    }
}

int main(int argc, char* argv[])
{
    if(SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Failed to initialize SDL2\n");
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    window = SDL_CreateWindow("Radiosity Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, SDL_WINDOW_OPENGL);
    if(!window)
    {
        fprintf(stderr, "Failed to create the SDL2 window.\n");
        return 0;
    }

    glContext = SDL_GL_CreateContext(window);
    if(!glContext)
    {
        fprintf(stderr, "Failed to create the OpenGL context.\n");
        return 0;
    }

    if(SDL_GL_MakeCurrent(window, glContext))
    {
        fprintf(stderr, "Failed to use the created OpenGL context.\n");
        return 0;
    }

    // TODO: Add a FPS counter
    SDL_GL_SetSwapInterval(1);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Failed to initialize GLEW.\n");
        return 0;
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    renderer = std::make_shared<Renderer> ();
    if(!renderer->initialize())
    {
        fprintf(stderr, "Failed to initialize the renderer.\n");
        return 0;
    }

    createScene();

    lightmapProcess = std::make_shared<LightmapBuildProcess> ();
    lightmapProcess->setScene(scene);
    lightmapProcess->start();

    auto lastTime = SDL_GetTicks();

    while(!quitting)
    {
        processEvents();

        auto newTime = SDL_GetTicks();
        auto deltaTime = newTime - lastTime;
        lastTime = newTime;

        update(deltaTime * 0.001f);
        render();
        //SDL_Delay(5);
    }

    lightmapProcess->shutdown();
    SDL_GL_MakeCurrent(nullptr, nullptr);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
