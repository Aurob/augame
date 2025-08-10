
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>
#include <climits>
#include <cmath>

extern float deltaTime;
extern GameState gameState;
extern entt::entity _player;

#include <cstdio> // For printf

// Helper function to select the main camera based on priority, position, and player interior logic
entt::entity selectMainCamera(entt::registry &registry) {
    auto cameraView = registry.view<Camera, Position, Shape>();
    if(cameraView.begin() == cameraView.end()) return entt::null;

    // Log the current camera mode for debugging
    static bool lastCameraMode = false;
    if (gameState.playerCameraMode != lastCameraMode) {
        printf("[DEBUG] Camera mode toggled. playerCameraMode = %d\n", (int)gameState.playerCameraMode);
        lastCameraMode = gameState.playerCameraMode;
    }

    // Gather player info
    bool playerExists = (_player != entt::null);
    bool playerIsInside = (playerExists && registry.all_of<Inside>(_player));
    entt::entity playerInterior = entt::null;
    Position playerPos = {0, 0, 0, 0, 0, 0};

    if(playerExists) {
        if(registry.all_of<Position>(_player)) {
            playerPos = registry.get<Position>(_player);
        }
        if(playerIsInside) {
            auto playerInside = registry.get<Inside>(_player);
            playerInterior = playerInside.interior;
        }
    }

    // Determine if we are in player camera mode (player context)
    bool usePlayerContext = (playerExists && gameState.playerCameraMode);

    if (usePlayerContext && playerExists) {
        // Player context: Use the player's own camera, or add one if not present
        entt::entity playerCamera = entt::null;
        if (registry.all_of<Camera>(_player)) {
            playerCamera = _player;
        } else {
            // Add a Camera to the player if not present
            Camera defaultCamera;
            registry.emplace<Camera>(_player, defaultCamera);
            playerCamera = _player;
        }
        return playerCamera;
    } else {
        // Global camera mode
        entt::entity playerCamera = entt::null;
        if (playerExists && registry.all_of<Camera>(_player)) {
            playerCamera = _player;
        }

        // Find the closest camera in the same "Inside" as the player (or both outside)
        entt::entity closestCamera = entt::null;
        float closestDist = std::numeric_limits<float>::max();

        // Determine the player's "inside" context (entt::null means outside)
        entt::entity playerInsideContext = entt::null;
        if (playerExists && registry.all_of<Inside>(_player)) {
            playerInsideContext = registry.get<Inside>(_player).interior;
        }

        for (auto candidateCamera : cameraView) {
            // In global mode, do NOT allow the camera entity to also have Player
            if (!gameState.playerCameraMode && registry.all_of<Player>(candidateCamera)) {
                continue;
            }

            // Camera's "inside" context (entt::null means outside)
            entt::entity cameraInsideContext = entt::null;
            if (registry.all_of<Inside>(candidateCamera)) {
                cameraInsideContext = registry.get<Inside>(candidateCamera).interior;
            }

            // Only consider cameras in the same "inside" context as the player (null means outside)
            if (cameraInsideContext != playerInsideContext)
                continue;

            // Find distance to player (if player exists)
            float dist = 0.0f;
            if (playerExists && registry.all_of<Position>(_player)) {
                auto& camPos = registry.get<Position>(candidateCamera);
                float dx = playerPos.x - camPos.x;
                float dy = playerPos.y - camPos.y;
                dist = std::sqrt(dx * dx + dy * dy);
            }

            if (dist < closestDist) {
                closestDist = dist;
                closestCamera = candidateCamera;
            }
        }

        // If a camera in the same Inside as the player was found, use it
        if (closestCamera != entt::null) {
            return closestCamera;
        }

        // If no camera in the same Inside as the player, but the player has a camera, use the player's camera (or add one)
        if (playerCamera != entt::null) {
            return playerCamera;
        } else if (playerExists) {
            // Add a Camera to the player if not present
            Camera defaultCamera;
            registry.emplace<Camera>(_player, defaultCamera);
            return _player;
        }

        // Fallback: if no suitable camera found, use the first available camera
        closestCamera = entt::null;
        if (!gameState.playerCameraMode) {
            for (auto candidateCamera : cameraView) {
                if (!registry.all_of<Player>(candidateCamera)) {
                    closestCamera = candidateCamera;
                    break;
                }
            }
            // If all cameras are Player, fallback to first anyway
            if (closestCamera == entt::null) {
                closestCamera = cameraView.front();
            }
        } else {
            closestCamera = cameraView.front();
        }

        return closestCamera;
    }
}

