#include "RenderingSystem.hpp"
#include <glad/glad.h>

#include "Constants.hpp"
#include "Log.hpp"

RenderingSystem::~RenderingSystem() {
    SDL_GL_DeleteContext(mContext);
}

bool RenderingSystem::init(SDL_Window* window) {
    Log::info() << "Initializing graphics...";

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, Constants::OPENGL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, Constants::OPENGL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create context
    mContext = SDL_GL_CreateContext(window);
    if(!mContext) {
        Log::sdl_error() << "Unable to create OpenGL context! This game requires OpenGL "
            << Constants::OPENGL_MAJOR_VERSION << "." << Constants::OPENGL_MINOR_VERSION
            << ", make sure your system supports it!";
        return false;
    }

    // Load GL functions
    if(gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress) == 0) {
        Log::error() << "Failed to load OpenGL!";
        return false;
    }

    Log::info() << "OpenGL: " << (const char* )glGetString(GL_VERSION);
    SDL_GL_SetSwapInterval(1);

    initGL(window);
    return true;
}

void RenderingSystem::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingSystem::render(SDL_Window* window) {
    SDL_GL_SwapWindow(window);
}

void RenderingSystem::initGL(SDL_Window* window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);

    GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

    glClearColor(
        Constants::BG_COLOR.r,
        Constants::BG_COLOR.g,
        Constants::BG_COLOR.b,
        1.0f
    );

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // Accept the fragment closer to the camera

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
