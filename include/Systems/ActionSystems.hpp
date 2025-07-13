
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

        // Rectangle (AABB) collision: no more radius, just exact bbox
        if (normalizedCursorX >= position.sx - shape.scaled_size.x &&
            normalizedCursorX <= position.sx + shape.scaled_size.x &&
            normalizedCursorY >= position.sy - shape.scaled_size.y &&
            normalizedCursorY <= position.sy + shape.scaled_size.y)
        {
            mouseCollides = true;
        }

        // Check if the player is already interacting with something
        bool playerIsInteracting = false;
        entt::entity interactingEntity = entt::null;
        auto interactedView = registry.view<Interacted>();
        for (auto e : interactedView) {
            auto &interacted = registry.get<Interacted>(e);
            if (interacted.interactor == _player) {
                playerIsInteracting = true;
                interactingEntity = e;
                break;
            }
        }

        // If the player is already interacting and mouse is still down, do not update anything else
        if (playerIsInteracting && keys[SDL_BUTTON_LEFT] && entity != interactingEntity) {
            // Only allow the currently interacted entity to process interaction
            // Remove hover state from others
            if (registry.all_of<Hovered>(entity)) {
                registry.remove<Hovered>(entity);
                auto &hovered = registry.get<Hoverable>(entity);
                hovered.duration = 0;
            }
            if (registry.all_of<Interacted>(entity)) {
                registry.remove<Interacted>(entity);
            }
            // Do not process further for this entity
            // (skip to next entity)
        }
        else if (mouseCollides)
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
        }
        else
        {
            if (registry.all_of<Hovered>(entity))
            {
                registry.remove<Hovered>(entity);
                auto &hovered = registry.get<Hoverable>(entity);
                hovered.duration = 0;
            }
            // Only remove Interacted if mouse is not down
            if (registry.all_of<Interacted>(entity) && !keys[SDL_BUTTON_LEFT])
            {
                registry.remove<Interacted>(entity);
            }
        }
    }

    // View entities with Interacted component
    auto interactedView = registry.view<Interacted>();
    for(auto entity : interactedView) {
        auto& interacted = registry.get<Interacted>(entity);
        // If the interactor has the Player component
        if (registry.all_of<Player>(interacted.interactor)) {
            // Check if the player has a Cursor component
            if (registry.all_of<Cursor>(interacted.interactor)) {
                auto& cursor = registry.get<Cursor>(interacted.interactor);
                // If Cursor.downtime > 2, move the interacted object's position to the cursor position
                if (cursor.downtime > 2) {
                    float final_x = cursor.position.sx;
                    float final_y = cursor.position.sy;

                    if (registry.all_of<PhysicsBodyRect>(entity)) {
                        auto& physBody = registry.get<PhysicsBodyRect>(entity);
                        if (physBody.body) {
                            physBody.body->setPosition({final_x, final_y});
                        }
                    }
                    // If entity does not have a PhysicsBodyRect, just set its Position
                    if (registry.all_of<Position>(entity)) {
                        auto& pos = registry.get<Position>(entity);
                        pos.x = final_x;
                        pos.y = final_y;
                    }
                }
            }
        }

        // Check if the entity is an InteriorPortal
        if (registry.all_of<InteriorPortal>(entity)) {
            auto &body = registry.get<PhysicsBodyRect>(entity).body;
            // Toggle the ignore state of the body
            body->ignore = !body->ignore;
        }
    }
}

