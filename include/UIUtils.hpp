#pragma once

#include <SDL_opengles2.h>
#include <cmath>
#include <array>
#include "../include/shaders.hpp"
#include "../include/json.hpp"
#include <SDL_opengles2.h>
#include <vector>
#include <unordered_map>
#include <string>

extern int width, height;
extern float gridSpacingValue;

void loadUI(SDL_Window *mpWindow, SDL_GLContext gl_context) {
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(mpWindow, gl_context);
    ImGui_ImplOpenGL3_Init("#version 100");
}



void dialogue_imgui(const std::string& initialText, float screenX, float screenY, float scaleX, float scaleY)
{
    static std::string displayText = initialText;
    static int stage = 0;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(screenX, screenY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(scaleX, scaleY), ImGuiCond_Always);
    ImGui::Begin("Dialogue", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    // Enable text wrapping
    ImGui::PushTextWrapPos(ImGui::GetWindowSize().x);
    
    // Display the text on the top half
    ImGui::TextWrapped("%s", displayText.c_str());
    ImGui::PopTextWrapPos();

    // Display buttons on the bottom half
    if (stage == 0) {
        if (ImGui::Button("Option 1")) { displayText = "You chose option 1"; stage = 1; }
        if (ImGui::Button("Option 2")) { displayText = "You chose option 2"; stage = 1; }
        if (ImGui::Button("Option 3")) { displayText = "You chose option 3"; stage = 1; }
        if (ImGui::Button("Option 4")) { displayText = "You chose option 4"; stage = 1; }
    } else if (stage == 1) {
        if (ImGui::Button("Close")) { ImGui::End(); ImGui::Render(); ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); return; }
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void test_imgui(const std::string& displayText, float screenX, float screenY, float scaleX, float scaleY)
{
    // ImGui_ImplOpenGL3_NewFrame();
    // ImGui_ImplSDL2_NewFrame();
    // ImGui::NewFrame();
    // ImGui::SetNextWindowPos(ImVec2(screenX, screenY), ImGuiCond_Always);
    // // ImGui::SetNextWindowSize(ImVec2(scaleX, scaleY), ImGuiCond_Always);
    // ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    
    // // Enable text wrapping
    // ImGui::PushTextWrapPos(ImGui::GetWindowSize().x);
    
    // // Center the text
    // ImVec2 windowSize = ImGui::GetWindowSize();
    // ImVec2 textSize = ImGui::CalcTextSize(displayText.c_str(), nullptr, true, windowSize.x);
    // ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
    // ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);
    
    // ImGui::TextWrapped("%s", displayText.c_str());
    // ImGui::PopTextWrapPos();
    // ImGui::End();
    // ImGui::Render();
    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    dialogue_imgui(displayText, screenX, screenY, scaleX, scaleY);
}


