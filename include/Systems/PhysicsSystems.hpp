// Start of Selection

#pragma once

#include "../JSUtils.hpp"
#include "../lib/entt.hpp"
#include "../lib/physics.hpp"
#include "../structs.hpp"
#include "../SceneManager.hpp"

extern entt::entity _player;
extern p2d::Physics physics;
extern float deltaTime;
extern SceneManager sceneManager;

void processCollisionInfo(p2d::CollisionInfo& info, entt::registry& registry)
{
    auto entityA = info.bodyA->m_entity;
    auto entityB = info.bodyB->m_entity;

    entt::entity door = entt::null;
    entt::entity nonPortalEntity = entt::null;
    bool entityAHasPortal = registry.all_of<InteriorPortal>(entityA);
    bool entityBHasPortal = registry.all_of<InteriorPortal>(entityB);

    if (entityAHasPortal && !entityBHasPortal) {
        door = entityA;
        nonPortalEntity = entityB;
    } else if (!entityAHasPortal && entityBHasPortal) {
        door = entityB;
        nonPortalEntity = entityA;
    }

    if (door != entt::null) {
        auto doorIP = registry.get<InteriorPortal>(door);

        // Check if the door has a key and if the colliding entity is the key entity
        if (doorIP.key != entt::null && (entityA == doorIP.key || entityB == doorIP.key)) {
            // Unlock the door
            doorIP.locked = false;
            registry.replace<InteriorPortal>(door, doorIP);

            // Flag the key entity (whichever it is) for destruction
            if (entityA == doorIP.key) {
                registry.emplace_or_replace<Flag>(entityA, Flag{"destroy"});
            } else if (entityB == doorIP.key) {
                registry.emplace_or_replace<Flag>(entityB, Flag{"destroy"});
            }
        }

        // If still locked after possible unlock, return
        if (doorIP.locked) return;

        // Log if the non-portal entity has Player component
        if (nonPortalEntity != entt::null) {
            // First, check if the non-door entity is inside, but not inside either A or B; if so, ignore
            bool isInside = registry.all_of<Inside>(nonPortalEntity);
            int currentInterior = -1;
            if (isInside) {
                auto& inside = registry.get<Inside>(nonPortalEntity);
                currentInterior = static_cast<int>(inside.interior);
                if (inside.interior != doorIP.A && inside.interior != doorIP.B) {
                    return;
                }
            }

            // If the entity is outside (not Inside), only allow transition if A or B is -1
            if (!isInside) {
                if (static_cast<int>(doorIP.A) != -1 && static_cast<int>(doorIP.B) != -1) {
                    // Both sides are interiors, do not allow outside entity to enter
                    return;
                }
            }

            if (!registry.all_of<OnInteriorPortal>(nonPortalEntity)) {

                // Add or update the Inside component for the non-interiorportal entity
                if (isInside) {
                    auto& inside = registry.get<Inside>(nonPortalEntity);
                    if (inside.interior == doorIP.A)
                        inside.interior = doorIP.B;
                    else
                        inside.interior = doorIP.A;

                    // If the new interior is -1, remove Inside from the entity
                    if (static_cast<int>(inside.interior) == -1) {
                        registry.remove<Inside>(nonPortalEntity);
                    }
                } else {
                    // If not inside, they are outside, use any value < 0
                    if (static_cast<int>(doorIP.A) == -1) {
                        // Do not add Inside if the destination is -1
                        if (registry.all_of<Inside>(nonPortalEntity)) {
                            registry.remove<Inside>(nonPortalEntity);
                        }
                    } else {
                        registry.emplace_or_replace<Inside>(nonPortalEntity, Inside { doorIP.A });
                    }
                }

                registry.emplace<OnInteriorPortal>(nonPortalEntity, OnInteriorPortal { door, 0 });
            }
        }
    }

}

void onCollision(p2d::CollisionInfo& info) { processCollisionInfo(info, sceneManager.getCurrentRegistry()); }

