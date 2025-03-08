#pragma once

#include "Systems/CollisionSystems.hpp"
#include "Systems/PlayerSystems.hpp"
#include "Systems/ActionSystems.hpp"
#include "Systems/ViewSystems.hpp"
#include "Systems/PhysicsSystems.hpp"

    
extern float deltaTime;

void updateFrame()
{
    updatePhysics(registry);
    updatePlayer(registry);
    updateInteractions(registry);
    updateShapes(registry);
    updatePositions(registry);
    updateActions(registry);
}
