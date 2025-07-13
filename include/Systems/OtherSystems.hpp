
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>
#include "../lib/physics.hpp"

extern p2d::Physics physics;
extern float deltaTime;
extern int width, height;
extern GLfloat toplefttile[2];
extern GLfloat offsetValue[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;
extern bool windowResized;

void updateOther(entt::registry &registry) {

    auto idEntities = registry.view<Id, Position>();
    for (auto entity : idEntities) {
        auto &id = idEntities.get<Id>(entity);
        if (id.name == "player_cursor") {
            auto &pos = idEntities.get<Position>(entity);
            // Get the Cursor from the _player entity
            if (registry.all_of<Cursor>(_player)) {
                auto &cursor = registry.get<Cursor>(_player);
                auto &shape = registry.get<Shape>(entity);
                pos.x = cursor.position.sx - shape.size.x/2;
                pos.y = cursor.position.sy - shape.size.y/2;
            }
        }
    }


    
    auto flagged = registry.view<Flag>();
    for(auto e : flagged) {
        auto& flag = registry.get<Flag>(e);
        if(flag.name == "destroy") {
            // If entity has PhysicsBodyRect, remove its body from physics
            if (registry.all_of<PhysicsBodyRect>(e)) {
                auto& rect = registry.get<PhysicsBodyRect>(e);
                if (rect.body) {
                    physics.remove(rect.body);
                }
            }
            registry.destroy(e);
        }
    }

}
