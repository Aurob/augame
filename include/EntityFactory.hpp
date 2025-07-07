#pragma once
#include "lib/entt.hpp"
#include "JSUtils.hpp"
#include "WebUtils.hpp"
#include "structs.hpp"
#include <random> 


extern entt::entity _player;
extern float gridSpacingValue;

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
    registry.emplace<Keys>(player);
    registry.emplace<Cursor>(player);
    if (!defaultPlayer) {
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
        
    
        // TickAction to animate the player, increment the texture index of the current TextureAlts
        registry.emplace_or_replace<TickAction>(player, TickAction{[](entt::registry &registry, entt::entity entity)
            {
                auto &textureAlts = registry.get<TextureAlts>(entity);
                auto &currentTextures = textureAlts.alts[textureAlts.current];
                currentTextures.current = (currentTextures.current + 1) % currentTextures.textures.size();
            },
            .19f});
    }

    _player = player;
}

void makeEffectEntity(entt::registry &registry, float _x, float _y, float _z, std::string name, entt::entity inside) {
    auto entity = registry.create();
    registry.emplace<Id>(entity, static_cast<int>(emscripten_get_now()), name);
    registry.emplace<Position>(entity, Position{_x, _y, _z});
    registry.emplace<Shape>(entity, Shape{1, 1, 1});
    registry.emplace<Color>(entity, Color{1, 1, 1});
    registry.emplace<RenderPriority>(entity, RenderPriority{2});
    // Add a rotation component with a random angle
    float randomAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 360.0f;
    registry.emplace<Rotation>(entity, Rotation{randomAngle, 0.0f, 0.0f, 1.0f});


    if(inside != entt::null) {
        registry.emplace<Inside>(entity, Inside{inside});
    }

    float scalex = 1;
    float scaley = 1;
    float x = 0;
    float y = 0;
    float w = 1;
    float h = 1;

    std::vector<Texture> hit1Textures;
    for (int i = 281; i <= 290; i++) {
        std::string textureName = "hit1" + std::to_string(i);
        hit1Textures.push_back({textureName, x, y, w, h, scalex, scaley});
    }

    registry.emplace<TextureAnimation>(entity, TextureAnimation{.interval=.15, .noloop=true});
    registry.emplace<Textures>(entity, Textures{hit1Textures, 0});

        // TickAction to animate the player, increment the texture index of the current TextureAlts
    registry.emplace<TickAction>(entity, TickAction{[](entt::registry &registry, entt::entity entity)
    {
        if (registry.all_of<Textures>(entity)) {
            auto& textures = registry.get<Textures>(entity);
            if (textures.current >= textures.textures.size()-1) {
                registry.emplace<Flag>(entity, "delete");
            }
        }
    },
    1.0f});

}

/**
 * @brief Runs the factory functions to create entities.
 */
void runFactories(entt::registry &registry)
{
    // entt::entity test = registry.create();
    // registry.emplace<Text>(test, Text{"Hello World"});
    // registry.emplace<Id>(test, Id{.name="menu_entity"});
    // registry.emplace<Position>(test);
    // registry.emplace<Shape>(test);
    // registry.emplace<Visible>(test);
  
}
