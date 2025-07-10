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
    
    updatePlayer(registry); // Handle player input
    updateActions(registry); // Process player/AI actions
    updateOther(registry); // Update AI/NPCs
    updatePhysics(registry); // Apply physics, resolve collisions
    updatePositions(registry); // Update positions based on physics/actions
    updateShapes(registry); // Update shapes for collision/interactions
    updateInteractions(registry); // Handle entity interactions
    updateAnimations(registry); // Update animations


    // Update JS with the player's position using a view to get Player
    auto playerView = registry.view<Player, Position>();
    for (auto entity : playerView) {
        Position &playerPos = playerView.get<Position>(entity);
        _js__update_user_position(playerPos.x, playerPos.y);
        break; // Only update for the first player found
    }
}
