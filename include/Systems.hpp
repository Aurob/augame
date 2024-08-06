#pragma once

#include "entt.hpp"
#include "structs.hpp"
#include <SDL2/SDL.h>
#include "Utils.hpp"

extern unordered_map<int, bool> keys;
extern float deltaTime;
extern int width, height;
extern GLfloat cursorPos[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;
extern float moveSpeed;


void updateMovement(entt::registry &registry)
{
    auto movement_entities = registry.view<InView, Movement, Position, Shape>();

    for (auto entity : movement_entities)
    {
        auto &movement = movement_entities.get<Movement>(entity);
        auto &position = movement_entities.get<Position>(entity);
        auto &shape = movement_entities.get<Shape>(entity);
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
            movement.velocity.x += movement.acceleration.x * deltaTime;
            movement.velocity.y += movement.acceleration.y * deltaTime;
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

        // Calculate the movement step
        float stepX = movement.velocity.x * deltaTime;
        float stepY = movement.velocity.y * deltaTime;

        // Move in smaller increments to prevent tunneling
        const int substeps = 10;
        for (int i = 0; i < substeps; ++i)
        {
            position.x += stepX / substeps;
            position.y += stepY / substeps;
        }
    }
}


void updateCollisions(entt::registry &registry)
{
    // Sort so that the player is checked first
    registry.sort<Collidable>([&](const entt::entity lhs, const entt::entity rhs)
                              { return registry.all_of<Player>(lhs) && !registry.all_of<Player>(rhs); });

    auto collidables = registry.view<Collidable, Position, Shape, InView>();

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
                if(_link.parent == entity) {
                    continue;
                }
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
                registry.emplace<Colliding>(entity, Colliding{overlaps});
            }

            // Calculate the maximum allowed movement per frame to prevent tunneling
            float maxMovement = std::min(entityShape.size.x, entityShape.size.y) / 2.0f;
            float currentMovement = std::sqrt(xv1 * xv1 + yv1 * yv1) * deltaTime;

            if (currentMovement > maxMovement)
            {
                float scale = maxMovement / currentMovement;
                entityMovement.velocity.x *= scale;
                entityMovement.velocity.y *= scale;
                xv1 = entityMovement.velocity.x;
                yv1 = entityMovement.velocity.y;
            }

            const float defaultRestitution = 0.5f;

            for (auto _entity : _collidables)
            {
                if (registry.all_of<Movement>(_entity) && registry.all_of<Moveable>(_entity))
                {
                    Movement &_entityMovement = registry.get<Movement>(_entity);
                    float m2 = _entityMovement.mass;
                    float xv2 = _entityMovement.velocity.x;
                    float yv2 = _entityMovement.velocity.y;

                    // Apply friction
                    float frictionCoeff = (entityMovement.friction + _entityMovement.friction) / 2.0f;
                    xv1 *= (1.0f - frictionCoeff * deltaTime);
                    yv1 *= (1.0f - frictionCoeff * deltaTime);
                    xv2 *= (1.0f - frictionCoeff * deltaTime);
                    yv2 *= (1.0f - frictionCoeff * deltaTime);

                    float xv1f = ((m1 - m2) / (m1 + m2)) * xv1 + ((2 * m2) / (m1 + m2)) * xv2;
                    float xv2f = ((2 * m1) / (m1 + m2)) * xv1 + ((m2 - m1) / (m1 + m2)) * xv2;

                    float yv1f = ((m1 - m2) / (m1 + m2)) * yv1 + ((2 * m2) / (m1 + m2)) * yv2;
                    float yv2f = ((2 * m1) / (m1 + m2)) * yv1 + ((m2 - m1) / (m1 + m2)) * yv2;

                    // Apply restitution
                    entityMovement.velocity.x = xv1f * defaultRestitution;
                    entityMovement.velocity.y = yv1f * defaultRestitution;

                    _entityMovement.velocity.x = xv2f * defaultRestitution;
                    _entityMovement.velocity.y = yv2f * defaultRestitution;
                }
            }

            for (const auto &overlap : overlaps)
            {
                overlapx += overlap.x;
                overlapy += overlap.y;
            }
            
            // Apply separation to resolve overlap
            entityPosition.x += overlapx;
            entityPosition.y += overlapy;

            // Adjust velocity based on the collision normal
            if (overlapx != 0 || overlapy != 0)
            {
                Vector2f normal{overlapx, overlapy};
                float length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
                if (length != 0) {
                    normal.x /= length;
                    normal.y /= length;
                }
                float dotProduct = entityMovement.velocity.x * normal.x + entityMovement.velocity.y * normal.y;
                entityMovement.velocity.x -= (1.0f + defaultRestitution) * dotProduct * normal.x;
                entityMovement.velocity.y -= (1.0f + defaultRestitution) * dotProduct * normal.y;
            }

            // Apply additional anti-tunneling measure
            float speed = std::sqrt(entityMovement.velocity.x * entityMovement.velocity.x + 
                                    entityMovement.velocity.y * entityMovement.velocity.y);
            if (speed > maxMovement / deltaTime)
            {
                float scaleFactor = (maxMovement / deltaTime) / speed;
                entityMovement.velocity.x *= scaleFactor;
                entityMovement.velocity.y *= scaleFactor;
            }
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
    auto collidable_entities = registry.view<Position, Shape, Collidable, InView>();
    for (auto &entity : collidable_entities)
    {
        auto &position = collidable_entities.get<Position>(entity);
        auto &shape = collidable_entities.get<Shape>(entity);

        auto teleport_entities = registry.view<Position, Shape, Teleport, InView>();
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
    bool playerInside = registry.all_of<Inside>(_player);
    entt::entity playerInterior = entt::null;
    if (playerInside) {
        playerInterior = registry.get<Inside>(_player).interior;
    }

    auto debug_entities = registry.view<InView, Interactable, Hoverable, Position, Shape>();
    for (auto entity : debug_entities)
    {
        // Skip entities that are not in the same interior as the player
        if (playerInside) {
            if (!registry.all_of<Inside>(entity) || registry.get<Inside>(entity).interior != playerInterior) {
                continue;
            }
        } else if (registry.all_of<Inside>(entity)) {
            continue;
        }

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
    bool playerIsInside = registry.all_of<Inside>(_player);
    entt::entity playerInterior = (playerIsInside) ? registry.get<Inside>(_player).interior : entt::null;
 
    auto entities = registry.view<Position, Shape>();
    for (auto &entity : entities)
    {
        bool skipCheck = false;
        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);
        bool has_visible = registry.all_of<Visible>(entity);
        bool is_inView = registry.all_of<InView>(entity);


        if(registry.all_of<Inside>(entity)) {
            if(!playerIsInside) {
                skipCheck = true;
            }
            else if(!registry.any_of<Interior, InteriorPortal>(entity)) {
                auto interior = registry.get<Inside>(entity).interior;
                if(interior != playerInterior) {
                    skipCheck = true;
                }
            }
        }
        else if(!registry.any_of<Interior, InteriorPortal>(entity)){
            if(playerIsInside) {
                printf("Skipping check\n");
                skipCheck = true;
            }
        }

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
            if (!has_visible && !skipCheck)
            {
                registry.emplace<Visible>(entity);
            }
            else if(skipCheck) {
                if(has_visible) {
                    registry.remove<Visible>(entity);
                }
                if(is_inView) {
                    registry.remove<InView>(entity);
                }
            }

            if(!is_inView) {
                registry.emplace<InView>(entity);
            }
        }
        else
        {
            if(has_visible) {
                registry.remove<Visible>(entity);
            }
            if(is_inView) {
                registry.remove<InView>(entity);
            }
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
    auto _entities = registry.view<InView, Interactable, Linkable>();
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

void updateOther(entt::registry &registry) {
    auto entities = registry.view<Color>();
    for(auto& entity : entities) {
        auto &color = entities.get<Color>(entity);
        // If hovered over, change color
        if(registry.all_of<Hovered>(entity) || registry.all_of<Interacted>(entity)) {
            if(registry.all_of<Interacted>(entity)) {
                color.r = 255;
                color.g = 0;
                color.b = 0;
            }
            else {
                color.r = 0;
                color.g = 255;
                color.b = 0;
            }
        }
        else {
            color.r = color.defaultR;
            color.g = color.defaultG;
            color.b = color.defaultB;
        }
    }

}

void updatePaths(entt::registry &registry) {
    auto entities = registry.view<BasicPathfinding, Position, Movement>();
    for(auto& entity : entities) {
        auto &pathfinding = entities.get<BasicPathfinding>(entity);
        auto &position = entities.get<Position>(entity);
        auto &movement = entities.get<Movement>(entity);

        float pathModX = 0;
        float pathModY = 0;

        // If the entity is currently colliding, iterate and adjust pathMod values perpendicularly to the overlap direction
        if(registry.all_of<Colliding>(entity)) {
            auto &colliding = registry.get<Colliding>(entity);
            for(auto& overlap : colliding.overlaps) {
                if (overlap.x > 0) {
                    pathModX += -overlap.y;
                } else {
                    pathModX += overlap.y;
                }
                if (overlap.y > 0) {
                    pathModY += overlap.x;
                } else {
                    pathModY += -overlap.x;
                }
            }
        }

        if(pathfinding.target != entt::null) {
            auto targetPos = registry.get<Position>(pathfinding.target);
            auto targetShape = registry.get<Shape>(pathfinding.target);
            float dx = targetPos.x - position.x + pathModX;
            float dy = targetPos.y - position.y + pathModY;
            float distance = std::sqrt(dx * dx + dy * dy);
            if(distance > 0) {
                dx /= distance;
                dy /= distance;
            }
            movement.velocity.x = dx * movement.speed;
            movement.velocity.y = dy * movement.speed;
        }
    }
}