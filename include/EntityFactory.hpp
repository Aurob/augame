#pragma once
#include "lib/entt.hpp"
#include "JSUtils.hpp"
#include "WebUtils.hpp"
#include "GameUtils.hpp"
#include "structs.hpp"
#include <random> 


extern entt::entity _player;
extern GameState gameState;

void makePlayer(entt::registry &registry);
void ensureCameraExists(entt::registry &registry);

void makePlayer(entt::registry &registry)
{
    bool defaultPlayer;

    // Player
    float px = 13.05, py = 12.45, pz = 5.0;
    float pw = 1.0f, ph = 1.0f, pd = 1.0f;

    entt::entity player = entt::null;
    auto player_view = registry.view<Player>();
    for (auto entity : player_view)
    {
        player = entity;
    }

    if(player == entt::null) {
        player = registry.create();
        registry.emplace<Position>(player);
        registry.emplace<Shape>(player);
        registry.emplace<PhysicsBodyRect>(player);
        registry.emplace<Collidable>(player);
        registry.emplace<Movement>(player, Movement{1000});

        defaultPlayer = true;
    }
    if (player == entt::null) return;

    registry.emplace_or_replace<Player>(player);
    registry.emplace_or_replace<Keys>(player);
    registry.emplace_or_replace<Cursor>(player);
    registry.emplace_or_replace<Rotation>(player, Rotation{0.0f, 0.0f, 0.0f, 1.0f});

    // Applies the animated player texture to the player entity (always add TextureAlts)
    if(!registry.any_of<Texture, TextureGroupPart, TextureAlts>(player))
    {
        // Add textures to the player
        std::vector<Textures> textureAlts;
        const std::vector<std::string> actions = {"Idle", "Run"};
        const std::vector<std::string> directions = {"Down", "Left", "Right", "Up"};
        std::unordered_map<std::string, Textures> textureMap;

        const int numFrames = 6;
        const float frameWidth = 1.0f / numFrames;
        const float frameHeight = 1.0f;
        const int textureWidth = 8;
        const int textureHeight = 8;

        for (size_t actionIndex = 0; actionIndex < actions.size(); ++actionIndex)
        {
            for (size_t directionIndex = 0; directionIndex < directions.size(); ++directionIndex)
            {
                std::string textureName = std::to_string(actionIndex + 1) + "_Template_" + actions[actionIndex] + "_" + directions[directionIndex] + "-Sheet";
                std::vector<Texture> textures;
                for (int i = 0; i < numFrames; ++i)
                {
                    textures.push_back({textureName, i * frameWidth, 0, frameWidth, frameHeight, textureWidth, textureHeight});
                }
                Texture metadata = {textureName, 0, 0, 0, 0, 0, 0};
                textureMap[actions[actionIndex] + "_" + directions[directionIndex]] = Textures{textures, 0, metadata};
            }
        }

        registry.emplace_or_replace<TextureAlts>(player, TextureAlts{textureMap, "Idle_Down"});
        
    
        // TickAction to animate the player textures
        registry.emplace_or_replace<TickAction>(player, TickAction{TickActions::animateTextures(), .19f});
    }

    _player = player;
    
    // Ensure camera component is assigned to player
    ensureCameraExists(registry);
}

void ensureCameraExists(entt::registry &registry) {
    entt::entity debugEntity = entt::null;
    // Check if any entity has a Camera component; if multiple, use the one with the highest Camera::priority value
    int maxPriority = std::numeric_limits<int>::min();
    auto cameraView = registry.view<Camera>();
    for (auto entity : cameraView) {
        const auto& camera = registry.get<Camera>(entity);
        if (camera.priority > maxPriority) {
            maxPriority = camera.priority;
            debugEntity = entity;
        }
    }
    // if (debugEntity != entt::null) {
    //     // Remove Camera component from all entities that have it
    //     auto cameraView = registry.view<Camera>();
    //     for (auto entity : cameraView) {
    //         registry.remove<Camera>(entity);
    //     }
    //     // Add Camera component to the debug entity
    //     registry.emplace_or_replace<Camera>(debugEntity);
    //     return;
    // }
    
    // If no camera exists, create one
    if(cameraView.empty()) {
        if(_player != entt::null) {
            // Assign camera to player
            registry.emplace<Camera>(_player);
        } else {
            // Create a default camera entity for stable defaults
            auto cameraEntity = registry.create();
            registry.emplace<Camera>(cameraEntity);
            registry.emplace<Position>(cameraEntity, Position{13.05f, 12.45f, 5.0f});
            registry.emplace<Shape>(cameraEntity, Shape{{1.0f, 1.0f, 1.0f}});
        }
    }
}

/**
 * @brief Runs the factory functions to create entities.
 */
void runFactories(entt::registry &registry)
{
    // Ensure camera exists for stable defaults
    ensureCameraExists(registry);
    
    addTickAction("watertest", TickActions::incrementShaderUniform(2, 10.0f), .09f);
    addTickAction("colortest", TickActions::incrementShaderUniform(2, .1f), .09f);
    addTickAction("maptest", TickActions::incrementShaderUniform(2, .01f), .09f);

}
