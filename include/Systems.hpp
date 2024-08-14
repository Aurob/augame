#pragma once

#include "entt.hpp"
#include "structs.hpp"
#include <SDL2/SDL.h>
#include "Utils.hpp"

extern unordered_map<int, bool> keys;
extern float deltaTime;
extern int width, height;
extern GLfloat cursorPos[2];
extern GLfloat toplefttile[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;

void updateMovement(entt::registry &registry)
{
    auto view = registry.view<Movement, Position>();

    for (auto entity : view)
    {
        auto &movement = view.get<Movement>(entity);
        auto &position = view.get<Position>(entity);

        bool isPlayer = registry.all_of<Player>(entity);

        // Handle player input
        if (isPlayer)
        {
            Vector2f input{static_cast<float>(keys[SDLK_d]) - static_cast<float>(keys[SDLK_a]),
                           static_cast<float>(keys[SDLK_s]) - static_cast<float>(keys[SDLK_w])};
            movement.acceleration = input.normalized() * movement.speed;
        }

        // Update velocity
        movement.velocity += movement.acceleration * deltaTime;

        // Apply drag
        movement.velocity *= (1.0f - movement.friction * deltaTime);

        // Clamp velocity to max speed
        float currentSpeed = movement.velocity.length();
        if (currentSpeed > movement.max_speed)
        {
            movement.velocity *= (movement.max_speed / currentSpeed);
        }

        // Update position
        position.x += movement.velocity.x * deltaTime;
        position.y += movement.velocity.y * deltaTime;

        // Reset acceleration
        movement.acceleration = {0, 0};
    }
}

void updateInteriorCollisions(entt::registry &registry, entt::entity entity, Position &entityPosition, Shape &entityShape, std::vector<entt::entity> &_collidables, std::vector<Vector2f> &overlaps)
{

    Position playerPos = registry.get<Position>(entity);
    auto collidables = registry.view<Collidable, Position, Shape, InView>();

    for (auto _entity : collidables)
    {
        if (_entity == entity)
            continue;

        if (registry.all_of<Linked>(_entity) && registry.get<Linked>(_entity).parent == entity)
            continue;

        Position &_entityPosition = registry.get<Position>(_entity);
        Shape &_entityShape = registry.get<Shape>(_entity);

        bool skipCollide = false;
        bool invert = false;

        if (registry.all_of<Inside>(entity) && !registry.all_of<OnInteriorPortal>(entity))
        {

            auto interior = registry.get<Inside>(entity).interior;
            if (interior == _entity)
            {
                invert = true;
            }
        }

        Vector2f overlap = positionsCollide(entityPosition, entityShape, _entityPosition, _entityShape, invert);

        if (overlap.x != 0 || overlap.y != 0)
        {
         

            if (registry.all_of<Interior>(_entity))
            {
                if (registry.all_of<Inside>(entity))
                {
                    auto interior = registry.get<Inside>(entity).interior;
                    if (interior == _entity)
                    {
                        if (!registry.all_of<OnInteriorPortal>(entity))
                        {
                            _collidables.push_back(_entity);
                            overlaps.push_back(overlap);
                        }
                        else
                        {
                            skipCollide = true;
                        }
                    }
                    else {
                        auto entityInside = registry.all_of<Inside>(entity) ? registry.get<Inside>(entity).interior : entt::null;
                        auto _entityInside = registry.all_of<Inside>(_entity) ? registry.get<Inside>(_entity).interior : entt::null;
                        if (entityInside != _entityInside) {
                            skipCollide = true;
                        }
                    }
                }
                if (registry.all_of<OnInteriorPortal>(entity))
                {
                    auto portal = registry.get<OnInteriorPortal>(entity).portal;
                    auto portalInterior = registry.get<InteriorPortal>(portal).A;
                    if (portalInterior == _entity)
                    {
                        skipCollide = true;
                    }
                }
            }

            if (registry.all_of<InteriorPortal>(_entity))
            {
                if (!registry.all_of<OnInteriorPortal>(entity))
                {
                    registry.emplace_or_replace<OnInteriorPortal>(entity, OnInteriorPortal{_entity});
                    auto &interiorPortal = registry.get<InteriorPortal>(_entity);
                    auto currentInterior = registry.all_of<Inside>(entity) ? registry.get<Inside>(entity).interior : entt::null;
                    auto newInterior = (interiorPortal.A != currentInterior) ? interiorPortal.A : interiorPortal.B;
                    if (newInterior != entt::null)
                    {
                        registry.emplace_or_replace<Inside>(entity, Inside{newInterior});
                    }
                    else
                    {
                        registry.remove<Inside>(entity);
                    }
                }
                skipCollide = true;
            }

            if (!skipCollide)
            {
                _collidables.push_back(_entity);
                overlaps.push_back(overlap);
            }
        }
        else
        {
            if (registry.all_of<Interior>(_entity) && registry.all_of<InteriorColliding>(entity))
            {
                // registry.remove<InteriorColliding>(entity);
            }
            if (registry.all_of<InteriorPortal>(_entity) && registry.all_of<OnInteriorPortal>(entity))
            {
                auto portal = registry.get<OnInteriorPortal>(entity);
                if (portal.portal == _entity)
                {
                    registry.remove<OnInteriorPortal>(entity);

                    auto& interiorPortal = registry.get<InteriorPortal>(_entity);
                    if (!registry.all_of<Inside>(entity)) {
                        registry.remove<OnInteriorPortal>(entity);
                    } else {
                        auto interior = registry.get<Inside>(entity).interior;
                        if (interior != interiorPortal.A && interior != interiorPortal.B ||
                            interiorPortal.A == entt::null || interiorPortal.B == entt::null) {
                            registry.remove<OnInteriorPortal>(entity);
                        }
                    }
                }
            }
        }
    }
}

void updateCollisions(entt::registry &registry)
{
    registry.sort<Collidable>([&](const entt::entity lhs, const entt::entity rhs)
                              { return registry.all_of<Player>(lhs) && !registry.all_of<Player>(rhs); });

    auto collidables = registry.view<Collidable, Position, Shape, InView>();

    for (auto entity : collidables)
    {
        if (!registry.all_of<Movement>(entity))
            continue;

        std::vector<entt::entity> _collidables;
        std::vector<Vector2f> overlaps;

        Shape &entityShape = registry.get<Shape>(entity);
        Position &entityPosition = registry.get<Position>(entity);

        updateInteriorCollisions(registry, entity, entityPosition, entityShape, _collidables, overlaps);

        if (!_collidables.empty())
        {
            if (registry.all_of<Colliding>(entity)) {
                auto& colliding = registry.get<Colliding>(entity);
                colliding.collidables = _collidables;
            } else {
                registry.emplace<Colliding>(entity, Colliding{_collidables, overlaps});
            }

            if(registry.get<Collidable>(entity).ignorePlayer) {
                continue;
            }
            else if(registry.get<Collidable>(entity).ignoreCollideAll) {
                continue;
            }

            auto &movement = registry.get<Movement>(entity);
            Vector2f totalOverlap{0, 0};
            for (size_t i = 0; i < _collidables.size(); ++i)
            {
                auto other = _collidables[i];
                if ((registry.get<Collidable>(other).ignorePlayer && registry.all_of<Player>(entity)) ||
                    registry.get<Collidable>(other).ignoreCollideAll)
                {
                    continue;
                }
                const auto &overlap = overlaps[i];
                Vector2f normal = overlap.normalized();

                bool otherHasMovement = registry.all_of<Movement>(other);
                float entityMass = movement.mass;
                float otherMass = otherHasMovement ? registry.get<Movement>(other).mass : std::numeric_limits<float>::infinity();

                Vector2f relativeVelocity = movement.velocity;
                if (otherHasMovement)
                {
                    relativeVelocity -= registry.get<Movement>(other).velocity;
                }

                float velocityAlongNormal = relativeVelocity.dot(normal);

                if (velocityAlongNormal < 0)
                {
                    float restitution = movement.restitution;
                    if (otherHasMovement)
                    {
                        restitution = std::max(restitution, registry.get<Movement>(other).restitution);
                    }

                    float j = -(1 + restitution) * velocityAlongNormal;
                    j /= 1 / entityMass + (otherHasMovement ? 1 / otherMass : 0);

                    Vector2f impulse = normal * j;
                    movement.velocity += impulse / entityMass;

                    if (otherHasMovement)
                    {
                        auto &otherMovement = registry.get<Movement>(other);
                        otherMovement.velocity -= impulse / otherMass;
                    }
                }

                totalOverlap += overlap;
            }
            // Anti-tunneling
            float maxMovement = std::min(movement.max_speed * deltaTime, 0.5f); // Limit max movement to half the smallest object size
            float totalOverlapMagnitude = totalOverlap.length();

            if (totalOverlapMagnitude > maxMovement)
            {
                totalOverlap = totalOverlap.normalized() * maxMovement;
            }

            // Apply position correction in smaller steps
            const int steps = 25;
            for (int i = 0; i < steps; ++i)
            {
                Vector2f stepCorrection = totalOverlap / static_cast<float>(steps);
                entityPosition.x += stepCorrection.x;
                entityPosition.y += stepCorrection.y;

                // Check for new collisions after each step
                std::vector<entt::entity> stepCollidables;
                std::vector<Vector2f> stepOverlaps;
                updateInteriorCollisions(registry, entity, entityPosition, entityShape, stepCollidables, stepOverlaps);

                if (!stepCollidables.empty())
                {
                    // If new collision detected, revert this step and break
                    entityPosition.x -= stepCorrection.x;
                    entityPosition.y -= stepCorrection.y;
                    break;
                }
            }

            // Adjust velocity based on the actual position change
            Vector2f actualMovement = {entityPosition.x - (entityPosition.x - totalOverlap.x),
                                       entityPosition.y - (entityPosition.y - totalOverlap.y)};
            movement.velocity = actualMovement / deltaTime;

            // Ensure velocity doesn't exceed max_speed
            float speed = movement.velocity.length();
            if (speed > movement.max_speed)
            {
                movement.velocity = movement.velocity.normalized() * movement.max_speed;
            }
        }
        else
        {
            registry.remove<Colliding>(entity);
        }
    }
}

void updatePositions(entt::registry &registry)
{
    Position playerPos = registry.get<Position>(_player);
    auto entities = registry.view<Position, Shape>();

    for (auto entity : entities)
    {
        
        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);

        float gridOffsetX = playerPos.x - (width / 2);
        float gridOffsetY = playerPos.y - (height / 2);

        float entityGridX = position.x - gridOffsetX;
        float entityGridY = position.y - gridOffsetY;

        float posX = (playerPos.x - position.x) * gridSpacingValue + width / 2;
        float posY = (playerPos.y - position.y) * gridSpacingValue + height / 2;
        float posZ = position.z * gridSpacingValue;
        position.sx = (2 * posX / width - 1) / defaultGSV - shape.scaled_size.x * 0.999f;
        position.sy = (2 * posY / height - 1) / defaultGSV - shape.scaled_size.y * 0.999f;
        position.sz = (2 * posZ / height - 1) / defaultGSV - shape.scaled_size.z * 0.999f;

        bool isWithinBounds = (entity == _player) || (position.sx + shape.scaled_size.x * 4 >= -1 && position.sx - shape.scaled_size.x * 4 <= 1 &&
                                                      position.sy + shape.scaled_size.y * 4 >= -1 && position.sy - shape.scaled_size.y * 4 <= 1);
        bool isAtOrAbovePlayer = position.z >= playerPos.z;
        if (isWithinBounds) // && isAtOrAbovePlayer)
        {
            bool shouldBeVisible = false;
            bool shouldBeInView = true;

            // if player is inside an interior, only show entities that are inside the same interior
            if (registry.all_of<Inside>(_player))
            {
                auto playerInterior = registry.get<Inside>(_player).interior;

                if (registry.all_of<Interior>(entity) && entity == playerInterior)
                {
                    shouldBeVisible = true;
                }

                if (registry.all_of<Inside>(entity))
                {
                    auto entityInterior = registry.get<Inside>(entity).interior;
                    if (playerInterior == entityInterior)
                    {
                        shouldBeVisible = true;
                    }
                }

                if (registry.all_of<InteriorPortal>(entity))
                {
                    auto &portal = registry.get<InteriorPortal>(entity);
                    if (portal.A == playerInterior || portal.B == playerInterior)
                    {
                        shouldBeVisible = true;
                    }
                }
            }
            else
            {
                // Check if the entity is inside something
                if (!registry.all_of<Inside>(entity))
                {
                    shouldBeVisible = true;
                }
                else {
                    shouldBeInView = false;
                }
            }

            if (shouldBeVisible && !registry.all_of<Visible>(entity))
            {
                registry.emplace<Visible>(entity);
            }
            else if (!shouldBeVisible && registry.all_of<Visible>(entity))
            {
                registry.remove<Visible>(entity);
            }

            if (shouldBeInView && !registry.all_of<InView>(entity))
            {
                registry.emplace<InView>(entity);
            }
            else if (!shouldBeInView && registry.all_of<InView>(entity))
            {
                registry.remove<InView>(entity);
            }
        }
        else
        {
            registry.remove<Visible>(entity);
            registry.remove<InView>(entity);
        }
    }
}

