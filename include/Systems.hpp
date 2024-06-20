#pragma once

#include "entt.hpp"
#include "structs.hpp"
#include <SDL2/SDL.h>
#include "Utils.hpp"

extern unordered_map<int, bool> keys;
extern float deltaTime;

void updateMovement(entt::registry &registry)
{
    auto movement_entities = registry.view<Visible, Movement, Position, Shape>();

    for (auto entity : movement_entities)
    {

        // if (registry.all_of<Player>(entity))
        // {
        auto &movement = movement_entities.get<Movement>(entity);
        auto &position = movement_entities.get<Position>(entity);
        auto &shape = movement_entities.get<Shape>(entity);
        bool isPlayer = registry.all_of<Player>(entity);

        if (isPlayer)
        {
            float deltaX = (keys[SDLK_d] - keys[SDLK_a]);
            float deltaY = (keys[SDLK_w] - keys[SDLK_s]);
            float normFactor = sqrt(deltaX * deltaX + deltaY * deltaY);
            if (normFactor != 0) {
                deltaX /= normFactor;
                deltaY /= normFactor;
            }
            movement.acceleration.x -= deltaX * movement.speed;
            movement.acceleration.y -= deltaY * movement.speed;
            movement.velocity.x = movement.acceleration.x * deltaTime;
            movement.velocity.y = movement.acceleration.y * deltaTime;

            movement.acceleration.x = 0;
            movement.acceleration.y = 0;
        }
        else
        {
            // Velocity updates
            if (movement.acceleration.x != 0 || movement.acceleration.y != 0)
            {
                movement.velocity.x = movement.acceleration.x * deltaTime;
                movement.velocity.y = movement.acceleration.y * deltaTime;

                movement.acceleration.x = 0;
                movement.acceleration.y = 0;
            }
        }

        if (movement.velocity.x != 0 || movement.velocity.y != 0)
        {
            movement.velocity.x -= movement.velocity.x * movement.friction * deltaTime;
            movement.velocity.y -= movement.velocity.y * movement.friction * deltaTime;
        }

        if (movement.max_speed > 0)
        {
            movement.velocity.x = max(-movement.max_speed, min(movement.max_speed, movement.velocity.x));
            movement.velocity.y = max(-movement.max_speed, min(movement.max_speed, movement.velocity.y));
        }

        position.x += movement.velocity.x;
        position.y += movement.velocity.y;

    }
}


void updateCollisions(entt::registry &registry)
{
    // First sort so that the player is checked first
    registry.sort<Collidable>([&](const entt::entity lhs, const entt::entity rhs)
                              { return registry.all_of<Player>(lhs) && !registry.all_of<Player>(rhs); });

    std::vector<std::pair<entt::entity, entt::entity>> collisions;

    auto collidables = registry.view<Collidable, Position, Shape, Visible>();
    #pragma omp parallel for
    for (auto entity : collidables)
    {
        bool isPlayer = registry.all_of<Player>(entity);
        if (!registry.all_of<Movement>(entity))
            continue;

        bool colliding = false;
        bool onEntry = false;
        std::vector<entt::entity> _collidables;
        std::vector<Vector2f> overlaps;
        
        Shape &entityShape = registry.get<Shape>(entity);


        // #pragma omp parallel for
        for (auto _entity : collidables)
        {
            if (_entity == entity)
                continue; // skip self

            Position &entityPosition = registry.get<Position>(entity);
            Position &_entityPosition = registry.get<Position>(_entity);
            Shape &_entityShape = registry.get<Shape>(_entity);

            Vector2f overlap = positionsCollide(entityPosition, entityShape, _entityPosition, _entityShape);
            // If the entity is colliding with another entity
            if (overlap.x != 0 || overlap.y != 0)
            {
                colliding = true;
                _collidables.push_back(_entity);

                overlaps.push_back(overlap);
            }
        }
        
        if (colliding)
        {
            Position &entityPosition = registry.get<Position>(entity);
            Shape &entityShape = registry.get<Shape>(entity);
            Movement &entityMovement = registry.get<Movement>(entity);

            float overlapx = 0;
            float overlapy = 0;
            bool movementCollide = false;
            float m1 = entityMovement.mass;
            float xv1 = entityMovement.velocity.x;
            float yv1 = entityMovement.velocity.y;

            // Iterate over all the components of _entity and print them
            if (!registry.all_of<Colliding>(entity))
            {
                registry.emplace<Colliding>(entity);
            }

            for (auto _entity : _collidables)
            {
                if (registry.all_of<Movement>(_entity))
                {
                    Movement &_entityMovement = registry.get<Movement>(_entity);
                    if (registry.all_of<Moveable>(_entity))
                    {

                        movementCollide = true;

                        // calculate final velocity
                        float m2 = _entityMovement.mass;
                        float xv2 = _entityMovement.velocity.x;
                        float yv2 = _entityMovement.velocity.y;

                        float xv1f = ((m1 - m2) / (m1 + m2)) * xv1 + ((2 * m2) / (m1 + m2)) * xv2;
                        float xv2f = ((2 * m1) / (m1 + m2)) * xv1 + ((m2 - m1) / (m1 + m2)) * xv2;

                        float yv1f = ((m1 - m2) / (m1 + m2)) * yv1 + ((2 * m2) / (m1 + m2)) * yv2;
                        float yv2f = ((2 * m1) / (m1 + m2)) * yv1 + ((m2 - m1) / (m1 + m2)) * yv2;

                        entityMovement.velocity.x = xv1f;
                        entityMovement.velocity.y = yv1f;

                        _entityMovement.velocity.x = xv2f;
                        _entityMovement.velocity.y = yv2f;
                    }
                }

            }

            for (const auto overlap : overlaps)
            {
                overlapx += overlap.x;
                overlapy += overlap.y;
            }
            entityPosition.x += overlapx;
            entityPosition.y += overlapy;
        }
        else
        {
            if (registry.all_of<Colliding>(entity))
            {
                registry.remove<Colliding>(entity);
            }

        }
    }
}