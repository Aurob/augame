#pragma once
#include "../include/entt.hpp"
#include <vector>
#include <type_traits>
#include <functional>
#include <random>
#include <string>
#include "JSUtils.hpp"
#include "AltEFactory.hpp"

extern entt::entity _player;

void makePlayer(entt::registry &registry)
{
    // Player
    float px = 13.05, py = 12.45, pz = 5.0;
    float pw = 1.0f, ph = 1.0f, pd = 1.0f;
    auto player = registry.create();
    registry.emplace_or_replace<Player>(player);
    registry.emplace_or_replace<Position>(player, Position{px, py, pz});
    registry.emplace_or_replace<Shape>(player, Shape{{pw, ph, pd}});
    registry.emplace_or_replace<Color>(player, Color{0, 0, 255, 0.0f});
    registry.emplace_or_replace<Movement>(player, Movement{75, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
    registry.emplace_or_replace<Moveable>(player);
    registry.emplace_or_replace<Collidable>(player);
    registry.emplace_or_replace<Teleportable>(player);
    registry.emplace_or_replace<RenderPriority>(player, RenderPriority{0});
    registry.emplace_or_replace<Test>(player, Test{"Player"});
    // registry.emplace_or_replace<RenderDebug>(player);
    registry.emplace<Keys>(player);

    // place inside Interior with Id of 0, look up hy compomnent Id
    // auto view = registry.view<Id>();
    // entt::entity interiorEntity = entt::null;
    // for (auto entity : view)
    // {
    //     if (view.get<Id>(entity).id == 6)
    //     {
    //         interiorEntity = entity;
    //         break;
    //     }
    // }
    // registry.emplace_or_replace<Inside>(player, Inside{interiorEntity});

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
 * @brief Creates a room entity with optional door.
 */
entt::entity createRoom(entt::registry &registry, float x, float y, float z, float width, float height,
                        Color color, entt::entity parent = entt::null, bool createDoor = false, int priority = 0, int doorPosition = 0)
{
    auto room = registry.create();
    registry.emplace_or_replace<Position>(room, Position{x, y, z});
    registry.emplace_or_replace<Shape>(room, Shape{width, height, 1});
    registry.emplace_or_replace<Color>(room, color);
    registry.emplace_or_replace<Interior>(room);
    registry.emplace_or_replace<Collidable>(room);
    registry.emplace_or_replace<Debug>(room, Debug{color});

    int basePriority = registry.get<RenderPriority>(_player).priority;
    registry.emplace_or_replace<RenderPriority>(room, RenderPriority{basePriority - 2});

    if (parent != entt::null)
    {
        registry.emplace_or_replace<Inside>(room, Inside{parent});
    }

    auto createWall = [&](float posX, float posY, float posZ, float shapeWidth,
                          float shapeHeight, float colorAlpha, entt::entity inside, bool isCollidable = false, int prio = 0, Color wallColor = Color{0, 0, 0, 1.0f})
    {
        wallColor.a = colorAlpha; // Override wallColor alpha with colorAlpha
        auto wall = registry.create();
        registry.emplace_or_replace<Position>(wall, Position{posX, posY, posZ});
        registry.emplace_or_replace<Shape>(wall, Shape{shapeWidth, shapeHeight, 1});
        registry.emplace_or_replace<Color>(wall, wallColor);
        registry.emplace_or_replace<RenderPriority>(wall, RenderPriority{prio});
        registry.emplace_or_replace<Debug>(wall, Debug{wallColor});
        if (inside != entt::null)
        {
            registry.emplace_or_replace<Inside>(wall, Inside{inside});
        }
        if (isCollidable)
        {
            registry.emplace_or_replace<Collidable>(wall);
        }
        registry.emplace_or_replace<Associated>(wall, Associated{{room}});

        return wall;
    };

    // back wall outer
    createWall(x, y - (height / 4), z, width, height / 4, 1.0f, parent, false, basePriority + priority + 1, color); // back wall
    // front wall outer
    Color darkerColor = {color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, color.a};
    createWall(x, y + height - (height / 4), z, width, height / 4, 1.0f, parent, false, basePriority + priority - 1, darkerColor); // front wall

    // back wall inner
    createWall(x, y - (height / 4), z, width, height / 4, 1.0f, room, false, basePriority + priority + 1, darkerColor); // back wall
    // front wall inner
    auto front_inner = createWall(x, y + height - (height / 4), z, width, height / 4, 1.0f, room, false, basePriority + priority + 1, darkerColor); // front wall
    // Create the lower part of the roof (3/4 height, opaque)
    auto roof_lower = registry.create();
    registry.emplace_or_replace<Position>(roof_lower, Position{x, y, z});
    registry.emplace_or_replace<Shape>(roof_lower, Shape{width, height * 3 / 4});
    registry.emplace_or_replace<Color>(roof_lower, color);
    registry.emplace_or_replace<RenderPriority>(roof_lower, RenderPriority{basePriority + priority + 2});
    registry.emplace_or_replace<Debug>(roof_lower, Debug{color});
    if (parent != entt::null)
    {
        registry.emplace_or_replace<Inside>(roof_lower, Inside{parent});
    }

    return room;
}

/**
 * @brief Generates a random float between min and max.
 */
float rand_float(float min = 0.0f, float max = 1.0f)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return static_cast<float>(dis(gen));
}

/**
 * @brief Runs the factory functions to create entities.
 */
void runFactories(entt::registry &registry)
{
    // testing_v2();
}

// std::vector<entt::entity> rooms;
// auto outerRoom = createRoom(registry, -20, -20, 0.1f, 25, 25, Color{180, 180, 180, 1.0f}, entt::null, true, 0, rand() % 4 + 1);
// rooms.push_back(outerRoom);

// // Create a slightly smaller room inside the first room
// float innerWidth = 15;
// float innerHeight = 15;
// float innerX = -20 + (25 - innerWidth) / 2;  // Center the inner room horizontally
// float innerY = -20 + (20 - innerHeight) / 2;  // Center the inner room vertically
// auto innerRoom = createRoom(registry, innerX, innerY, 0.0f,
//     innerWidth, innerHeight, Color{200, 200, 200, 1.0f}, outerRoom, true, -1, rand() % 4 + 1);
// rooms.push_back(innerRoom);

// // create another room above the first room, not inside, but the same size and position as the inside room
// auto aboveRoom = createRoom(registry, innerX, -20 + (20 - innerHeight) / 2, 25/4,
//     innerWidth, innerHeight, Color{200, 200, 200, 1.0f}, entt::null, true, 10);
// rooms.push_back(aboveRoom);

// // add 10 entities into the above room
// for (int i = 0; i < 10; ++i) {
//     float x = innerX + rand_float(0, innerWidth);
//     float y = -20 + (20 - innerHeight) / 2 + rand_float(0, innerHeight);
//     auto entity = registry.create();
//     registry.emplace_or_replace<Position>(entity, Position{x, y, 25/4});
//     registry.emplace_or_replace<Shape>(entity, Shape{1, 1});
//     registry.emplace_or_replace<Color>(entity, Color{255, 0, 0, 1.0f});
//     registry.emplace_or_replace<RenderPriority>(entity, RenderPriority{playerPriority - 1});
//     registry.emplace_or_replace<Debug>(entity, Debug{Color{0, 0, 0, 1.0f}});
//     registry.emplace_or_replace<Inside>(entity, Inside{aboveRoom});
//     registry.emplace_or_replace<Collidable>(entity);
//     registry.emplace_or_replace<Moveable>(entity);
//     registry.emplace_or_replace<Movement>(entity, Movement{100, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
//     registry.emplace_or_replace<Test>(entity, Test{"test entity"});
// }

// float doorX, doorY;

// float doorWidth = 1.5;
// float doorHeight = 0.25;

// // Randomly choose horizontal position along the top wall
// doorX = -15;
// doorY = -20 + 30;

// auto door = registry.create();
// registry.emplace_or_replace<Position>(door, Position{doorX, doorY - doorHeight-1, 0});
// registry.emplace_or_replace<Shape>(door, Shape{doorWidth, doorHeight, 1});
// registry.emplace_or_replace<Debug>(door, Debug{Color{0, 0, 0, 1.0f}});
// registry.emplace_or_replace<RenderPriority>(door, RenderPriority{playerPriority - 1});
// registry.emplace<Test>(door, Test{"Door"});

// registry.emplace_or_replace<InteriorPortal>(door, InteriorPortal{rooms[0], entt::null});
// registry.emplace_or_replace<Collidable>(door);
// // Associate component

// // Add a door texture entitiy using the Teleport Component with .disabled = true
// auto doorTexture = registry.create();
// registry.emplace_or_replace<Position>(doorTexture, Position{doorX, doorY - 2});
// registry.emplace_or_replace<Shape>(doorTexture, Shape{1.5, 2});
// Color doorTextureColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
// registry.emplace_or_replace<Color>(doorTexture, doorTextureColor);
// registry.emplace_or_replace<Texture>(doorTexture, Texture{"door", 0, 0, 1, 1});
// registry.emplace<InteriorPortalTexture>(doorTexture, InteriorPortalTexture{door});
// registry.emplace_or_replace<RenderPriority>(doorTexture, RenderPriority{2});
// registry.emplace_or_replace<Associated>(doorTexture, Associated{{door}});

// // Add a door to the interior room
// doorX = innerX + innerWidth/2;
// doorY = innerY + innerHeight - 2;

// auto innerDoor = registry.create();
// registry.emplace_or_replace<Position>(innerDoor, Position{doorX, doorY});
// registry.emplace_or_replace<Shape>(innerDoor, Shape{1.5, 2, 1});
// registry.emplace_or_replace<InteriorPortal>(innerDoor, InteriorPortal{innerRoom, outerRoom});
// registry.emplace_or_replace<Collidable>(innerDoor);
// registry.emplace_or_replace<Inside>(innerDoor, Inside{outerRoom});

// // Add a door texture entitiy
// auto innerDoorTexture = registry.create();
// registry.emplace_or_replace<Position>(innerDoorTexture, Position{doorX, doorY});
// registry.emplace_or_replace<Shape>(innerDoorTexture, Shape{1, 2, 1});
// Color innerDoorTextureColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
// registry.emplace_or_replace<Color>(innerDoorTexture, innerDoorTextureColor);
// registry.emplace_or_replace<Texture>(innerDoorTexture, Texture{"door", 0, 0, 1, 1});
// registry.emplace<InteriorPortalTexture>(innerDoorTexture, InteriorPortalTexture{innerDoor});
// registry.emplace_or_replace<RenderPriority>(innerDoorTexture, RenderPriority{3});
// registry.emplace_or_replace<Associated>(innerDoorTexture, Associated{{innerDoor}});
// registry.emplace_or_replace<Inside>(innerDoorTexture, Inside{outerRoom});

// // Add another to inside room, but place inside the inside room at the center and make it go to the above room
// doorX = innerX + innerWidth / 2 - doorWidth / 2;
// doorY = innerY + 1;
// auto innerDoor2 = registry.create();
// registry.emplace_or_replace<Position>(innerDoor2, Position{doorX, doorY});
// registry.emplace_or_replace<Shape>(innerDoor2, Shape{doorWidth, doorHeight, 1});
// registry.emplace_or_replace<InteriorPortal>(innerDoor2, InteriorPortal{aboveRoom, innerRoom});
// registry.emplace_or_replace<Collidable>(innerDoor2);
// registry.emplace_or_replace<Inside>(innerDoor2, Inside{innerRoom});

// auto innerDoorTexture2 = registry.create();
// registry.emplace_or_replace<Position>(innerDoorTexture2, Position{doorX, doorY});
// registry.emplace_or_replace<Shape>(innerDoorTexture2, Shape{2, 2, 1});
// Color innerDoorTextureColor2{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
// registry.emplace_or_replace<Color>(innerDoorTexture2, innerDoorTextureColor2);
// registry.emplace_or_replace<Textures>(innerDoorTexture2, Textures{{{"ladder_up", 0, 0, 1, 1}, {"ladder_down", 0, 0, 1, 1}}, 0});
// registry.emplace<InteriorPortalTexture>(innerDoorTexture2, InteriorPortalTexture{innerDoor2});
// registry.emplace_or_replace<RenderPriority>(innerDoorTexture2, RenderPriority{playerPriority - 1});
// registry.emplace_or_replace<Associated>(innerDoorTexture2, Associated{{innerDoor2}});
// registry.emplace_or_replace<Inside>(innerDoorTexture2, Inside{innerRoom});
// registry.emplace_or_replace<Associated>(innerDoor2, Associated{{innerDoorTexture2}});
