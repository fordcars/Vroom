#include "UISys.hpp"

#include <memory>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

// Static
UISys& UISys::get() {
    static std::unique_ptr<UISys> instance = std::make_unique<UISys>();
    return *instance;
}

bool UISys::init(SDL_Window* window, SDL_GLContext gl_context) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

    return true;
}

void UISys::handleEvent(const SDL_Event& event) { ImGui_ImplSDL2_ProcessEvent(&event); }

void UISys::update(float deltaTime) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // Show demo window! :)

    if(mShowFPSOverlay) showFPSOverlay();
}

void UISys::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UISys::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void UISys::showFPSOverlay()
{
    ImGui::SetNextWindowSize(ImVec2(100, 50)); // Set a fixed size
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always); // Fixed position

    ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
}