void updatePositions(entt::registry &registry)
{
    // Log C key press for debugging
    static bool lastPlayerCameraMode = false;
    if (gameState.playerCameraMode != lastPlayerCameraMode) {
        printf("[DEBUG] updatePositions: playerCameraMode toggled to %d\n", (int)gameState.playerCameraMode);
        lastPlayerCameraMode = gameState.playerCameraMode;
    }

    // Select the main camera using our helper function
    entt::entity cameraEntity = selectMainCamera(registry);
    if(cameraEntity == entt::null) return;
    
    auto& camera = registry.get<Camera>(cameraEntity);
    Position cameraPos = registry.get<Position>(cameraEntity);
    Shape cameraShape = registry.get<Shape>(cameraEntity);

    bool cameraIsInside = registry.all_of<Inside>(cameraEntity);
    entt::entity cameraInterior;
    if(cameraIsInside) {
        auto cameraInside = registry.get<Inside>(cameraEntity);
        cameraInterior = cameraInside.interior;
    }

    // Also update Cursor positions if present
    auto cursorEntities = registry.view<Cursor>();
    for (auto entity : cursorEntities)
    {
    }


    auto entities = registry.view<Position, Shape>();
    for (auto entity : entities)
    {
        bool logit;

        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);

        float posX = (cameraPos.x - position.x) * camera.gridSpacing + gameState.width / 2;
        float posY = (cameraPos.y - position.y) * camera.gridSpacing + gameState.height / 2;
        float posZ = (position.z) * camera.gridSpacing + gameState.height / 2;

        position.sx = (2 * posX / gameState.width - 1) / camera.defaultGSV - shape.scaled_size.x;
        position.sy = (2 * posY / gameState.height - 1) / camera.defaultGSV - shape.scaled_size.y;
        position.sz = (2 * posZ / gameState.height - 1) / camera.defaultGSV - shape.scaled_size.z;


        if(entity == _player) {
            registry.emplace_or_replace<Visible>(entity);
            registry.emplace_or_replace<InView>(entity);
            continue;
        }

        bool isWithinBounds = (
            position.sx + shape.scaled_size.x >= -1.1 
            && position.sx - shape.scaled_size.x <= 1.1 
            && (position.sy + shape.scaled_size.y + shape.scaled_size.z*2 + shape.scaled_size.y) >= -1.1
            && (position.sy - shape.scaled_size.y - shape.scaled_size.z*2 - shape.scaled_size.y) <= 1.1
        );


        bool isVisible = true;
        bool isInView = true;

        if (isWithinBounds)
        {
            bool entityIsPortal = registry.all_of<InteriorPortal>(entity);

            if(registry.all_of<Inside>(entity)) { 
                auto _interior = registry.get<Inside>(entity).interior;

                if(!entityIsPortal) {
                    if(!cameraIsInside) {
                        bool hideInside = registry.get<Interior>(_interior).hideInside;
                        if(hideInside) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else if(entityIsPortal) {
                        if(_interior != cameraInterior) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else {

                        if(entity != cameraInterior && _interior != cameraInterior) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                }
                else {
                    auto portal = registry.get<InteriorPortal>(entity);
                    if(cameraIsInside) {
                        auto cameraInside = registry.get<Inside>(cameraEntity);

                        if(cameraInside.interior != portal.A && cameraInside.interior != portal.B) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else {
                        if(portal.A != entt::null && portal.B != entt::null) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                }
            }
            else if(cameraIsInside) {
                auto cameraInside = registry.get<Inside>(cameraEntity);
                if(registry.all_of<Interior>(entity)) {
                    if(entity != cameraInside.interior) {
                        isVisible = false;
                        isInView = false;
                    }
                }
                else {
                    if(!registry.all_of<Inside>(entity)) {
                        isVisible = false;
                        isInView = false;
                    }
                }
            }
        }
        
        if (isVisible) {
            registry.emplace_or_replace<Visible>(entity);
        } else if (registry.all_of<Visible>(entity)) {
            registry.remove<Visible>(entity);
        }
        if (isInView) {
            registry.emplace_or_replace<InView>(entity);
            if (registry.all_of<PhysicsBodyRect>(entity) && !registry.all_of<InteriorPortal>(entity)) {
                auto& physBody = registry.get<PhysicsBodyRect>(entity);
                // physBody.body->ignore = false;
            }
        }
        else if (registry.all_of<InView>(entity)) {
            // registry.remove<InView>(entity);
            // if (registry.all_of<PhysicsBodyRect>(entity)) {
            //     auto& physBody = registry.get<PhysicsBodyRect>(entity);
            //     physBody.body->ignore = true;
            // }
        }
    }

}


// Adjust entity Shape scaled sizes based on current zoom level
// TODO: Only update shapes if a zoom update occurred
void updateShapes(entt::registry &registry)
{
    // update shapes
    auto entities = registry.view<Shape>();
    
    // Select the main camera using our helper function
    entt::entity mainCameraEntity = selectMainCamera(registry);
    if(mainCameraEntity == entt::null) return;
    
    auto& camera = registry.get<Camera>(mainCameraEntity);
    
    // Pre-calculate common scaling factors
    float xScale = camera.gridSpacing / (camera.defaultGSV * gameState.width);
    float yScale = camera.gridSpacing / (camera.defaultGSV * gameState.height);
    
    for (auto &entity : entities)
    {
        auto &shape = entities.get<Shape>(entity);
        shape.scaled_size.x = shape.size.x * xScale;
        shape.scaled_size.y = shape.size.y * yScale;
    }
}