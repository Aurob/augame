
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>

extern float deltaTime;
extern int width, height;
extern GLfloat toplefttile[2];
extern GLfloat offsetValue[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;
extern bool windowResized;

void updatePlayer(entt::registry &registry) {
    
    // Update player-based calculations
    Position &playerPos = registry.get<Position>(_player);
    Shape &playerShape = registry.get<Shape>(_player);
    Cursor &playerCursorPos = registry.get<Cursor>(_player);
    Keys &playerKeys = registry.get<Keys>(_player);
    // Set view offset
    offsetValue[0] = ((fmod(playerPos.x, defaultGSV) * gridSpacingValue) / defaultGSV) - (playerShape.size.x / 2);
    offsetValue[1] = ((fmod(playerPos.y, defaultGSV) * gridSpacingValue) / defaultGSV) - (playerShape.size.y / 2);

    // Set bounds
    toplefttile[0] = static_cast<int>(playerPos.x / defaultGSV) - (width / gridSpacingValue / 2);
    toplefttile[1] = static_cast<int>(playerPos.y / defaultGSV) - (height / gridSpacingValue / 2);

    // Calculate cursor normalized shader coordinates (sx, sy)
    playerCursorPos.position.sx = playerPos.x + ((playerCursorPos.position.x - width / 2) * defaultGSV / gridSpacingValue) + playerShape.size.x/2;
    playerCursorPos.position.sy = playerPos.y + ((playerCursorPos.position.y - height / 2) * defaultGSV / gridSpacingValue) + playerShape.size.x/2;


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

