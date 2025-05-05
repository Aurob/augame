
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
std::unordered_map<std::string, int> maxInstances = {{"hit1", 100}};      // Maps effect name to max number of concurrent instances
std::unordered_map<std::string, float> timeoutIntervals = {{"hit1", 0.01f}}; // Maps effect name to timeout interval in seconds
std::unordered_map<std::string, int> currentInstances = {{"hit1", 0}};   // Tracks current number of instances per effect
std::unordered_map<std::string, float> lastCreationTime = {{"hit1", 0.0f}}; // Tracks when each effect was last created

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

        // action.time += deltaTime*5;
        // if (action.time >= interval) {
        //     action.action(registry, entity);
        //     action.time = 0.0f; // Reset the time after triggering the action
        // }
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
        
        if(flag.name == "delete") {
            // Check if entity has Effect component and decrement the counter if applicable
            if (registry.all_of<Effect>(e)) {
                auto idView = registry.try_get<Id>(e);
                if (idView && currentInstances.find(idView->name) != currentInstances.end()) {
                    // Decrement the counter for this effect type
                    currentInstances[idView->name]--;
                }
            }
            
            registry.destroy(e);
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
        printf("3\n");

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

                if(registry.all_of<Text>(entity)) {
                    auto text = registry.get<Text>(entity);
                    _js__speak(text.text);
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
                // Check if we can create a new effect based on limits
                std::string effectName = "hit1";
                float currentTime = emscripten_get_now() / 1000.0f;
                
                // Check if we've exceeded the maximum number of instances
                if(currentInstances[effectName] < maxInstances[effectName]) {
                    // Check if enough time has passed since the last creation
                    if(currentTime - lastCreationTime[effectName] >= timeoutIntervals[effectName]) {
                    
                        makeEffectEntity(registry, cursorScreenX, cursorScreenY, 0, effectName, playerInterior);
                        currentInstances[effectName]++;
                        lastCreationTime[effectName] = currentTime;
                    }
                }
                // // Add effect entity with TickAction to destroy itself
                // auto effect = registry.create();
                // auto idview = registry.view<Id>();
                // for(auto e : idview) {
                //     auto id = idview.get<Id>(e);
                //     if(id.name=="hit1") {
                //         // Get player cursor position
                //         auto& cursor = registry.get<Cursor>(_player);
                        
                //         // Set the hit effect position to the cursor position
                //         if (registry.all_of<Position>(e)) {
                //             Position& position = registry.get<Position>(e);
                //             position.x = cursor.position.x;
                //             position.y = cursor.position.y;
                //         }
                        
                //         // Reset animation to first frame
                //         if (registry.all_of<Textures>(e)) {
                //             auto& textures = registry.get<Textures>(e);
                //             textures.current = 1;
                //         }
                //     }
                // }

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

