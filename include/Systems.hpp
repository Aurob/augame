#pragma once

#include "Systems/OtherSystems.hpp"
#include "Systems/PlayerSystems.hpp"
#include "Systems/ActionSystems.hpp"
#include "Systems/ViewSystems.hpp"
#include "Systems/PhysicsSystems.hpp"

    
extern float deltaTime;

void updateFrame()
{
    updateOther(registry);
    updateActions(registry);
    updatePhysics(registry);
    updatePlayer(registry);
    updateInteractions(registry);
    updateShapes(registry);
    updatePositions(registry);
}
