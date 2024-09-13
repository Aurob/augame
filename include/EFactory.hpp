#pragma once
#include "../include/entt.hpp"
#include "JSUtils.hpp"

extern entt::entity _player;

void makePlayer(entt::registry &registry)
{
    // Player
    float px = 13.05, py = 12.45, pz = 5.0;
    float pw = 1.0f, ph = 1.0f, pd = 1.0f;
    auto player = registry.create();

    auto player_id_view = registry.view<Id>();
    for (auto entity : player_id_view)
    {
        if (player_id_view.get<Id>(entity).name == "player")
        {
            player = entity;
            break;
        }
    }

    registry.emplace_or_replace<Player>(player);
    registry.emplace<Keys>(player);
    registry.emplace<Cursor>(player);

    auto view = registry.view<Tone>(entt::exclude<InteractionAction>);
    for (auto entity : view)
    {
        registry.emplace_or_replace<InteractionAction>(entity, InteractionAction{[](entt::registry &registry, entt::entity entity, std::optional<entt::entity> optEntity)
            {
                if (registry.get<Hoverable>(entity).duration == 0) {
                    auto &tone = registry.get<Tone>(entity);
                    tone.playing = true;
                }
            }, false});
    }

    

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
        0.12f});

    _player = player;
}


/**
 * @brief Runs the factory functions to create entities.
 */
void runFactories(entt::registry &registry)
{
    auto key2 = registry.create();
    registry.emplace<Position>(key2, Position{1, -5, 0});
    registry.emplace<Shape>(key2, Shape{.25, .25, .1});
    registry.emplace<Color>(key2, Color{250, 255, 0, 1.0f});
    registry.emplace<RenderPriority>(key2, RenderPriority{0});
    registry.emplace<Debug>(key2, Debug{Color{0, 1.0, 0, 1.0f}});
    registry.emplace<Test>(key2, Test{"key entity"});
    registry.emplace<Interactable>(key2, Interactable{.radius = 1.0f});
    registry.emplace<Collidable>(key2, Collidable{.ignorePlayer = true});
    registry.emplace<Hoverable>(key2);
    registry.emplace<UIElement>(key2, UIElement{"Key", false});
    registry.emplace_or_replace<InteractionAction>(key2, InteractionAction{[](entt::registry &registry, entt::entity entity, std::optional<entt::entity> opt_entity) {
        auto& uiElement = registry.get<UIElement>(entity);
        uiElement.visible = !uiElement.visible;
    }, true});

    auto view = registry.view<Id>();
    entt::entity room3 = entt::null;
    for (auto entity : view) {
        if (view.get<Id>(entity).name == "room3") {
            room3 = entity;
            break;
        }
    }

    if (room3 != entt::null) {
        registry.emplace_or_replace<Inside>(key2, Inside{room3});
    }

}
