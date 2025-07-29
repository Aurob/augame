
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>

extern float deltaTime;
extern GameState gameState;
extern entt::entity _player;
extern bool windowResized;

void updatePositions(entt::registry &registry)
{
    // Find the entity with Camera component
    auto cameraView = registry.view<Camera, Position, Shape>();
    if(cameraView.begin() == cameraView.end()) return;
    
    auto cameraEntity = cameraView.front();
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

        // Skip if entity has Id.name == "player_cursor"
        if (registry.all_of<Id>(entity)) {
            auto &id = registry.get<Id>(entity);
            if (id.name == "player_cursor") {
                isWithinBounds = false;
            }
        }

        if (isWithinBounds)
        {
            bool entityIsPortal = registry.all_of<InteriorPortal>(entity);

            if(registry.all_of<Inside>(entity)) { 
                auto _interior = registry.get<Inside>(entity).interior;
                // if(_interior != playerInterior) {
                //             isVisible = false;
                //             isInView = false;
                // }
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
    
    // Find the entity with Camera component
    auto cameraView = registry.view<Camera>();
    if(cameraView.begin() == cameraView.end()) return;
    
    auto& camera = registry.get<Camera>(cameraView.front());
    
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