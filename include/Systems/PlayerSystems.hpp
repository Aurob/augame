
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>

extern float deltaTime;
extern GameState gameState;
extern entt::entity _player;
extern bool windowResized;

void updateCamera(entt::registry &registry);
void updatePlayer(entt::registry &registry);

void updateCamera(entt::registry &registry) {
    // Find the entity with Camera component
    auto cameraView = registry.view<Camera, Position, Shape>();
    if(cameraView.begin() == cameraView.end()) return;
    
    auto cameraEntity = cameraView.front();
    auto& camera = registry.get<Camera>(cameraEntity);
    Position &cameraPos = registry.get<Position>(cameraEntity);
    Shape &cameraShape = registry.get<Shape>(cameraEntity);
    
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
    
    // Find camera for cursor calculations
    auto cameraView = registry.view<Camera, Position>();
    if(cameraView.begin() != cameraView.end()) {
        auto& camera = registry.get<Camera>(cameraView.front());
        
        // Calculate cursor normalized shader coordinates (sx, sy)
        playerCursorPos.position.sx = playerPos.x + ((playerCursorPos.position.x - gameState.width / 2) * camera.defaultGSV / camera.gridSpacing) + playerShape.size.x/2;
        playerCursorPos.position.sy = playerPos.y + ((playerCursorPos.position.y - gameState.height / 2) * camera.defaultGSV / camera.gridSpacing) + playerShape.size.x/2;
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

