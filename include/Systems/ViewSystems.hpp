
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

void updatePositions(entt::registry &registry)
{
    Position playerPos = registry.get<Position>(_player);

    bool playerIsInside = registry.all_of<Inside>(_player);
    entt::entity playerInterior;
    if(playerIsInside) {
        auto playerInside = registry.get<Inside>(_player);
        playerInterior = playerInside.interior;
    }

    auto entities = registry.view<Position, Shape>();
    for (auto entity : entities)
    {
        bool logit;
        if(registry.all_of<Id>(entity)) {
            if(registry.get<Id>(entity).name == "door2") logit = true;
        }

        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);

        float posX = (playerPos.x - position.x) * gridSpacingValue + width / 2;
        float posY = (playerPos.y - position.y) * gridSpacingValue + height / 2;
        float posZ = (position.z) * gridSpacingValue + height / 2;
        
        position.sx = (2 * posX / width - 1) / defaultGSV - shape.scaled_size.x * 0.999f;
        position.sy = (2 * posY / height - 1) / defaultGSV - shape.scaled_size.y * 0.999f;
        position.sz = (2 * posZ / height - 1) / defaultGSV - shape.scaled_size.z * 0.999f;


        if(entity == _player) {
            registry.emplace_or_replace<Visible>(entity);
            registry.emplace_or_replace<InView>(entity);
            continue;
        }

        bool isWithinBounds = (
            position.sx + shape.scaled_size.x >= -1 
            && position.sx - shape.scaled_size.x <= 1 
            && (position.sy + shape.scaled_size.y + shape.scaled_size.z*2 + shape.scaled_size.y) >= -1
            && (position.sy - shape.scaled_size.y - shape.scaled_size.z*2 - shape.scaled_size.y) <= 1
        );


        bool isVisible = true;
        bool isInView = true;
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
                    if(!playerIsInside) {
                        bool hideInside = registry.get<Interior>(_interior).hideInside;
                        if(hideInside) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else if(entityIsPortal) {
                        if(_interior != playerInterior) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else {

                        if(entity != playerInterior && _interior != playerInterior) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                }
                else {
                    auto portal = registry.get<InteriorPortal>(entity);
                    if(playerIsInside) {
                        auto playerInside = registry.get<Inside>(_player);

                        if(playerInside.interior != portal.A && playerInside.interior != portal.B) {
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
            else if(playerIsInside) {
                auto playerInside = registry.get<Inside>(_player);
                if(registry.all_of<Interior>(entity)) {
                    if(entity != playerInside.interior) {
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
                physBody.body->ignore = false;
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
    
    // Pre-calculate common scaling factors
    float xScale = gridSpacingValue / (defaultGSV * width);
    float yScale = gridSpacingValue / (defaultGSV * height);
    
    for (auto &entity : entities)
    {
        auto &shape = entities.get<Shape>(entity);
        shape.scaled_size.x = shape.size.x * xScale*1.1;
        shape.scaled_size.y = shape.size.y * yScale*1.1;
    }
}