void updateTeleporters(entt::registry &registry)
{
    // do collision checks for teleport entities
    auto collidable_entities = registry.view<Position, Shape, Collidable, InView, Teleportable>();
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

                if (teleport.reverse)
                {
                    if (registry.all_of<Movement>(entity))
                    {
                        auto &movement = registry.get<Movement>(entity);
                        movement.velocity.x = 0;
                        movement.velocity.y = 0;
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
    if (playerInside)
    {
        playerInterior = registry.get<Inside>(_player).interior;
    }

    auto debug_entities = registry.view<InView, Interactable, Hoverable, Position, Shape>();
    for (auto entity : debug_entities)
    {
        // Skip entities that are not in the same interior as the player
        if (playerInside)
        {
            if (!registry.all_of<Inside>(entity) || registry.get<Inside>(entity).interior != playerInterior)
            {
                continue;
            }
        }
        else if (registry.all_of<Inside>(entity))
        {
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
            normalizedCursorY <= position.sy + shape.scaled_size.y)
        {
            mouseCollides = true;
        }

        if (mouseCollides)
        {
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
                else
                {
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

    // For Interacted entities, perform the interaction action
    auto interacted_entities = registry.view<Interacted, InteractionAction>();
    for (auto entity : interacted_entities)
    {
        auto &interacted = interacted_entities.get<Interacted>(entity);
        auto &action = interacted_entities.get<InteractionAction>(entity);

        for (auto &entity : interacted_entities)
        {
            action.action(registry, entity);
        }
    }

    // For Colliding entities, perform the collision action
    auto colliding_entities = registry.view<Colliding, CollisionAction>();
    for (auto entity : colliding_entities)
    {
        auto &colliding = colliding_entities.get<Colliding>(entity);
        auto &action = colliding_entities.get<CollisionAction>(entity);
        printf("Colliding\n");
        // for (auto &collidable : colliding.collidables)
        // {
        //     action.action(registry, entity, collidable);
        // }
    }
}

void updateShapes(entt::registry &registry)
{

    // update shapes
    auto entities = registry.view<Shape>();
    for (auto &entity : entities)
    {
        auto &shape = entities.get<Shape>(entity);
        shape.scaled_size.x = (shape.size.x / defaultGSV) * gridSpacingValue / width;
        shape.scaled_size.y = (shape.size.y / defaultGSV) * gridSpacingValue / height;
    }
}

void updateLinkedEntities(entt::registry &registry)
{

    // Check for new links
    auto _entities = registry.view<InView, Interactable, Linkable>();
    for (auto &entity : _entities)
    {

        auto &interactable = _entities.get<Interactable>(entity);

        if (registry.all_of<Interacted>(entity))
        {
            if (!registry.all_of<Linked>(entity))
            {
                auto interact = registry.get<Interacted>(entity);
                registry.emplace<Linked>(entity, Linked{interact.interactor, 1});
            }
            else
            {
                registry.remove<Linked>(entity);
            }
        }
    }

    auto entities = registry.view<Position, Shape, Linked>();
    for (auto &entity : entities)
    {
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
        if (linked.distance > 0)
        {
            float distance = std::sqrt(std::pow(position.x - parentPos.x, 2) + std::pow(position.y - parentPos.y, 2));
            if (distance > linked.distance)
            {
                float angle = std::atan2(parentPos.y - position.y, parentPos.x - position.x);
                float adjustmentSpeed = 0.05f; // Adjust this value to control the adjustment speed
                position.x += (parentPos.x - cos(angle) * linked.distance - position.x) * adjustmentSpeed;
                position.y += (parentPos.y - sin(angle) * linked.distance - position.y) * adjustmentSpeed;
            }
        }
    }
}

void updateOther(entt::registry &registry)
{
    auto entities = registry.view<Color>();
    for (auto &entity : entities)
    {
        auto &color = entities.get<Color>(entity);
        // If hovered over, change color
        if (registry.all_of<Hovered>(entity) || registry.all_of<Interacted>(entity))
        {
            if (registry.all_of<Interacted>(entity))
            {
                color.r = 255;
                color.g = 0;
                color.b = 0;
            }
            else
            {
                color.r = 0;
                color.g = 255;
                color.b = 0;
            }
        }
        else
        {
            color.r = color.defaultR;
            color.g = color.defaultG;
            color.b = color.defaultB;
        }
    }
}

void updatePaths(entt::registry &registry)
{
    auto entities = registry.view<BasicPathfinding, Position, Movement>();
    for (auto &entity : entities)
    {
        auto &pathfinding = entities.get<BasicPathfinding>(entity);
        auto &position = entities.get<Position>(entity);
        auto &movement = entities.get<Movement>(entity);

        float pathModX = 0;
        float pathModY = 0;

        // If the entity is currently colliding, iterate and adjust pathMod values perpendicularly to the overlap direction
        if (registry.all_of<Colliding>(entity))
        {
            auto &colliding = registry.get<Colliding>(entity);
            for (auto &overlap : colliding.overlaps)
            {
                if (overlap.x > 0)
                {
                    pathModX += -overlap.y;
                }
                else
                {
                    pathModX += overlap.y;
                }
                if (overlap.y > 0)
                {
                    pathModY += overlap.x;
                }
                else
                {
                    pathModY += -overlap.x;
                }
            }
        }

        if (pathfinding.target != entt::null)
        {
            auto targetPos = registry.get<Position>(pathfinding.target);
            auto targetShape = registry.get<Shape>(pathfinding.target);
            float dx = targetPos.x - position.x + pathModX;
            float dy = targetPos.y - position.y + pathModY;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance > 0)
            {
                dx /= distance;
                dy /= distance;
            }
            movement.velocity.x = dx * movement.speed;
            movement.velocity.y = dy * movement.speed;
        }
    }
}

void updateFlags(entt::registry &registry)
{
    auto entities = registry.view<Flag>();
    for (auto &entity : entities)
    {
        auto &flag = entities.get<Flag>(entity);
        for (const auto& [flagName, flagValue] : flag.flags)
        {
            if (flagName == "Destroy" && std::any_cast<bool>(flagValue))
            {
                // Destroy associated entities
                auto associatedView = registry.view<Associated>();
                for (auto [associatedEntity, associated] : associatedView.each()) {
                    if (std::find(associated.entities.begin(), associated.entities.end(), entity) != associated.entities.end()) {
                        registry.destroy(associatedEntity);
                    }
                }
                
                // Destroy the entity itself
                registry.destroy(entity);
                break;
            }
        }
    }
}