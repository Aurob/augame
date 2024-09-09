#pragma once

#include <SDL2/SDL.h>
#include "entt.hpp"
#include "structs.hpp"
#include "../include/imgui/imgui.h"
#include "../include/imgui/imgui_impl_sdl.h"
#include "../include/imgui/imgui_impl_opengl3.h"

using namespace std;

extern int width, height;
extern GLfloat cursorPos[2];
extern float gridSpacingValue;
extern entt::entity _player;
extern entt::registry registry;

void EventHandler(int type, SDL_Event *event)
{
    // wasd for offset
    auto &playerKeys = registry.get<Keys>(_player).keys;
    if (event->type == SDL_KEYDOWN)
    {
        playerKeys[event->key.keysym.sym] = true;
    }
    else if (event->type == SDL_KEYUP)
    {
        playerKeys[event->key.keysym.sym] = false;
    }

    // Mouse/Touch position
    if (event->type == SDL_MOUSEMOTION || event->type == SDL_FINGERMOTION)
    {
        if (event->type == SDL_MOUSEMOTION)
        {
            cursorPos[0] = event->motion.x;
            cursorPos[1] = event->motion.y;
        }
        else
        {
            cursorPos[0] = event->tfinger.x * width;
            cursorPos[1] = event->tfinger.y * height;
        }
    }

    // Mouse/Touch Interactions
    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_FINGERDOWN)
    {
        playerKeys[SDL_BUTTON_LEFT] = true;
    }
    else if (event->type == SDL_MOUSEBUTTONUP || event->type == SDL_FINGERUP)
    {
        playerKeys[SDL_BUTTON_LEFT] = false;
    }

    // Zoom in and out (Mouse wheel and pinch)
    if (event->type == SDL_MOUSEWHEEL)
    {
        if (event->wheel.y > 0)
        {
            gridSpacingValue *= 1.08f;
        }
        else if (event->wheel.y < 0)
        {
            gridSpacingValue /= 1.08f;
        }
    }
    else if (event->type == SDL_MULTIGESTURE)
    {
        if (event->mgesture.numFingers == 2)
        {
            if (event->mgesture.dDist > 0)
            {
                gridSpacingValue *= (1.0f + event->mgesture.dDist);
            }
            else if (event->mgesture.dDist < 0)
            {
                gridSpacingValue /= (1.0f - event->mgesture.dDist);
            }
        }
    }
}

void processEvents(void (*EventHandler)(int, SDL_Event*), context *ctx) {
    while (SDL_PollEvent(&ctx->event))
    {
        ImGui_ImplSDL2_ProcessEvent(&ctx->event);
        EventHandler(0, &ctx->event);
    }

    auto keys = registry.get<Keys>(_player).keys;

    // If Shift is pressed, increase speed
    auto& playerMovement = registry.get<Movement>(_player);
    // if(keys[SDLK_LSHIFT]) {
    //     playerMovement.speed = defaultMoveSpeed * 20;
    // }
    // else {
    //     playerMovement.speed = defaultMoveSpeed;
    // }
    
    // If B increase player z
    if(keys[SDLK_b]) {
        auto& playerPos = registry.get<Position>(_player);
        playerPos.z += 1;
        keys[SDLK_b] = false;
    }
    // If N decrease player z
    if(keys[SDLK_n]) {
        auto& playerPos = registry.get<Position>(_player);
        playerPos.z -= 1;
        keys[SDLK_n] = false;
    }
    
    // Update player's TextureAlts based on direction and movement
    if (registry.all_of<TextureAlts>(_player)) {
        auto& textureAlts = registry.get<TextureAlts>(_player);
        bool isMoving = keys[SDLK_w] || keys[SDLK_s] || keys[SDLK_a] || keys[SDLK_d];
        std::string action = isMoving ? "Run" : "Idle";
        static std::string lastDirection = "Down"; // Static variable to remember last direction

        if (keys[SDLK_w]) {
            lastDirection = "Up";
        } else if (keys[SDLK_s]) {
            lastDirection = "Down";
        } else if (keys[SDLK_a]) {
            lastDirection = "Left";
        } else if (keys[SDLK_d]) {
            lastDirection = "Right";
        }

        textureAlts.current = action + "_" + lastDirection;
    }

}