#pragma once

#include "Systems/OtherSystems.hpp"
#include "Systems/PlayerSystems.hpp"
#include "Systems/ActionSystems.hpp"
#include "Systems/ViewSystems.hpp"
#include "Systems/PhysicsSystems.hpp"
#include "Systems/TextureSystems.hpp"
#include "../include/JSUtils.hpp"

    
extern float deltaTime;
void updateFrame()
{
    
    updateActions(registry);
    updatePhysics(registry);
    updatePlayer(registry);
    updateShapes(registry);
    updatePositions(registry);   
    updateInteractions(registry);
    updateAnimations(registry);
    updateOther(registry);


    // Update JS with the player's position using a view to get Player
    auto playerView = registry.view<Player, Position>();
    for (auto entity : playerView) {
        Position &playerPos = playerView.get<Position>(entity);
        _js__update_user_position(playerPos.x, playerPos.y);
        break; // Only update for the first player found
    }
}
