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
        auto &shape = movement_entities.get<Shape>(entity);
        bool isPlayer = registry.all_of<Player>(entity);

        if (isPlayer)
        {
            float deltaX = (keys[SDLK_a] - keys[SDLK_d]);
            float deltaY = (keys[SDLK_w] - keys[SDLK_s]);

            movement.acceleration.x -= deltaX * movement.speed * moveSpeed;
            movement.acceleration.y -= deltaY * movement.speed * moveSpeed;
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

    // Check if mouse is colliding with any Debug entities. use the screen position
    auto debug_entities = registry.view<Position, Shape, Debug, Hoverable, Interactable, Color>();
    for (auto &entity : debug_entities)
    {
        auto &position = debug_entities.get<Position>(entity);
        auto &debug = debug_entities.get<Debug>(entity);

        // Convert cursor position to normalized screen coordinates
        float normalizedCursorX = (2.0f * cursorPos[0] / width) - 1.0f;
        float normalizedCursorY = 1.0f - (2.0f * cursorPos[1] / height);

        // Convert entity position to screen coordinates
        float screenEntityX = (position.sx);
        float screenEntityY = (position.sy);

        // Check for collision
        bool is_hovered = registry.all_of<Hovered>(entity);

        auto &color = debug_entities.get<Color>(entity);

        auto &shape = debug_entities.get<Shape>(entity);
        float radiusX = shape.scaled_size.x;
        float radiusY = shape.scaled_size.y;
        if (fabs(normalizedCursorX - screenEntityX) < radiusX && fabs(normalizedCursorY - screenEntityY) < radiusY)
        {
            // Handle collision with debug entity
            if (!is_hovered)
            {
                registry.emplace<Hovered>(entity);
            }

            if (keys[SDL_BUTTON_LEFT])
            {
                if (!registry.all_of<Interacted>(entity))
                {
                    registry.emplace<Interacted>(entity);
                }
                // Change color to blue when interacted and hovered
                color.r = 0;
                color.g = 0;
                color.b = 255;

                // update entity position based on globalCursorPos
                position.x = globalCursorPos[0];
                position.y = globalCursorPos[1];
            }
            else
            {
                // Change color to red when only hovered
                color.r = 255;
                color.g = 0;
                color.b = 0;
            }
        }
        else if (is_hovered)
        {
            registry.remove<Hovered>(entity);
            if (registry.all_of<Interacted>(entity))
            {
                registry.remove<Interacted>(entity);
            }

            // Reset color to default
            color.r = debug.defaultColor.r;
            color.g = debug.defaultColor.g;
            color.b = debug.defaultColor.b;
        }
    }
}

void updateEntities(entt::registry &registry) {

    Position playerPos = registry.get<Position>(_player);

    auto entities = registry.view<Position, Shape, Validation>();
    for (auto &entity : entities)
    {

        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);
        auto &validation = entities.get<Validation>(entity);

        if (validation.state == 2)
        {
            continue;
        }

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

            bool isDebug = registry.all_of<Debug>(entity);
            bool isTeleport = registry.all_of<Teleport>(entity);

            if (isDebug || isTeleport)
            {
                validation.state = 1;
            }

            validation.state = 1;

            // // check if entity is visible
            if (validation.state == 1 && !has_visible)
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