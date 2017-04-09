#include "SDL.h"
#include "SDL_main.h"
#include "GLCommon.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "SceneObject.hpp"
#include "Camera.hpp"
#include "GenericMesh.hpp"

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
glm::vec3 cameraVelocity;


static void onKeyDown(const SDL_KeyboardEvent &event)
{
    switch(event.keysym.sym)
    {
    case SDLK_ESCAPE:
        quitting = true;
        break;
    }
}

static void onKeyUp(const SDL_KeyboardEvent &event)
{
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

    camera = std::make_shared<Camera> ();
    scene->addObject(camera);

    // Add a cube in the scene of the scene.
    {
        auto cube = std::make_shared<SceneMeshObject> ();
        cube->setMesh(GenericMeshBuilder()
            .addCube(glm::vec3(0.5, 0.5, 0.5))
            .mesh()
        );
        scene->addObject(cube);
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

    while(!quitting)
    {
        processEvents();
        render();
        SDL_Delay(5);
    }

    SDL_GL_MakeCurrent(nullptr, nullptr);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
