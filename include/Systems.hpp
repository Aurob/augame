#pragma once

#include "Systems/OtherSystems.hpp"
#include "Systems/PlayerSystems.hpp"
#include "Systems/ActionSystems.hpp"
#include "Systems/ViewSystems.hpp"
#include "Systems/PhysicsSystems.hpp"
#include "Systems/TextureSystems.hpp"
#include "../include/JSUtils.hpp"
#include "SceneManager.hpp"

    
extern float deltaTime;
extern SceneManager sceneManager;
void updateFrame()
{
    auto& currentRegistry = sceneManager.getCurrentRegistry();
    
    updateActions(currentRegistry);
    
    updatePhysics(currentRegistry);
    
    updatePlayer(currentRegistry);
    
    updateShapes(currentRegistry);
    
    updateInteractions(currentRegistry);
    
    updateAnimations(currentRegistry);
    
    updateOther(currentRegistry);
    
    updatePositions(currentRegistry);
    
    updateCamera(currentRegistry);



    // Update JS with the player's position using a view to get Player
    auto playerView = currentRegistry.view<Player, Position>();
    for (auto entity : playerView) {
        Position &playerPos = playerView.get<Position>(entity);
        _js__update_user_position(playerPos.x, playerPos.y);
        break; // Only update for the first player found
    }
}
