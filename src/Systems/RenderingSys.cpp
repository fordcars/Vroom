#include "RenderingSys.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp> // For lookAt()

#include "Constants.hpp"
#include "Log.hpp"

#include "Entities/EntityFilter.hpp"
#include "ResourceSys/Obj/ObjResource.hpp"

// Static
RenderingSys& RenderingSys::get() {
    static std::unique_ptr<RenderingSys> instance = std::make_unique<RenderingSys>();
    return *instance;
}

RenderingSys::~RenderingSys() {
    SDL_GL_DeleteContext(mContext);
}

bool RenderingSys::init(SDL_Window* window) {
    Log::info() << "Initializing graphics...";

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, Constants::OPENGL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, Constants::OPENGL_MINOR_VERSION);
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create context
    mContext = SDL_GL_CreateContext(window);
    if(!mContext) {
        Log::sdlError() << "Unable to create OpenGL context! This game requires OpenGL "
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

void RenderingSys::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderingSys::render(SDL_Window* window) {
    EntityFilter<PositionComp, RenderableComp> filter;
    for(const auto& [position, renderable] : filter) {
        renderEntity(position, renderable);
    }
    CameraEntity::instances[0].get<PositionComp>().coords.x += 0.01; // TODO: remove
    CameraEntity::instances[0].get<PositionComp>().coords.y -= 0.01; // TODO: remove

    // Check for gl error
    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        Log::glError(error) << "Encountered an OpenGL error while rendering!";
    }
    SDL_GL_SwapWindow(window);
}

void RenderingSys::initGL(SDL_Window* window) {
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

    glEnable(GL_MULTISAMPLE); // Enable anti-aliasing (SDL attributes)
}

void RenderingSys::renderEntity(const PositionComp& position,
                                const RenderableComp& renderable) {
    const CameraEntity& camera = CameraEntity::instances[0];
    glm::mat4 MVP = getProjectionMatrix(camera) * getViewMatrix(camera) * getModelMatrix(position);

    glUseProgram(renderable.shader->getId());
    glUniformMatrix4fv(renderable.shader->findUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    if(renderable.shader->findUniformBlock("ObjMaterialsBlock") != -1) {
        glBindBufferBase(
            GL_UNIFORM_BUFFER,
            renderable.shader->findUniformBlock("ObjMaterialsBlock"),
            renderable.objectResource->materialUniformBuffer.getId()
        );
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Pass vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderable.objectResource->vertexBuffer.getId());
    glVertexAttribPointer(
        0,					// Attribute index
        3,					// Size. Number of values per vertex, must be 1, 2, 3 or 4.
        GL_FLOAT,			// Type of data
        GL_FALSE,			// Normalized?
        0,					// Stride
        (void*)0			// Array buffer offset
    );

    // Pass material id buffer
    glBindBuffer(GL_ARRAY_BUFFER, renderable.objectResource->materialIdBuffer.getId());
    glVertexAttribIPointer(
        1,					// Attribute index
        1,					// Size. Number of values per vertex, must be 1, 2, 3 or 4.
        GL_INT,		     	// Type of data
        0,					// Stride
        (void*)0			// Array buffer offset
    );

    // Render all meshes
    for(const auto& mesh : renderable.meshes) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer.getId());
        glDrawElements(
            GL_TRIANGLES,            // Mode
            mesh->indexBuffer.getCount(),
            GL_UNSIGNED_INT,         // Type
            (void*)0                 // Element array buffer offset
        );
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

glm::mat4 RenderingSys::getModelMatrix(const PositionComp& position) {
    return glm::translate(glm::mat4(1.0f), position.coords);
}

glm::mat4 RenderingSys::getViewMatrix(const CameraEntity& camera) {
    glm::vec3 position = camera.get<PositionComp>().coords;
    const CameraInfoComp& info = camera.get<CameraInfoComp>();

    glm::vec3 vec3Direction;
    if(info.direction.w == 0) {
        // Direction is a vector
        vec3Direction = glm::vec3(info.direction) + position;
    } else {
        // Direction is a point (position)
        vec3Direction = glm::vec3(info.direction);
    }

    return glm::lookAt(position, vec3Direction, info.upVector);
}

glm::mat4 RenderingSys::getProjectionMatrix(const CameraEntity& camera) {
    glm::vec3 position = camera.get<PositionComp>().coords;
    const CameraInfoComp& info = camera.get<CameraInfoComp>();
    float vertFOX = 2 * atan( tan(info.horizFOV / 2) / info.aspectRatio);
    
    return glm::perspective(
        vertFOX,
        info.aspectRatio,
        info.nearClippingPlane,
        info.farClippingPlane
    );
}