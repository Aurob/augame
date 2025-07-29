#pragma once

#include <SDL2/SDL.h>
#include "lib/entt.hpp"
#include "structs.hpp"
#include "../include/lib/physics.hpp"

extern entt::entity _player;
extern entt::registry registry;
extern p2d::Physics physics;
extern GameState gameState;

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

    // Handle downtime increment/reset for Cursor while button is held or released
    auto view = registry.view<Cursor, Player>();
    for (auto entity : view)
    {
        auto &cursor = view.get<Cursor>(entity);
        if (playerKeys[SDL_BUTTON_LEFT])
        {
            cursor.downtime += 1;
        }
        else
        {
            cursor.downtime = 0;
        }
    }

    // Mouse/Touch position
    if (event->type == SDL_MOUSEMOTION || event->type == SDL_FINGERMOTION)
    {
        auto view = registry.view<Cursor, Player>();
        for (auto entity : view)
        {
            auto &cursor = view.get<Cursor>(entity);

            // Get player position
            auto &playerPos = registry.get<Position>(_player);

            // Get cursor position in screen coordinates
            float screenX, screenY;
            if (event->type == SDL_MOUSEMOTION)
            {
                screenX = event->motion.x;
                screenY = event->motion.y;
                cursor.position.x = screenX;
                cursor.position.y = screenY;
            }
            else
            {
                screenX = event->tfinger.x * gameState.width;
                screenY = event->tfinger.y * gameState.height;
                cursor.position.x = screenX;
                cursor.position.y = screenY;
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

    if(gameState.gameState > 0) {
        // Find camera for zoom adjustments
        auto cameraView = registry.view<Camera>();
        if(cameraView.begin() != cameraView.end()) {
            auto& camera = registry.get<Camera>(cameraView.front());
        
        // Zoom in and out (Mouse wheel and pinch)
        if (event->type == SDL_MOUSEWHEEL)
        {
            if (event->wheel.y > 0)
            {
                camera.gridSpacing *= 1.08f;
            }
            else if (event->wheel.y < 0)
            {
                camera.gridSpacing /= 1.08f;
            }
        }
        else if (event->type == SDL_MULTIGESTURE)
        {
            if (event->mgesture.numFingers == 2)
            {
                if (event->mgesture.dDist > 0)
                {
                    camera.gridSpacing *= (1.0f + event->mgesture.dDist);
                }
                else if (event->mgesture.dDist < 0)
                {
                    camera.gridSpacing /= (1.0f - event->mgesture.dDist);
                }
            }
        }
        } // Close camera view check
    }
}

void processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        EventHandler(0, &event);
    }

    auto key_entities = registry.view<Keys>();
    for (auto e: key_entities) {
        auto &keys = key_entities.get<Keys>(e).keys;

        if(gameState.gameState > 0) {
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

            // Speed Boost
            if (registry.all_of<Movement>(e)) {
                auto &movement = registry.get<Movement>(e);
                if (keys[SDLK_LSHIFT]) {
                    movement.speed = movement.default_speed * 10;
                }
                else if(movement.speed != movement.default_speed) {
                    movement.speed = movement.default_speed;
                }
            }

            // Update player's TextureAlts based on direction and movement
            if (registry.all_of<TextureAlts>(e)) {
                auto& textureAlts = registry.get<TextureAlts>(e);
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

                // If entity has Rotation, set angle to 90 degree increments based on direction
                if (registry.all_of<Rotation>(e)) {
                    auto& rotation = registry.get<Rotation>(e);
                    // Use .angle field as defined in structs.hpp
                    if (lastDirection == "Up") {
                        rotation.angle = 270.0f;
                    } else if (lastDirection == "Down") {
                        rotation.angle = 90.0f;
                    } else if (lastDirection == "Left") {
                        rotation.angle = 180.0f;
                    } else if (lastDirection == "Right") {
                        rotation.angle = 0.0f;
                    }
                }
            }
        }

        // Check for ESC key press to toggle gameState between 1 and 0
        if ((gameState.gameState == 0 || gameState.gameState == 1)  && keys[SDLK_ESCAPE]) {
            if (gameState.gameState == 1) {
                gameState.gameState = 0;
            } else {
                gameState.gameState = 1;
            }
            keys[SDLK_ESCAPE] = false; // Prevent repeated toggling while holding ESC
        }

        if (gameState.gameState == 0 && keys[SDLK_1]) {
            gameState.gameState = -1;
            keys[SDLK_1] = false;
        }

        if (gameState.gameState == -1) {
            if (keys[SDL_BUTTON_LEFT]) {
                gameState.gameState = 1;
            }
        }
    }
}