// Start of Selection

#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include "../lib/physics.hpp"
#include "../JSUtils.hpp"

extern entt::entity _player;
extern p2d::Physics physics;
extern float deltaTime;

void processCollisionInfo(p2d::CollisionInfo& info) {
    auto entityA = info.bodyA->m_entity;
    auto entityB = info.bodyB->m_entity;

    // if (registry.all_of<PhysicsBodyRect>(entityA) && registry.all_of<PhysicsBodyRect>(entityB)) {
    //     auto& physBodyA = registry.get<PhysicsBodyRect>(entityA);
    //     auto& physBodyB = registry.get<PhysicsBodyRect>(entityB);
    //     if (physBodyA.body->ignore || physBodyB.body->ignore) return;
    // }

    entt::entity door = entt::null;
    entt::entity nonPortalEntity = entt::null;
    entt::entity teleporter = entt::null;
    entt::entity teleportable = entt::null;

    bool entityAHasPortal = registry.all_of<InteriorPortal>(entityA);
    bool entityBHasPortal = registry.all_of<InteriorPortal>(entityB);
    bool entityAHasTeleport = registry.all_of<Teleport>(entityA);
    bool entityBHasTeleport = registry.all_of<Teleport>(entityB);
    bool entityAIsTeleportable = registry.all_of<Teleportable>(entityA);
    bool entityBIsTeleportable = registry.all_of<Teleportable>(entityB);

    if (entityAHasPortal && !entityBHasPortal) {
        door = entityA;
        nonPortalEntity = entityB;
    } else if (!entityAHasPortal && entityBHasPortal) {
        door = entityB;
        nonPortalEntity = entityA;
    }

    if (entityAHasTeleport && entityBIsTeleportable) {
        teleporter = entityA;
        teleportable = entityB;

    } else if (entityBHasTeleport && entityAIsTeleportable) {
        teleporter = entityB;
        teleportable = entityA;
    }

    if (door != entt::null) {
        // Log if the non-portal entity has Player component
        if (nonPortalEntity != entt::null) {
    
            if(!registry.all_of<OnInteriorPortal>(nonPortalEntity)) {
                
                // Add or update the Inside component for the non-interiorportal entity
                auto doorIP = registry.get<InteriorPortal>(door);
                if (registry.all_of<Inside>(nonPortalEntity)) {
                    auto& inside = registry.get<Inside>(nonPortalEntity);
                    if(inside.interior == doorIP.A) inside.interior = doorIP.B;
                    else inside.interior = doorIP.A;
                    auto &body = registry.get<PhysicsBodyRect>(door).body;
                        _js__play_tone("C5", "1n", -20.0f, "sparkle1.mp3");

                } 

                registry.emplace<OnInteriorPortal>(nonPortalEntity, OnInteriorPortal{door});
            }
        }
    }

    // if (teleporter != entt::null && teleportable != entt::null) {
    //     auto& teleport = registry.get<Teleport>(teleporter);
    //     auto& position = registry.get<Position>(teleportable);
    //     auto &tpl = registry.get<Teleportable>(teleportable);

    //     if(!tpl.refresh) {
    //         // Teleport the entity to the destination
    //         tpl.refresh = true;
    //     }
    //     else {
    //         int currentTime = static_cast<int>(emscripten_get_now());
    //         if (currentTime > tpl.timer + 5000) { // Check if more than 5 seconds have passed since last teleport
    //             if(tpl.refresh) {
    //                 tpl.refresh = false;
    //             }
    //         }
    //     }
    //     position.x = teleport.destination.x;
    //     position.y = teleport.destination.y;
    //     position.z = teleport.destination.z;
    // }
}

void onCollision(p2d::CollisionInfo& info) {
    processCollisionInfo(info);
}

void updatePhysics(entt::registry &registry) {
    
    // physics.setOnCollision(onCollision);


    auto view = registry.view<PhysicsBodyRect, Position>();
    for(auto entity : view) {
        auto &rect = view.get<PhysicsBodyRect>(entity);
        auto &pos = view.get<Position>(entity);
        
        // Handle Collidable and Shape components
        if(registry.all_of<Collidable, Shape>(entity)) {
            auto &shape = registry.get<Shape>(entity);

            if(!rect.added) {
                rect.added = true;
                // Add mass and restitution values to avoid NaN
                float mass = 10;
                float restitution = 1;
                bool isstatic = true;
                if(registry.all_of<Movement>(entity)) {
                    auto movement = registry.get<Movement>(entity);
                    mass = movement.mass;
                    restitution = movement.restitution;
                    isstatic = false;
                }
                rect.body = new p2d::RectangleBody(
                    shape.size.x, shape.size.y, 
                    pos.x + shape.size.x/2, pos.y + shape.size.y/2, 
                    mass, restitution, isstatic, entity
                );

                if(registry.any_of<InteriorPortal, Interior>(entity)) {
                    rect.body->ignore = true;
                }

                physics.add(rect.body);
            }
            else if (!rect.body->isStatic()) {
                
                bool update = true;
                if(update) {
                    p2d::Vec2f currentPos = rect.body->getPosition();
                    pos.x = currentPos.x - shape.size.x/2;
                    pos.y = currentPos.y - shape.size.y/2;
                }
            }
        }

        // Handle Keys, Movement, and InView components
        if(registry.all_of<Keys, Movement, InView>(entity)) {
            auto &keys = registry.get<Keys>(entity).keys;
            auto &movement = registry.get<Movement>(entity);

            Vector3f input{
                static_cast<float>(keys[SDLK_d]) - static_cast<float>(keys[SDLK_a]),
                static_cast<float>(keys[SDLK_s]) - static_cast<float>(keys[SDLK_w]),
                0.0f
            };
            float length = std::sqrt(input.x * input.x + input.y * input.y);
            if (length != 0) {
                float fx = (input.x / length) * movement.speed;
                float fy = (input.y / length) * movement.speed;
                rect.body->applyForce({fx, fy});
                // rect.body->applyThetaDotDot(1.0);
            }
        }
    }    

    physics.update(deltaTime);

    // // After update, you can also access all collisions that occurred
    const auto& collisions = physics.getCollisions();
    for(auto info : collisions) {
        // char collisionMsg[100];
        // sprintf(collisionMsg, "Collision at: %f, %f", info.collisionPoint.x, info.collisionPoint.y);
        // emlog(collisionMsg);
        processCollisionInfo(info);
    }

    // Create a set of entities that are currently colliding
    std::unordered_set<entt::entity> collidingEntities;
    
    for (const auto& collision : collisions) {
        // Get the entities associated with the colliding bodies
        auto* bodyA = collision.bodyA;
        auto* bodyB = collision.bodyB;
        
        // Find entities with these physics bodies
        auto view = registry.view<PhysicsBodyRect>();
        for (auto entity : view) {
            auto& rect = view.get<PhysicsBodyRect>(entity);
            if (rect.body == bodyA || rect.body == bodyB) {
                collidingEntities.insert(entity);
            }
        }
    }
    
    // Check for entities with OnInteriorPortal that are no longer colliding
    auto portalView = registry.view<OnInteriorPortal>();
    for (auto entity : portalView) {
        // If entity has OnInteriorPortal but is not in the current collisions
        if (collidingEntities.find(entity) == collidingEntities.end()) {
            registry.remove<OnInteriorPortal>(entity);
        }
    }

}
