#pragma once

#include "entt.hpp"
#include "structs.hpp"
#include <SDL2/SDL.h>
#include "Utils.hpp"

extern unordered_map<int, bool> keys;
extern float deltaTime;
extern int width, height;
extern GLfloat cursorPos[2];
extern GLfloat globalCursorPos[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;
extern float moveSpeed;
extern float defaultMoveSpeed;


void updateMovement(entt::registry &registry)
{
    auto movement_entities = registry.view<Visible, Movement, Position, Shape>();

    for (auto entity : movement_entities)
    {
        auto &movement = movement_entities.get<Movement>(entity);
        auto &position = movement_entities.get<Position>(entity);
        bool isPlayer = registry.all_of<Player>(entity);

        if (isPlayer)
        {
            float deltaX = keys[SDLK_a] - keys[SDLK_d];
            float deltaY = keys[SDLK_w] - keys[SDLK_s];

            movement.acceleration.x -= deltaX * movement.speed * moveSpeed;
            movement.acceleration.y -= deltaY * movement.speed * moveSpeed;
        }

        if (movement.acceleration.x != 0 || movement.acceleration.y != 0)
        {
            movement.velocity.x = movement.acceleration.x * deltaTime;
            movement.velocity.y = movement.acceleration.y * deltaTime;
            movement.acceleration.x = 0;
            movement.acceleration.y = 0;
        }

        if (movement.velocity.x != 0 || movement.velocity.y != 0)
        {
            movement.velocity.x -= movement.velocity.x * movement.friction * deltaTime;
            movement.velocity.y -= movement.velocity.y * movement.friction * deltaTime;
        }

        if (movement.max_speed > 0)
        {
            movement.velocity.x = std::clamp(movement.velocity.x, -movement.max_speed, movement.max_speed);
            movement.velocity.y = std::clamp(movement.velocity.y, -movement.max_speed, movement.max_speed);
        }

        position.x += movement.velocity.x;
        position.y += movement.velocity.y;
    }
}

void updateCollisions(entt::registry &registry)
{
    // Sort so that the player is checked first
    registry.sort<Collidable>([&](const entt::entity lhs, const entt::entity rhs)
                              { return registry.all_of<Player>(lhs) && !registry.all_of<Player>(rhs); });

    auto collidables = registry.view<Collidable, Position, Shape, Visible>();

    for (auto entity : collidables)
    {
        if (!registry.all_of<Movement>(entity))
            continue;

        bool colliding = false;
        std::vector<entt::entity> _collidables;
        std::vector<Vector2f> overlaps;

        Shape &entityShape = registry.get<Shape>(entity);
        Position &entityPosition = registry.get<Position>(entity);

        for (auto _entity : collidables)
        {
            if (_entity == entity)
                continue; // skip self

            Position &_entityPosition = registry.get<Position>(_entity);
            Shape &_entityShape = registry.get<Shape>(_entity);

            Vector2f overlap = positionsCollide(entityPosition, entityShape, _entityPosition, _entityShape);
            if (overlap.x != 0 || overlap.y != 0)
            {
                colliding = true;
                _collidables.push_back(_entity);
                overlaps.push_back(overlap);
            }
        }

        if (colliding)
        {
            Movement &entityMovement = registry.get<Movement>(entity);
            float overlapx = 0, overlapy = 0;
            float m1 = entityMovement.mass;
            float xv1 = entityMovement.velocity.x;
            float yv1 = entityMovement.velocity.y;

            if (!registry.all_of<Colliding>(entity))
            {
                registry.emplace<Colliding>(entity);
            }

            for (auto _entity : _collidables)
            {
                if (registry.all_of<Movement>(_entity) && registry.all_of<Moveable>(_entity))
                {
                    Movement &_entityMovement = registry.get<Movement>(_entity);
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

            for (const auto &overlap : overlaps)
            {
                overlapx += overlap.x;
                overlapy += overlap.y;
            }
            entityPosition.x += overlapx;
            entityPosition.y += overlapy;
        }
        else if (registry.all_of<Colliding>(entity))
        {
            registry.remove<Colliding>(entity);
        }
    }
}

void updateTeleporters(entt::registry &registry)
{
    // do collision checks for teleport entities
    auto collidable_entities = registry.view<Position, Shape, Collidable, Visible>();
    for (auto &entity : collidable_entities)
    {
        auto &position = collidable_entities.get<Position>(entity);
        auto &shape = collidable_entities.get<Shape>(entity);

        auto teleport_entities = registry.view<Position, Shape, Teleport, Visible>();
        for (auto &tentity : teleport_entities)
        {
            auto &tposition = teleport_entities.get<Position>(tentity);
            auto &tshape = teleport_entities.get<Shape>(tentity);
            auto &teleport = teleport_entities.get<Teleport>(tentity);

            Vector2f collides = positionsCollide(position, shape, tposition, tshape);
            if (collides.x != 0 || collides.y != 0)
            {
                position.x = teleport.destination.x;
                position.y = teleport.destination.y;

                if(teleport.reverse) {
                    if(registry.all_of<Movement>(entity)) {
                        auto &movement = registry.get<Movement>(entity);
                        movement.velocity.x = 0;
                        movement.velocity.y *= -1;
                    }
                }
            }
        }
    }
}

void updateInteractions(entt::registry &registry)
{
    // auto debug_entities = registry.view<Visible, Interactable>();
}

void updatePositions(entt::registry &registry) {

    Position playerPos = registry.get<Position>(_player);

    auto entities = registry.view<Position, Shape>();
    for (auto &entity : entities)
    {

        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);
        bool has_visible = registry.all_of<Visible>(entity);

        float gridOffsetX = playerPos.x - (width / 2);
        float gridOffsetY = playerPos.y - (height / 2);

        // Adjust entity coordinates relative to the grid offset
        float entityGridX = position.x - gridOffsetX;
        float entityGridY = position.y - gridOffsetY;
        
        // Check whether any part of the entity falls within the screen boundaries
        if (entityGridX + shape.size.x > 0 && entityGridX < width &&
            entityGridY + shape.size.y > 0 && entityGridY < height)
        {

            float posX = (playerPos.x - position.x) * gridSpacingValue + width/2;
            float posY = (playerPos.y - position.y) * gridSpacingValue + height/2;

            position.sx = (2 * posX / width - 1) / defaultGSV;
            position.sy = (2 * posY / height - 1) / defaultGSV;

            position.sx -= shape.scaled_size.x * (.999);
            position.sy -= shape.scaled_size.y * (.999);

            // // check if entity is visible
            if (!has_visible)
            {
                registry.emplace<Visible>(entity);
            }
        }
        else if (has_visible)
        {
            registry.remove<Visible>(entity);
        }
    }
}

void updateShapes(entt::registry &registry) {

    // update shapes
    auto entities = registry.view<Shape>();
    for(auto& entity : entities) {
        auto &shape = entities.get<Shape>(entity);
        shape.scaled_size.x = (shape.size.x / defaultGSV) * gridSpacingValue / width;
        shape.scaled_size.y = (shape.size.y / defaultGSV) * gridSpacingValue / height;
    }
}