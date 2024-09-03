#pragma once

#include "entt.hpp"
#include "structs.hpp"
#include <SDL2/SDL.h>
#include "Utils.hpp"

extern float deltaTime;
extern int width, height;
extern GLfloat cursorPos[2];
extern GLfloat toplefttile[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;


void updateMovement(entt::registry &registry)
{
    auto view = registry.view<Movement, Position, InView>();

    for (auto entity : view)
    {
        auto &movement = view.get<Movement>(entity);
        auto &position = view.get<Position>(entity);

        if (registry.any_of<Keys>(entity))
        {
            auto keys = registry.get<Keys>(entity).keys;
            Vector3f input{static_cast<float>(keys[SDLK_d]) - static_cast<float>(keys[SDLK_a]),
                           static_cast<float>(keys[SDLK_s]) - static_cast<float>(keys[SDLK_w])};
            float length = std::sqrt(input.x * input.x + input.y * input.y);
            if (length != 0) {
                movement.acceleration.x = (input.x / length) * movement.speed;
                movement.acceleration.y = (input.y / length) * movement.speed;
            } else {
                movement.acceleration.x = 0;
                movement.acceleration.y = 0;
            }
        }

        movement.velocity += movement.acceleration * deltaTime;
        movement.velocity *= (1.0f - movement.friction * deltaTime);

        float currentSpeed = movement.velocity.length();
        if (currentSpeed > movement.max_speed)
        {
            movement.velocity *= (movement.max_speed / currentSpeed);
        }

        position.x += movement.velocity.x * deltaTime;
        position.y += movement.velocity.y * deltaTime;

        movement.acceleration = {0, 0};
    }
}

void checkCollisions(entt::registry &registry, entt::entity entity, Position &entityPosition, Shape &entityShape, std::vector<entt::entity> &_collidables, std::vector<Vector3f> &overlaps)
{
    auto collidables = registry.view<Collidable, Position, Shape, InView>();

    for (auto _entity : collidables)
    {
        if (_entity == entity || (registry.any_of<Linked>(_entity) && registry.get<Linked>(_entity).parent == entity))
            continue;

        Position &_entityPosition = registry.get<Position>(_entity);
        Shape &_entityShape = registry.get<Shape>(_entity);

        bool skipCollide = false;
        bool invert = false;

        if (registry.any_of<Inside>(entity) && !registry.any_of<OnInteriorPortal>(entity))
        {
            auto interior = registry.get<Inside>(entity).interior;
            if (interior == _entity)
            {
                invert = true;
            }
        }

        Vector3f overlap = positionsCollide(entityPosition, entityShape, _entityPosition, _entityShape, invert);

        if (overlap.x != 0 || overlap.y != 0 || overlap.z != 0)
        {
            if (registry.any_of<Interior>(_entity))
            {
                if (registry.any_of<Inside>(entity))
                {
                    auto interior = registry.get<Inside>(entity).interior;
                    if (interior == _entity)
                    {
                        if (registry.any_of<OnInteriorPortal>(entity))
                        {
                            skipCollide = true;
                        }
                    }
                    else
                    {
                        auto entityInside = registry.any_of<Inside>(entity) ? registry.get<Inside>(entity).interior : entt::null;
                        auto _entityInside = registry.any_of<Inside>(_entity) ? registry.get<Inside>(_entity).interior : entt::null;
                        if (entityInside != _entityInside)
                        {
                            skipCollide = true;
                        }
                    }
                }
                if (registry.any_of<OnInteriorPortal>(entity))
                {
                    auto portal = registry.get<OnInteriorPortal>(entity).portal;
                    auto portalInterior = registry.get<InteriorPortal>(portal).A;
                    if (portalInterior == _entity)
                    {
                        skipCollide = true;
                    }
                }
            }

            if (registry.any_of<InteriorPortal>(_entity))
            {
                if (!registry.any_of<OnInteriorPortal>(entity))
                {
                    registry.emplace_or_replace<OnInteriorPortal>(entity, OnInteriorPortal{_entity});
                    auto &interiorPortal = registry.get<InteriorPortal>(_entity);
                    auto currentInterior = registry.any_of<Inside>(entity) ? registry.get<Inside>(entity).interior : entt::null;
                    auto newInterior = (interiorPortal.A != currentInterior) ? interiorPortal.A : interiorPortal.B;
                    if (newInterior != entt::null)
                    {
                        registry.emplace_or_replace<Inside>(entity, Inside{newInterior});
                    }
                    else
                    {
                        registry.remove<Inside>(entity);
                    }
                    if (entity == _player && registry.any_of<Associated>(_entity))
                    {
                        auto &associated = registry.get<Associated>(_entity);
                        for (auto assoc_entity : associated.entities)
                        {
                            if (registry.any_of<InteriorPortalTexture>(assoc_entity) && registry.any_of<Textures>(assoc_entity))
                            {
                                auto &textures = registry.get<Textures>(assoc_entity);
                                textures.current = (textures.current + 1) % textures.textures.size();
                            }
                        }
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
            if (registry.any_of<InteriorPortal>(_entity) && registry.any_of<OnInteriorPortal>(entity))
            {
                auto portal = registry.get<OnInteriorPortal>(entity);
                if (portal.portal == _entity)
                {
                    registry.remove<OnInteriorPortal>(entity);

                    auto &interiorPortal = registry.get<InteriorPortal>(_entity);
                    if (!registry.any_of<Inside>(entity))
                    {
                        registry.remove<OnInteriorPortal>(entity);
                    }
                    else
                    {
                        auto interior = registry.get<Inside>(entity).interior;
                        if (interior != interiorPortal.A && interior != interiorPortal.B ||
                            interiorPortal.A == entt::null || interiorPortal.B == entt::null)
                        {
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
                              { return registry.any_of<Player>(lhs) && !registry.any_of<Player>(rhs); });

    auto collidables = registry.view<Collidable, Position, Shape, InView>();

    for (auto entity : collidables)
    {
        if (!registry.any_of<Movement>(entity))
            continue;

        std::vector<entt::entity> _collidables;
        std::vector<Vector3f> overlaps;

        Shape &entityShape = registry.get<Shape>(entity);
        Position &entityPosition = registry.get<Position>(entity);

        checkCollisions(registry, entity, entityPosition, entityShape, _collidables, overlaps);

        if (!_collidables.empty())
        {
            if (registry.any_of<Colliding>(entity))
            {
                auto &colliding = registry.get<Colliding>(entity);
            }
            else
            {
                registry.emplace<Colliding>(entity, Colliding{_collidables, overlaps});
            }
            
            // offset entity position by overlaps
            for (auto &overlap : overlaps)
            {
                entityPosition.x += overlap.x;
                entityPosition.y += overlap.y;
            }

            // Apply movement to colliding entities
            for (auto _entity : _collidables)
            {
                if (registry.any_of<Movement>(_entity))
                {
                    auto &movement = registry.get<Movement>(entity);
                    auto &_movement = registry.get<Movement>(_entity);

                    if (registry.any_of<Moveable>(_entity))
                    {
                        _movement.velocity += movement.velocity * 0.5f;
                        _movement.acceleration += movement.acceleration * 0.5f;
                    }

                    if (registry.any_of<Collidable>(_entity))
                    {
                        auto &collidable = registry.get<Collidable>(_entity);
                        if (!collidable.ignoreCollideAll)
                        {
                            collidable.colliding_with.push_back(entity);
                        }
                    }

                }
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
    bool playerIsInside = registry.all_of<Inside>(_player);

    auto entities = registry.view<Position, Shape>();
    for (auto entity : entities)
    {
        
        auto &position = entities.get<Position>(entity);
        auto &shape = entities.get<Shape>(entity);

        float posX = (playerPos.x - position.x) * gridSpacingValue + width / 2;
        float posY = (playerPos.y - position.y) * gridSpacingValue + height / 2;
        float posZ = (position.z) * gridSpacingValue + height / 2;
        
        position.sx = (2 * posX / width - 1) / defaultGSV - shape.scaled_size.x * 0.999f;
        position.sy = (2 * posY / height - 1) / defaultGSV - shape.scaled_size.y * 0.999f;
        position.sz = (2 * posZ / height - 1) / defaultGSV - shape.scaled_size.z * 0.999f;

        bool isWithinBounds = (entity == _player) || (
            position.sx + shape.scaled_size.x >= -1 
            && position.sx - shape.scaled_size.x <= 1 
            && (position.sy + shape.scaled_size.y + shape.scaled_size.z*2 + shape.scaled_size.y) >= -1
            && (position.sy - shape.scaled_size.y - shape.scaled_size.z*2 - shape.scaled_size.y) <= 1
        );

        bool isVisible = true;
        bool isInView = true;
        if (isWithinBounds)
        {
            bool entityIsPortal = registry.all_of<InteriorPortal>(entity);

            // // If is Inside, but player is not Inside check the Interior and if Interior.hideInside, don't show
            if(registry.all_of<Inside>(entity)) { 
                auto _interior = registry.get<Inside>(entity).interior;

                if(!entityIsPortal) {
                    if(!playerIsInside) {
                        bool hideInside = registry.get<Interior>(_interior).hideInside;
                        if(hideInside) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else {
                        auto playerInside = registry.get<Inside>(_player);

                        if(entity != playerInside.interior && _interior != playerInside.interior) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                }
                else {
                    auto portal = registry.get<InteriorPortal>(entity);
                    if(playerIsInside) {
                        auto playerInside = registry.get<Inside>(_player);

                        if(playerInside.interior != portal.A && playerInside.interior != portal.B) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                    else {
                        if(portal.A != entt::null && portal.B != entt::null) {
                            isVisible = false;
                            isInView = false;
                        }
                    }
                }
            }
            else if(playerIsInside) {
                auto playerInside = registry.get<Inside>(_player);
                if(registry.all_of<Interior>(entity)) {
                    if(entity != playerInside.interior) {
                        isVisible = false;
                        isInView = false;
                    }
                }
                else {
                    if(!registry.all_of<Inside>(entity)) {
                        isVisible = false;
                        isInView = false;
                    }
                }
            }
        }
        

        if (isVisible)
            registry.emplace_or_replace<Visible>(entity);
        else if (registry.all_of<Visible>(entity)) {
            registry.remove<Visible>(entity);
        }
        if (isInView)
            registry.emplace_or_replace<InView>(entity);
        else if (registry.all_of<InView>(entity)) {
            registry.remove<InView>(entity);
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

    auto keys = registry.get<Keys>(_player).keys;

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
                    // interactable.toggle = !interactable.toggle;
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

    auto interacted_entities = registry.view<Interacted, InteractionAction>();
    for (auto entity : interacted_entities)
    {
        auto &interacted = interacted_entities.get<Interacted>(entity);
        auto &action = interacted_entities.get<InteractionAction>(entity);

        // get the interacting entity put in _entity
        auto _entity = interacted.interactor;
        if(action.toggle) {
            auto interaction = registry.get<Interactable>(entity);
            if(interaction.toggle()) {
                action.action(registry, entity, _entity);
            }
        }
        else {
            action.action(registry, entity, _entity);
        }
        
    }


    auto colAction_entities = registry.view<CollisionAction, Colliding>();
    for (auto entity : colAction_entities)
    {
        auto &action = colAction_entities.get<CollisionAction>(entity);
        action.action(registry, entity);
    }

    auto tickAction_entities = registry.view<TickAction, Movement>();
    for (auto entity : tickAction_entities)
    {
        auto &action = tickAction_entities.get<TickAction>(entity);

        float interval = action.interval;
        if(entity == _player) {
            
            auto &movement = tickAction_entities.get<Movement>(entity);
            
            if (std::abs(movement.velocity.x) < 0.01f && std::abs(movement.velocity.y) < 0.01f) {
                interval *= 4.0f; // Increase interval if player is not moving
            }
        }

        action.time += deltaTime;
        if (action.time >= interval) {
            action.action(registry, entity);
            action.time = 0.0f; // Reset the time after triggering the action
        }
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
        shape.scaled_size.z = (shape.size.z / defaultGSV) * gridSpacingValue / height;
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

    auto collidable_entities = registry.view<Position, Shape, Collidable, InView, Teleportable>();
    for (auto &entity : collidable_entities)
    {
        auto &position = collidable_entities.get<Position>(entity);
        auto &shape = collidable_entities.get<Shape>(entity);

        auto teleport_entities = registry.view<Position, Shape, Teleport, InView>();
        for (auto &tentity : teleport_entities)
        {

            printf("Checking teleport\n");
            auto &tposition = teleport_entities.get<Position>(tentity);
            auto &tshape = teleport_entities.get<Shape>(tentity);
            auto &teleport = teleport_entities.get<Teleport>(tentity);

            Vector3f collides = positionsCollide(position, shape, tposition, tshape, false);
            if (collides.x != 0 || collides.y != 0 || collides.z != 0)
            {
                position.x = teleport.destination.x;
                position.y = teleport.destination.y;
                position.z = teleport.destination.z;

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

void updateFlags(entt::registry &registry)
{
    std::vector<entt::entity> entitiesToDestroy;
    auto entities = registry.view<Flag>();
    for (auto entity : entities)
    {
        auto &flag = entities.get<Flag>(entity);
        for (const auto& [flagName, flagValue] : flag.flags)
        {
            if (flagName == "Destroy" && std::any_cast<bool>(flagValue))
            {
                // printf if has InteractionAction
                if (registry.all_of<InteractionAction>(entity))
                {
                }
                // Mark associated entities for destruction
                auto associatedView = registry.view<Associated>();
                for (auto [associatedEntity, associated] : associatedView.each())
                {
                    if (associatedEntity != entity &&
                        std::find(associated.entities.begin(), associated.entities.end(), entity) != associated.entities.end())
                    {
                        if(associated.destroy) {
                            entitiesToDestroy.push_back(associatedEntity);
                        }
                    }
                }

                entitiesToDestroy.push_back(entity);
                break;
            }
        }
    }

    // Destroy marked entities
    for (auto entityToDestroy : entitiesToDestroy)
    {
        if (registry.valid(entityToDestroy))
        {
            registry.destroy(entityToDestroy);
        }
    }
}