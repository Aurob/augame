
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include "../JSUtils.hpp"

extern entt::entity _player;
void updateActions(entt::registry &registry)
{
    auto tickAction_entities = registry.view<Visible, TickAction, Movement>();
    for (auto entity : tickAction_entities)
    {
        auto &action = tickAction_entities.get<TickAction>(entity);

        float interval = action.interval;
        if(entity == _player) {
            
            auto &movement = tickAction_entities.get<Movement>(entity);
            
            // if (std::abs(movement.velocity.x) < 0.01f && std::abs(movement.velocity.y) < 0.01f) {
            //     interval *= 4.0f; // Increase interval if player is not moving
            // }
        }

        action.time += deltaTime*5;
        if (action.time >= interval) {
            action.action(registry, entity);
            action.time = 0.0f; // Reset the time after triggering the action
        }
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
        }
    }


    // View entities with Interacted component
    auto interactedView = registry.view<Interacted>();
    for(auto entity : interactedView) {
        auto& interacted = registry.get<Interacted>(entity);
        
        // Check if interactor has Cursor component
        if(registry.all_of<Cursor>(interacted.interactor)) {
            emlog("Moving interacted entity to cursor position");
            
            // Get cursor position and move interacted entity there
            auto& cursor = registry.get<Cursor>(interacted.interactor);
            auto& position = registry.get<Position>(entity);
            position.x = cursor.position.x;
            position.y = cursor.position.y;
        }
    }
}

