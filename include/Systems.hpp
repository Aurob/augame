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
extern GLfloat toplefttile[2];


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

        bool hasLink = registry.all_of<Linked>(entity);
        bool isInterior = registry.all_of<Interior>(entity);

        bool colliding = false;
        std::vector<entt::entity> _collidables;
        std::vector<Vector2f> overlaps;

        Shape &entityShape = registry.get<Shape>(entity);
        Position &entityPosition = registry.get<Position>(entity);
        
        for (auto _entity : collidables)
        {
            if (_entity == entity)
                continue; // skip self

            if(registry.all_of<Linked>(_entity)) {
                auto _link = registry.get<Linked>(_entity);
            }

            Position &_entityPosition = registry.get<Position>(_entity);
            Shape &_entityShape = registry.get<Shape>(_entity);
            bool skipCollide = false;
            
            bool invert = false;
            if(registry.all_of<Inside>(entity) && !registry.all_of<OnInteriorPortal>(entity)) {
                auto interior = registry.get<Inside>(entity).interior;
                if(interior == _entity) {
                    invert = true;
                }
            }

            Vector2f overlap = positionsCollide(entityPosition, entityShape, _entityPosition, _entityShape, invert);

            if (overlap.x != 0 || overlap.y != 0)
            {
                if(registry.all_of<Interior>(_entity)) {
                    if(!registry.all_of<InteriorColliding>(entity)) {
                        registry.emplace<InteriorColliding>(entity);
                    }
                    if(registry.all_of<Inside>(entity)) {
                        auto interior = registry.get<Inside>(entity).interior;
                        if(interior == _entity) {
                            if(!registry.all_of<OnInteriorPortal>(entity)) {
                                colliding = true;
                                _collidables.push_back(_entity);
                                overlaps.push_back(overlap);
                            } else {
                                skipCollide = true;
                            }
                        }
                    } else {
                        if(registry.all_of<OnInteriorPortal>(entity)) {
                            auto portal = registry.get<OnInteriorPortal>(entity).portal;
                            auto portalInterior = registry.get<InteriorPortal>(portal).A;
                            if(portalInterior == _entity) {
                                printf("Entering InteriorPortal\n");
                                skipCollide = true;
                            }
                        }
                    }
                }
                
                if(registry.all_of<InteriorPortal>(_entity)) {
                    if(!registry.all_of<OnInteriorPortal>(entity)) {
                        registry.emplace_or_replace<OnInteriorPortal>(entity, OnInteriorPortal{_entity});
                        printf("Entering InteriorPortal\n");

                        if(!registry.all_of<Inside>(entity)) {
                            auto portalInterior = registry.get<InteriorPortal>(_entity).A;
                            registry.emplace<Inside>(entity, Inside{portalInterior});
                        }
                    }
                    skipCollide = true;
                }
                
                if(!skipCollide) {
                    colliding = true;
                    _collidables.push_back(_entity);
                    overlaps.push_back(overlap);
                }
            }
            else {

                if(registry.all_of<Interior>(_entity) && registry.all_of<InteriorColliding>(entity)) {
                    registry.remove<InteriorColliding>(entity);
                }

                if(registry.all_of<InteriorPortal>(_entity) && registry.all_of<OnInteriorPortal>(entity)) {
                    auto portal = registry.get<OnInteriorPortal>(entity);
                    if(portal.portal == _entity) {
                        registry.remove<OnInteriorPortal>(entity);
                        printf("Exiting InteriorPortal\n");

                        if(!registry.all_of<InteriorColliding>(entity)) {
                            registry.remove<Inside>(entity);
                        }
                    }
                }
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

                // If is Interior disable parent entity collisions
                // if(registry.all_of<Interior>(_entity)) {
                //     continue;
                // }

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

            // TODO
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

            Vector2f collides = positionsCollide(position, shape, tposition, tshape, false);
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
    auto playerPos = registry.get<Position>(_player);
    auto playerShape = registry.get<Shape>(_player);

    auto debug_entities = registry.view<Visible, Interactable, Hoverable, Position, Shape>();
    for (auto entity : debug_entities)
    {
        auto &position = debug_entities.get<Position>(entity);
        auto &shape = debug_entities.get<Shape>(entity);
        auto &interactable = debug_entities.get<Interactable>(entity);
        bool mouseCollides = false;

        // Calculate cursor position in shader coordinates
        float normalizedCursorX = -((cursorPos[0] / width) * 2.0f - 1.0f) - playerShape.scaled_size.x;
        float normalizedCursorY = (1.0f - (cursorPos[1] / height) * 2.0f) - playerShape.scaled_size.y;

        // Check for exact boundary collision
        if (normalizedCursorX >= position.sx - shape.scaled_size.x &&
            normalizedCursorX <= position.sx + shape.scaled_size.x &&
            normalizedCursorY >= position.sy - shape.scaled_size.y &&
            normalizedCursorY <= position.sy + shape.scaled_size.y ) {
            mouseCollides = true;
        }

        if(mouseCollides) {
            if (!registry.all_of<Hovered>(entity))
            {
                registry.emplace<Hovered>(entity);
            }

            if (keys[SDL_BUTTON_LEFT])
            {
                if (!registry.all_of<Interacted>(entity))
                {
                    registry.emplace<Interacted>(entity, _player);
                }
                else {
                    interactable.interactions++;
                    interactable.toggle = !interactable.toggle;
                }

                keys[SDL_BUTTON_LEFT] = false;
            }
            else
            {
                if (registry.all_of<Interacted>(entity))
                {
                    registry.remove<Interacted>(entity);
                }
            }
        }
        else
        {
            if (registry.all_of<Hovered>(entity))
            {
                registry.remove<Hovered>(entity);
            }
            if (registry.all_of<Interacted>(entity))
            {
                registry.remove<Interacted>(entity);
            }
        }
    }
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

void updateLinkedEntities(entt::registry &registry) {

    // Check for new links
    auto _entities = registry.view<Visible, Interactable, Linkable>();
    for(auto& entity : _entities) {
        
        auto &interactable = _entities.get<Interactable>(entity);

        if(registry.all_of<Interacted>(entity)) {
            if(!registry.all_of<Linked>(entity)) {
                auto interact = registry.get<Interacted>(entity); 
                registry.emplace<Linked>(entity, Linked{interact.interactor, 1});
            }
            else {
                registry.remove<Linked>(entity);
            }
        }


    }


    auto entities = registry.view<Position, Shape, Linked>();
    for(auto& entity : entities) {
        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);
        auto &linked = entities.get<Linked>(entity);

        auto parentPos = registry.get<Position>(linked.parent);
        auto parentShape = registry.get<Shape>(linked.parent);

        // Update Movement to follow parent
        // if(registry.all_of<Movement>(entity)) {
        //     auto &movement = registry.get<Movement>(entity);
        //     float dx = parentPos.x - position.x;
        //     float dy = parentPos.y - position.y;
        //     float followSpeed = 0.1f; // Adjust this value to control the following speed
        //     movement.velocity.x = dx * followSpeed;
        //     movement.velocity.y = dy * followSpeed;
        // }

        // If the link distance is > 0 ensure the entity is within the link distance
        if(linked.distance > 0) {
            float distance = std::sqrt(std::pow(position.x - parentPos.x, 2) + std::pow(position.y - parentPos.y, 2));
            if(distance > linked.distance) {
                float angle = std::atan2(parentPos.y - position.y, parentPos.x - position.x);
                float adjustmentSpeed = 0.05f; // Adjust this value to control the adjustment speed
                position.x += (parentPos.x - cos(angle) * linked.distance - position.x) * adjustmentSpeed;
                position.y += (parentPos.y - sin(angle) * linked.distance - position.y) * adjustmentSpeed;
            }
        }
        
    }
}