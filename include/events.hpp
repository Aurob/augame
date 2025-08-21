#pragma once

#include <SDL2/SDL.h>
#include "lib/entt.hpp"
#include "structs.hpp"
#include "../include/lib/physics.hpp"
#include "../include/Systems/ViewSystems.hpp"
#include "../include/SceneManager.hpp"
#include "../include/WebUtils.hpp"

extern entt::entity _player;
extern SceneManager sceneManager;
extern p2d::Physics physics;
extern GameState gameState;
extern MetaData metaData;

void EventHandler(int type, SDL_Event *event)
{
    // wasd for offset
    auto &playerKeys = sceneManager.getCurrentRegistry().get<Keys>(_player).keys;
    if (event->type == SDL_KEYDOWN)
    {
        playerKeys[event->key.keysym.sym] = true;
        
        // Scene switching with Shift+< and Shift+>
        if ((event->key.keysym.mod & KMOD_SHIFT) && 
            event->key.keysym.sym == SDLK_COMMA) {
            switchToPrevScene();
        }
        else if ((event->key.keysym.mod & KMOD_SHIFT) && 
                 event->key.keysym.sym == SDLK_PERIOD) {
            switchToNextScene();
        }
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
    auto view = sceneManager.getCurrentRegistry().view<Cursor, Player>();
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
        auto view = sceneManager.getCurrentRegistry().view<Cursor, Player>();
        for (auto entity : view)
        {
            auto &cursor = view.get<Cursor>(entity);

            // Get player position
            auto &playerPos = sceneManager.getCurrentRegistry().get<Position>(_player);

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
        // Find camera for zoom adjustments using priority-based selection
        entt::entity cameraEntity = selectMainCamera(sceneManager.getCurrentRegistry());
        if(cameraEntity != entt::null) {
            auto& camera = sceneManager.getCurrentRegistry().get<Camera>(cameraEntity);
        
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

    auto key_entities = sceneManager.getCurrentRegistry().view<Keys>();
    for (auto e: key_entities) {
        auto &keys = key_entities.get<Keys>(e).keys;

        if(gameState.gameState > 0) {
            // If B increase player z
            if(keys[SDLK_b]) {
                auto& playerPos = sceneManager.getCurrentRegistry().get<Position>(_player);
                playerPos.z += 1;
                keys[SDLK_b] = false;
            }
            // If N decrease player z
            if(keys[SDLK_n]) {
                auto& playerPos = sceneManager.getCurrentRegistry().get<Position>(_player);
                playerPos.z -= 1;
                keys[SDLK_n] = false;
            }
            
            // Get _player shape and increase by 10 only when RSHIFT and '/' are held
            if (keys[SDLK_RSHIFT] && keys[SDLK_SLASH]) {
                if (sceneManager.getCurrentRegistry().all_of<Shape>(_player)) {
                    auto& playerShape = sceneManager.getCurrentRegistry().get<Shape>(_player);
                    playerShape.size.x += 10;
                    playerShape.size.y += 10;
                    playerShape.size.z += 10;
                }
            }

            // Speed Boost
            if (sceneManager.getCurrentRegistry().all_of<Movement>(e)) {
                auto &movement = sceneManager.getCurrentRegistry().get<Movement>(e);
                if (keys[SDLK_LSHIFT]) {
                    movement.speed = movement.default_speed * 2;
                }
                else if(movement.speed != movement.default_speed) {
                    movement.speed = movement.default_speed;
                }
            }

            // Update player's TextureAlts based on direction and movement
            if (sceneManager.getCurrentRegistry().all_of<TextureAlts>(e)) {
                auto& textureAlts = sceneManager.getCurrentRegistry().get<TextureAlts>(e);
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
                if (sceneManager.getCurrentRegistry().all_of<Rotation>(e)) {
                    auto& rotation = sceneManager.getCurrentRegistry().get<Rotation>(e);
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

        // Check for ESC key press to toggle gameState between 0 (paused) and 1 (camera)
        if ((gameState.gameState == 0 || gameState.gameState == 1) && keys[SDLK_ESCAPE]) {
            gameState.gameState = (gameState.gameState == 1) ? 0 : 1;
            keys[SDLK_ESCAPE] = false; // Prevent repeated toggling while holding ESC
        }
        
        // Check for C key press to toggle camera mode when player exists
        if (gameState.gameState == 1 && _player != entt::null && keys[SDLK_c]) {
            gameState.playerCameraMode = !gameState.playerCameraMode;
            keys[SDLK_c] = false; // Prevent repeated toggling while holding C
        }

        // Check for '1' key press to go from pause menu to start menu
        if (gameState.gameState == 0 && keys[SDLK_1]) {
            gameState.gameState = -1;
            keys[SDLK_1] = false; // Prevent repeated toggling while holding 1
        }

        bool temp_skip = false;
        // Transition from start menu to first cutscene
        if (gameState.gameState == -1 && keys[SDL_BUTTON_LEFT]) {
            gameState.gameState = 1;
            temp_skip = true;
        }

        // Handle cutscene progression
        if (gameState.gameState >= 1) {
            if (keys[SDL_BUTTON_LEFT]) {
                if(gameState.gameState == 1 && temp_skip) {
                    // Transition from start menu, find first slide or go to gameplay
                    int firstSlide = -1;
                    for (const auto& slide : sceneManager.getCurrentMetadata().slides) {
                        if (firstSlide == -1 || slide.first < firstSlide) {
                            firstSlide = slide.first;
                        }
                    }
                    gameState.gameState = (firstSlide > 1) ? firstSlide : 1;
                } else if (gameState.gameState == 1 && !temp_skip) {
                    // In gameplay, clicking should NOT change gameState - do nothing
                    // This preserves normal gameplay interactions
                    // Do NOT clear SDL_BUTTON_LEFT here - let interaction system handle it
                    return; // Exit early to avoid clearing button state
                } else {
                    // In a slide, find the next slide or return to gameplay
                    int maxSlideId = 1;
                    int nextSlide = -1;
                    
                    for (const auto& slide : sceneManager.getCurrentMetadata().slides) {
                        if (slide.first > maxSlideId) {
                            maxSlideId = slide.first;
                        }
                        if (slide.first > gameState.gameState && (nextSlide == -1 || slide.first < nextSlide)) {
                            nextSlide = slide.first;
                        }
                    }
                    
                    if (gameState.gameState >= maxSlideId || nextSlide == -1) {
                        gameState.gameState = 1; // Transition to gameplay after last slide
                    } else {
                        gameState.gameState = nextSlide; // Proceed to next slide
                    }
                }

                keys[SDL_BUTTON_LEFT] = false;
            }
        }
    }
}