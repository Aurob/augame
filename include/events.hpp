#pragma once

#include <SDL2/SDL.h>
#include "lib/entt.hpp"
#include "structs.hpp"
#include "../include/lib/physics.hpp"

extern int width, height;
extern float gridSpacingValue;
extern entt::entity _player;
extern entt::registry registry;
extern p2d::Physics physics;

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


    // Mouse/Touch Interactions
    if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_FINGERDOWN)
    {
        playerKeys[SDL_BUTTON_LEFT] = true;
    }
    else if (event->type == SDL_MOUSEBUTTONUP || event->type == SDL_FINGERUP)
    {
        playerKeys[SDL_BUTTON_LEFT] = false;

    }


    // Mouse/Touch position
    if (event->type == SDL_MOUSEMOTION || event->type == SDL_FINGERMOTION)
    {
        auto view = registry.view<Cursor, Player>();
        for (auto entity : view)
        {
            auto &cursor = view.get<Cursor>(entity);
            if (event->type == SDL_MOUSEMOTION)
            {
                cursor.position.x = event->motion.x;
                cursor.position.y = event->motion.y;
            }
            else
            {
                cursor.position.x = event->tfinger.x * width;
                cursor.position.y = event->tfinger.y * height;
            }

            if (playerKeys[SDL_BUTTON_LEFT]) {
                if (cursor.firstdown == false) {
                    cursor.firstdown = true;
                    if (cursor.firstup == true) {
                        cursor.firstup = false;
                    }
                }
            } else {
                if (cursor.firstdown == true) {
                    cursor.firstdown = false;
                    cursor.firstup = true;
                }
            }
        }
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

void processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        EventHandler(0, &event);
    }

    auto keys = registry.get<Keys>(_player).keys;
    
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
    
    // Get _player shape and increase by 10 only when RSHIFT and '/' are held
    if (keys[SDLK_RSHIFT] && keys[SDLK_SLASH]) {
        if (registry.all_of<Shape>(_player)) {
            auto& playerShape = registry.get<Shape>(_player);
            playerShape.size.x += 10;
            playerShape.size.y += 10;
            playerShape.size.z += 10;
        }
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

    auto &playerKeys = registry.get<Keys>(_player).keys;

}