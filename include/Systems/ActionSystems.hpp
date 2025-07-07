
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include "../JSUtils.hpp"
#include "../lib/physics.hpp"
#include "../EntityFactory.hpp"
#include <vector>
#include <unordered_map>

extern p2d::Physics physics;
extern float deltaTime;

extern entt::entity _player;
bool triggered;

void updateActions(entt::registry &registry)
{
    auto tickAction_entities = registry.view<Visible, TickAction, Movement>();
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
    }

    auto tickAction_entities2 = registry.view<TickAction>();
    for(auto e : tickAction_entities2) {
        auto& tickAction = registry.get<TickAction>(e);
        
        // Check if it's time to trigger the action
        if (tickAction.time + tickAction.interval <= SDL_GetTicks() / 1000.0f) {
            // Execute the action
            if (tickAction.action) {
                tickAction.action(registry, e);
            }
            
            // Reset the timer
            tickAction.time = SDL_GetTicks() / 1000.0f;
        }
    }
    
    auto flagged = registry.view<Flag>();
    for(auto e : flagged) {
        auto& flag = registry.get<Flag>(e);
    }
}


void updateInteractions(entt::registry &registry)
{

    auto playerPos = registry.get<Position>(_player);
    auto playerShape = registry.get<Shape>(_player);
    bool playerInside = registry.all_of<Inside>(_player);
    Cursor &cursor = registry.get<Cursor>(_player);
    
    entt::entity playerInterior = entt::null;
    if (playerInside)
    {
        playerInterior = registry.get<Inside>(_player).interior;
    }

    auto keys = registry.get<Keys>(_player).keys;

    auto debug_entities = registry.view<Visible, Interactable, Hoverable, Position, Shape>();
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
        float normalizedCursorX = -((cursor.position.x / width) * 2.0f - 1.0f) - playerShape.scaled_size.x;
        float normalizedCursorY = (1.0f - (cursor.position.y / height) * 2.0f) - playerShape.scaled_size.y;

        // Normalize interactable radius using the average of the scaled sizes
        float normalizedRadius = interactable.radius * (shape.scaled_size.x + shape.scaled_size.y) / 2.0f;
        
        // Calculate cursor position in screen coordinates
        float cursorScreenX = cursor.position.sx;
        float cursorScreenY = cursor.position.sy;
        
        // Check for boundary collision extended by normalized interactable.radius
        if (normalizedCursorX >= position.sx - shape.scaled_size.x - normalizedRadius &&
            normalizedCursorX <= position.sx + shape.scaled_size.x + normalizedRadius &&
            normalizedCursorY >= position.sy - shape.scaled_size.y - normalizedRadius &&
            normalizedCursorY <= position.sy + shape.scaled_size.y + normalizedRadius)
        {
            mouseCollides = true;
        }
        
        if (mouseCollides)
        {
            if (!registry.all_of<Hovered>(entity))
            {
                registry.emplace<Hovered>(entity);
            }
            else
            {
                auto &hovered = registry.get<Hoverable>(entity);
                hovered.duration += deltaTime;
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
                    auto &interaction = registry.get<Interacted>(entity);
                    interaction.interactions++;
                    // interactable.toggle = !interactable.toggle;
                }

                if(registry.all_of<InteriorPortal>(entity)) {
                    auto door = registry.get<InteriorPortal>(entity);
                    if (registry.all_of<Inside>(_player)) {
                        auto& inside = registry.get<Inside>(_player);
                        if(inside.interior == door.A) inside.interior = door.B;
                        else inside.interior = door.A;
                    } 
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
                auto &hovered = registry.get<Hoverable>(entity);
                hovered.duration = 0;
            }
            if (registry.all_of<Interacted>(entity))
            {
                registry.remove<Interacted>(entity);
            }

            if(keys[SDL_BUTTON_LEFT]) {
                
            }
        }
    }


    // View entities with Interacted component
    auto interactedView = registry.view<Interacted>();
    for(auto entity : interactedView) {
        auto& interacted = registry.get<Interacted>(entity);
        
        // Check if the entity is an InteriorPortal
        if (registry.all_of<InteriorPortal>(entity)) {
            auto &body = registry.get<PhysicsBodyRect>(entity).body;
            // Toggle the ignore state of the body
            body->ignore = !body->ignore;
        }
    }
}

