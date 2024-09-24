#pragma once
#include "../include/entt.hpp"
#include "JSUtils.hpp"
#include <random> 


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
  
    // query all the Id.name = "p2key", choose 5 random ones to give the PuzzlePiece component
    auto view = registry.view<Id>();
    std::vector<entt::entity> p2keyEntities;

    for (auto entity : view)
    {
        const auto& id = view.get<Id>(entity).name;
        if (id.find("p2key2") == 0)
        {
            p2keyEntities.push_back(entity);
        }
    }

    std::shuffle(p2keyEntities.begin(), p2keyEntities.end(), std::mt19937{std::random_device{}()});
    std::vector<entt::entity> selectedEntities(p2keyEntities.begin(), p2keyEntities.begin() + std::min<size_t>(5, p2keyEntities.size()));

    std::array<Color, 5> colors = {Color{1.0f, 0.0f, 0.0f}, Color{0.0f, 0.0f, 1.0f}, Color{0.0f, 1.0f, 0.0f}, Color{1.0f, 1.0f, 0.0f}, Color{1.0f, 0.5f, 0.0f}};

    for (size_t i = 0; i < selectedEntities.size(); ++i)
    {
        auto entity = selectedEntities[i];
        registry.emplace_or_replace<PuzzlePiece>(entity, PuzzlePiece{false});
        registry.emplace_or_replace<Color>(entity, colors[i]);
    }


    auto puzzleEntity = registry.create();
    registry.emplace<Puzzle>(puzzleEntity, Puzzle{selectedEntities});

    // get Id.name= npctest
    auto view3 = registry.view<Id>();
    for (auto entity : view3)
    {
        const auto& id = view3.get<Id>(entity).name;
   
        // if explode1
        if (id.find("explode1") == 0)
        {
            // Add a tick action to move the entity around
            registry.emplace_or_replace<TickAction>(entity, TickAction{[](entt::registry &registry, entt::entity entity)
                {
                    auto &movement = registry.get<Movement>(entity);
                    float time = SDL_GetTicks() / 1000.0f; // Get time in seconds
                    movement.velocity.x += std::sin(time) * std::cos(time) * 0.5f;
                    movement.velocity.y -= std::cos(time) * 0.5f;
                }, .1f});

            // Add a collision action to create 10 .1 shape entities at the entity position with all components
            auto &interactionActions = registry.get_or_emplace<InteractionActions>(entity);
            interactionActions.actions.push_back(InteractionAction{[](entt::registry &registry, entt::entity entity, std::optional<entt::entity> optEntity)
                {
                    printf("explode1\n");
                    auto &position = registry.get<Position>(entity);
                    auto &color = registry.get<Color>(entity);
                    auto &movement = registry.get<Movement>(entity);
                    auto &renderPriority = registry.get<RenderPriority>(entity);
                    auto &inside = registry.get<Inside>(entity);

                    for (int i = 0; i < 40; ++i)
                    {
                        auto newEntity = registry.create();
                        registry.emplace<Position>(newEntity, Position{
                            position.x + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
                            position.y + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
                            position.z
                        });
                        registry.emplace<Shape>(newEntity, Shape{0.05f, 0.05f, 0.05f});
                        registry.emplace<Color>(newEntity, color);
                        registry.emplace<RenderPriority>(newEntity, renderPriority);
                        registry.emplace<Inside>(newEntity, inside);

                        // Add a tick action to slowly reduce opacity to 0 and then destroy
                        registry.emplace<TickAction>(newEntity, TickAction{
                            [](entt::registry &registry, entt::entity entity) {
                                auto &color = registry.get<Color>(entity);
                                color.a -= 0.01f; // Reduce opacity
                                if (color.a <= 0.0f) {
                                    registry.destroy(entity); // Destroy entity when fully transparent
                                }
                            }, 0.1f // Interval for the tick action
                        });
                    }

                    // Create a unique item with random color and Tone
                    auto uniqueEntity = registry.create();
                    registry.emplace<Position>(uniqueEntity, Position{
                        position.x + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
                        position.y + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f,
                        position.z
                    });
                    registry.emplace<Shape>(uniqueEntity, Shape{0.5f, 0.5f, 0.5f});
                    registry.emplace<Color>(uniqueEntity, Color{
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        1.0f
                    });
                    registry.emplace<Tone>(uniqueEntity, Tone{
                        "C4", // Random note can be assigned here
                        "4n", // Duration
                        "sine", // Type
                        -10.0f, // Volume
                    });
                    printf("uniqueEntity\n");
                    registry.emplace<RenderPriority>(uniqueEntity, renderPriority);
                    registry.emplace<Inside>(uniqueEntity, inside);
                    registry.emplace<Collidable>(uniqueEntity);
                    registry.emplace<Hoverable>(uniqueEntity);
                    registry.emplace<Interactable>(uniqueEntity);
                    registry.emplace<UIElement>(uniqueEntity, UIElement{"Winner!", true, 0, 0});

                    // Add an action to play the note
                    auto &interactionActions = registry.get_or_emplace<InteractionActions>(uniqueEntity);
                    interactionActions.actions.push_back(InteractionAction{[](entt::registry &registry, entt::entity entity, std::optional<entt::entity> optEntity)
                        {
                            printf("Tone\n");
                            auto &tone = registry.get<Tone>(entity);
                            tone.playing = true;
                        }, false});

                    // Destroy the entity
                    registry.destroy(entity);
                }, false});
        }
    }



    auto view2 = registry.view<Tone>();
    for (auto entity : view2)
    {
        auto &interactionActions = registry.get_or_emplace<InteractionActions>(entity);
        interactionActions.actions.push_back(InteractionAction{[](entt::registry &registry, entt::entity entity, std::optional<entt::entity> optEntity)
            {
                if (registry.get<Hoverable>(entity).duration == 0) {
                    printf("Tone\n");
                    auto &tone = registry.get<Tone>(entity);
                    tone.playing = true;

                    if (registry.all_of<PuzzlePiece>(entity))
                    {
                        auto &puzzlePiece = registry.get<PuzzlePiece>(entity);
                        auto parentPuzzle = registry.view<Puzzle>().front();
                        auto &puzzle = registry.get<Puzzle>(parentPuzzle);

                        if (!puzzle.solved)
                        {
                            puzzlePiece.active = true;
                        }
                    }
                }
            }, false});
    }


}
