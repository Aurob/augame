
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include "../JSUtils.hpp"
#include "../lib/physics.hpp"
#include <vector>

extern p2d::Physics physics;
extern float deltaTime;

extern entt::entity _player;
bool triggered;
int current_target = 0;
std::vector<std::string> target_list = {"door1", "door2", "thing"};
int ctdx = 1;
int dialog_stage = 0;
int dialog_times = 1;
std::vector<std::string> dialog_list = {
    "Hello! Enter the next room whenever you're ready.",
    "Please Join me to the next room",
    "One more room to go",
    "There it is!",
};
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


    int trigger_room = 2;

    std::string target_npc = "npc";
    auto onDoor_entities = registry.view<OnInteriorPortal, Inside>();
    for (auto entity: onDoor_entities) {
        auto ip = onDoor_entities.get<OnInteriorPortal>(entity);
        auto in = onDoor_entities.get<Inside>(entity);
        if(registry.get<Id>(in.interior).id == trigger_room) {
            triggered = true;
            break;
        }
    }

    // Find the target NPC and door entities (only once)
    entt::entity npcEntity = entt::null;
    entt::entity doorEntity = entt::null;
   
    if (triggered && (npcEntity == entt::null || doorEntity == entt::null)) {
        auto idView = registry.view<Id>();
        for (auto e : idView) {
            auto& id = idView.get<Id>(e);
    
            if (id.name == target_npc) {
                npcEntity = e;
            } else if (id.name == target_list[current_target]) {
                doorEntity = e;
            }
            
            if (npcEntity != entt::null && doorEntity != entt::null) {
                break;
            }
        }
    }
    
    // Move NPC toward door if triggered
    if (triggered && npcEntity != entt::null && doorEntity != entt::null && 
        registry.all_of<PhysicsBodyRect>(npcEntity) && 
        registry.all_of<Position>(doorEntity) && 
        registry.all_of<Position>(npcEntity)) {
        
        auto& npcPhysics = registry.get<PhysicsBodyRect>(npcEntity);
        auto& doorPos = registry.get<Position>(doorEntity);
        auto& npcPos = registry.get<Position>(npcEntity);
        
        // Calculate direction vector from NPC to door
        p2d::Vec2f direction = {doorPos.x - npcPos.x, doorPos.y - npcPos.y};
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        
        // Only apply force if not already at the door
        if (length > 0.1f) {
        }
        else {
            if (current_target >= target_list.size()) { //} || current_target < 0) {        
            }
            else {
                current_target++; //ctdx;
                dialog_stage++;
                dialog_times = 1;
            }
        }

        direction.x = direction.x / length * 250.0f;
        direction.y = direction.y / length * 250.0f;
        
        npcPhysics.body->applyForce(direction);
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
        // Check if the entity is an NPC
        else if (registry.all_of<Id>(entity)) {
            auto &id = registry.get<Id>(entity);
            if (id.name == "npc") {
                auto position = registry.get<Position>(entity);
                
                if(dialog_times > 0) {
                    _js__show_alert(dialog_list[dialog_stage]);
                    dialog_times -= 1;
                }
            }
        }
    }
}

