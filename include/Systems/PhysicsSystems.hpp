// Start of Selection

#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include "../lib/physics.hpp"


extern entt::entity _player;
extern p2d::Physics physics;
extern float deltaTime;

void onCollision(p2d::CollisionInfo& info) {

    auto entityA = info.bodyA->m_entity;
    auto entityB = info.bodyB->m_entity;

    if (registry.all_of<InteriorPortal>(entityA) || registry.all_of<InteriorPortal>(entityB)) {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Collision detected involving an InteriorPortal at positions (%.2f, %.2f) and (%.2f, %.2f)",
                 info.bodyA->getPosition().x, info.bodyA->getPosition().y,
                 info.bodyB->getPosition().x, info.bodyB->getPosition().y);
        emlog(buffer, LogLevel::ERROR);
    }
}


void updatePhysics(entt::registry &registry) {
    
    physics.setOnCollision(onCollision);


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
                // rect.body = new p2d::CircleBody(shape.size.x / 4, pos.x + shape.size.x / 4, pos.y + shape.size.y / 4, mass, restitution, isstatic);
                physics.add(rect.body);
            }
            else if (!rect.body->isStatic()) {
                p2d::Vec2f currentPos = rect.body->getPosition();
                pos.x = currentPos.x - shape.size.x/2;
                pos.y = currentPos.y - shape.size.y/2;
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
    // const auto& collisions = physics.getCollisions();
    // for (const auto& collision : collisions) {
    //     // printf("Collision between Body A and Body B:\n");
    //     // printf("Collision Point: (%f, %f)\n", collision.collisionPoint.x, collision.collisionPoint.y);
    //     // printf("Normal: (%f, %f)\n", collision.normal.x, collision.normal.y);
    //     // printf("Penetration Depth: %f\n", collision.penetrationDepth);
        
    // }

}
