
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>

extern float deltaTime;
extern GameState gameState;
extern entt::entity _player;

// Forward declaration from ViewSystems.hpp
entt::entity selectMainCamera(entt::registry &registry);

void updateCamera(entt::registry &registry);
void updatePlayer(entt::registry &registry);

void updateCamera(entt::registry &registry) {
    // Select the main camera using our helper function
    entt::entity mainCameraEntity = selectMainCamera(registry);
    if(mainCameraEntity == entt::null) return;
    
    auto& camera = registry.get<Camera>(mainCameraEntity);
    Position &cameraPos = registry.get<Position>(mainCameraEntity);
    Shape &cameraShape = registry.get<Shape>(mainCameraEntity);
    
    // Set view offset
    camera.offset.x = ((fmod(cameraPos.x, camera.defaultGSV) * camera.gridSpacing) / camera.defaultGSV) - (cameraShape.size.x / 2);
    camera.offset.y = ((fmod(cameraPos.y, camera.defaultGSV) * camera.gridSpacing) / camera.defaultGSV) - (cameraShape.size.y / 2);

    // Set bounds
    camera.topLeftTile.x = static_cast<int>(cameraPos.x / camera.defaultGSV) - (gameState.width / camera.gridSpacing / 2);
    camera.topLeftTile.y = static_cast<int>(cameraPos.y / camera.defaultGSV) - (gameState.height / camera.gridSpacing / 2);
}

void updatePlayer(entt::registry &registry) {
    if(_player == entt::null) return;
    
    Position &playerPos = registry.get<Position>(_player);
    Shape &playerShape = registry.get<Shape>(_player);
    Cursor &playerCursorPos = registry.get<Cursor>(_player);
    Keys &playerKeys = registry.get<Keys>(_player);
    
    // Find camera for cursor calculations using our helper function
    entt::entity mainCameraEntity = selectMainCamera(registry);
    if(mainCameraEntity != entt::null) {
        auto& camera = registry.get<Camera>(mainCameraEntity);
        Position cameraPos = registry.get<Position>(mainCameraEntity);
        
        // Calculate cursor world coordinates relative to camera position (not player position)
        playerCursorPos.position.sx = cameraPos.x + ((playerCursorPos.position.x - gameState.width / 2) * camera.defaultGSV / camera.gridSpacing);
        playerCursorPos.position.sy = cameraPos.y + ((playerCursorPos.position.y - gameState.height / 2) * camera.defaultGSV / camera.gridSpacing);
    }

    // Find the entity with Id.name == "player_cursor"
    // entt::entity cursorEntity = entt::null;
    // auto idView = registry.view<Id>();
    // for (auto entity : idView) {
    //     const auto& id = idView.get<Id>(entity);
    //     if (id.name == "player_cursor") {
    //         cursorEntity = entity;
    //         break;
    //     }
    // }

    // if (cursorEntity != entt::null && registry.all_of<Texture>(cursorEntity)) {
    //     auto& texture = registry.get<Texture>(cursorEntity);
    //     if (playerKeys.keys[SDL_BUTTON_LEFT]) {
    //         if (texture.name != "hand_closed") {
    //             texture.name = "hand_closed";
    //         }
    //     } else {
    //         if (texture.name != "hand_open") {
    //             texture.name = "hand_open";
    //         }
    //     }
    // }

}