void updatePhysics(entt::registry& registry)
{ 

    auto view = registry.view<PhysicsBodyRect, Position>();
    for (auto entity : view) {
        auto& rect = view.get<PhysicsBodyRect>(entity);
        auto& pos = view.get<Position>(entity);
        // Handle Collidable and Shape components
        if (registry.all_of<Shape>(entity)) {
            auto& shape = registry.get<Shape>(entity);

            if (!rect.added) {
                rect.added = true;
                // Add mass and restitution values to avoid NaN
                float mass = 10;
                float restitution = 1;
                bool isstatic = true;
                if (registry.all_of<Movement>(entity)) {
                    auto movement = registry.get<Movement>(entity);
                    mass = movement.mass;
                    restitution = movement.restitution;
                    isstatic = false;
                }
                rect.body = new p2d::RectangleBody(shape.size.x, shape.size.y, pos.x + shape.size.x / 2, pos.y + shape.size.y / 2, mass, restitution, isstatic, entity);
                if (!registry.any_of<Collidable>(entity)) {
                    rect.body->ignore = true;
                }

                physics.add(rect.body);

            } 
			else if (!rect.body->isStatic()) {

                p2d::Vec2f currentPos = rect.body->getPosition();
                pos.x = currentPos.x - shape.size.x / 2;
                pos.y = currentPos.y - shape.size.y / 2;
            }
        }

        // Handle Keys, Movement, and InView components
        if (registry.all_of<Keys, Movement, InView>(entity)) {
            auto& keys = registry.get<Keys>(entity).keys;
            auto& movement = registry.get<Movement>(entity);
            Vector3f input { static_cast<float>(keys[SDLK_d]) - static_cast<float>(keys[SDLK_a]), static_cast<float>(keys[SDLK_s]) - static_cast<float>(keys[SDLK_w]), 0.0f };
            float length = std::sqrt(input.x * input.x + input.y * input.y);
            if (length != 0) {
                float fx = (input.x / length) * movement.speed;
                float fy = (input.y / length) * movement.speed;

                rect.body->applyForce({ fx, fy });
                rect.body->applyThetaDotDot(1.0);
            } else {
                // Apply friction when no input
                auto vel = rect.body->getVelocity();
                vel.x *= 0.9f; // Damping factor
                vel.y *= 0.9f;
                rect.body->setVelocity(vel);
            }
        }
    }

    float clampedDeltaTime = std::min(deltaTime, 0.1f); // Max 100ms per 
    physics.update(clampedDeltaTime);

    // After update, you can also access all collisions that occurred
    const auto& collisions = physics.getCollisions();

    for (auto info : collisions) {
        processCollisionInfo(info, registry);
    }


    // Map from entity to the set of entities it is colliding with
    std::unordered_map<entt::entity, std::vector<entt::entity>> entityCollisions;

    // Build a mapping of which entities are colliding with which
    auto physView = registry.view<PhysicsBodyRect>();
    for (const auto& collision : collisions) {
        auto* bodyA = collision.bodyA;
        auto* bodyB = collision.bodyB;

        entt::entity entityA = entt::null;
        entt::entity entityB = entt::null;

        for (auto entity : physView) {
            auto& rect = physView.get<PhysicsBodyRect>(entity);
            if (rect.body == bodyA) entityA = entity;
            if (rect.body == bodyB) entityB = entity;
        }

        if (entityA != entt::null && entityB != entt::null) {
            entityCollisions[entityA].push_back(entityB);
            entityCollisions[entityB].push_back(entityA);
        }
    }

    // For each Inside entity, check if it is outside its Interior bounds and push
    // it back in
    auto insideView = registry.view<Inside, PhysicsBodyRect>(entt::exclude<InteriorPortal>);
    for (auto entity : insideView) {
        auto& inside = registry.get<Inside>(entity);
        auto interiorEntity = inside.interior;

        if (interiorEntity == entt::null || !registry.valid(interiorEntity) || !registry.all_of<PhysicsBodyRect, Shape>(interiorEntity))
            continue;
		
        auto& rectA = registry.get<PhysicsBodyRect>(entity);
        auto& rectB = registry.get<PhysicsBodyRect>(interiorEntity);
        auto& shapeB = registry.get<Shape>(interiorEntity);

        // Get bounds of the interior
        float left = rectB.body->getPosition().x - shapeB.size.x / 2;
        float right = rectB.body->getPosition().x + shapeB.size.x / 2;
        float top = rectB.body->getPosition().y - shapeB.size.y / 2;
        float bottom = rectB.body->getPosition().y + shapeB.size.y / 2;

        // Get bounds of the entity
        auto& shapeA = registry.get<Shape>(entity);
        float ax = rectA.body->getPosition().x;
        float ay = rectA.body->getPosition().y;
        float halfWidthA = shapeA.size.x / 2;
        float halfHeightA = shapeA.size.y / 2;

        float newX = ax;
        float newY = ay;
        bool outOfBounds = false;

        // Clamp entity's position to stay within the interior bounds
        if (ax - halfWidthA < left) {
            newX = left + halfWidthA;
            outOfBounds = true;
        }
        if (ax + halfWidthA > right) {
            newX = right - halfWidthA;
            outOfBounds = true;
        }
        if (ay - halfHeightA < top) {
            newY = top + halfHeightA;
            outOfBounds = true;
        }
        if (ay + halfHeightA > bottom) {
            newY = bottom - halfHeightA;
            outOfBounds = true;
        }

        if (outOfBounds) {
            // Move the entity back inside
            rectA.body->setPosition({ newX, newY });
            rectA.body->setTempPosition({ newX, newY });
            // Optionally, zero or invert velocity to simulate a bounce
            auto vel = rectA.body->getVelocity();
            if (ax - halfWidthA < left || ax + halfWidthA > right)
                vel.x = -vel.x * 0.5f; // bounce with damping
            if (ay - halfHeightA < top || ay + halfHeightA > bottom)
                vel.y = -vel.y * 0.5f;
            
            rectA.body->setVelocity(vel);
            rectA.body->setTempVelocity(vel);
        }
    }

    // For each entity with OnInteriorPortal, check if it is colliding with any InteriorPortal
    auto portalView = registry.view<OnInteriorPortal>();
    for (auto entity : portalView) {
        bool collidesWithPortal = false;
        auto &oip = registry.get<OnInteriorPortal>(entity);
        oip.timeout += 1;
        auto it = entityCollisions.find(entity);
        if (it != entityCollisions.end()) {
            for (auto other : it->second) {
                if (registry.all_of<InteriorPortal>(other)) {
                    collidesWithPortal = true;
                    break;
                }
            }
        }
        // If not colliding with any InteriorPortal, remove OnInteriorPortal
        if (!collidesWithPortal) {
            if(oip.timeout > 10) {
                registry.remove<OnInteriorPortal>(entity);
            }
        }
    }

    // Apply terrain bounds as physical barriers for entities outside interiors
    auto& metadata = sceneManager.getCurrentMetadata();
    if (!metadata.terrain_bounds.empty() && metadata.terrain_bounds.size() == 4) {
        float minX = metadata.terrain_bounds[0];
        float minY = metadata.terrain_bounds[1];
        float maxX = metadata.terrain_bounds[2];
        float maxY = metadata.terrain_bounds[3];

        // Only apply bounds if they're not all zero (which means no bounds)
        if (minX != 0.0f || minY != 0.0f || maxX != 0.0f || maxY != 0.0f) {
            auto terrainBoundsView = registry.view<PhysicsBodyRect, Shape>(entt::exclude<Inside>);
            for (auto entity : terrainBoundsView) {
                auto& rect = registry.get<PhysicsBodyRect>(entity);
                auto& shape = registry.get<Shape>(entity);

                // Get current position
                float ax = rect.body->getPosition().x;
                float ay = rect.body->getPosition().y;
                float halfWidthA = shape.size.x / 2;
                float halfHeightA = shape.size.y / 2;

                float newX = ax;
                float newY = ay;
                bool outOfBounds = false;

                // Clamp entity's position to stay within terrain bounds
                if (ax - halfWidthA < minX) {
                    newX = minX + halfWidthA;
                    outOfBounds = true;
                }
                if (ax + halfWidthA > maxX) {
                    newX = maxX - halfWidthA;
                    outOfBounds = true;
                }
                if (ay - halfHeightA < minY) {
                    newY = minY + halfHeightA;
                    outOfBounds = true;
                }
                if (ay + halfHeightA > maxY) {
                    newY = maxY - halfHeightA;
                    outOfBounds = true;
                }

                if (outOfBounds) {
                    // Move the entity back inside bounds
                    rect.body->setPosition({ newX, newY });
                    rect.body->setTempPosition({ newX, newY });
                    // Bounce with damping
                    auto vel = rect.body->getVelocity();
                    if (ax - halfWidthA < minX || ax + halfWidthA > maxX)
                        vel.x = 0.0f;
                    if (ay - halfHeightA < minY || ay + halfHeightA > maxY)
                        vel.y = 0.0f;

                    rect.body->setVelocity(vel);
                    rect.body->setTempVelocity(vel);
                }
            }
        }
    }
}
