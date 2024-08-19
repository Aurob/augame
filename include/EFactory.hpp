#pragma once
#include "../include/entt.hpp"
#include <vector>
#include <type_traits>
#include <functional>
#include <random>
#include <string>
#include "JSUtils.hpp"


void makePlayer(entt::registry& registry) {
        // Player
    float px = 0.0f, py = 25.0f, pz = 0.0f;
    float pw = 1.0f, ph = 1.0f;
    auto player = registry.create();
    registry.emplace<Player>(player);
    registry.emplace<Position>(player, Position{px, py, pz});
    registry.emplace<Shape>(player, Shape{pw, ph});    
    registry.emplace<Color>(player, Color{0, 0, 255, 1.0f});
    registry.emplace<Movement>(player,
        Movement{100, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
    registry.emplace<Moveable>(player);
    registry.emplace<Collidable>(player);
    registry.emplace<Teleportable>(player);
    registry.emplace<RenderPriority>(player, RenderPriority{1});
    registry.emplace<Test>(player, Test{"Player"});
    registry.emplace<RenderDebug>(player);
    // Add textures to the player
    // texture full width: 768, full height: 128
    // 6 columns, 1 row
    // std::vector<Textures> textureAlts;

    // const std::vector<std::string> actions = {"Idle", "Run"};
    // const std::vector<std::string> directions = {"Down", "Left", "Right", "Up"};
    // std::unordered_map<std::string, Textures> textureMap;

    // for (size_t actionIndex = 0; actionIndex < actions.size(); ++actionIndex) {
    //     for (size_t directionIndex = 0; directionIndex < directions.size(); ++directionIndex) {
    //         std::string textureName = std::to_string(actionIndex + 1) + "_Template_" + actions[actionIndex] + "_" + directions[directionIndex] + "-Sheet";
    //         std::vector<Texture> textures;
    //         for (int i = 0; i < 6; ++i) {
    //             textures.push_back({textureName, i / 6.0f, 0.0f, 1.0f / 6, 1.0f, 8, 8});
    //         }
    //         textureMap[actions[actionIndex] + "_" + directions[directionIndex]] = Textures{textures, 0};
    //     }
    // }

    // registry.emplace<TextureAlts>(player, TextureAlts{textureMap, "Idle_Down"});

    // TickAction to animate the player, increment the texture index of the current TextureAlts
    // registry.emplace<TickAction>(player, TickAction{
    //     [](entt::registry& registry, entt::entity entity) {
    //         auto& movement = registry.get<Movement>(entity);
    //         auto& textureAlts = registry.get<TextureAlts>(entity);
    //         auto& currentTextures = textureAlts.alts[textureAlts.current];
    //         currentTextures.current = (currentTextures.current + 1) % currentTextures.textures.size();
    //     }, 0.1f
    // });

    _player = player;
}

extern entt::entity _player;
/**
 * @brief Creates a room entity with optional door.
 */
entt::entity createRoom(entt::registry& registry, float x, float y, float width, float height, Color color, entt::entity parent = entt::null, bool createDoor = false, int priority = 52, int doorPosition = 0) {
    auto room = registry.create();
    registry.emplace<Position>(room, Position{x, y, 0});
    registry.emplace<Shape>(room, Shape{width, height});
    registry.emplace<Color>(room, color);
    registry.emplace<Interior>(room);
    registry.emplace<Collidable>(room);
    registry.emplace<Debug>(room, Debug{color});

    if (parent != entt::null) {
        registry.emplace<Inside>(room, Inside{parent});
        if (registry.all_of<RenderPriority>(parent)) {
            auto parentPriority = registry.get<RenderPriority>(parent).priority;
            registry.emplace<RenderPriority>(room, RenderPriority{parentPriority - 1});
        } else {
            registry.emplace<RenderPriority>(room, RenderPriority{priority});
        }
    } else {
        registry.emplace<RenderPriority>(room, RenderPriority{priority});
    }

    if (createDoor) {
        float doorX, doorY;
        
        float doorWidth = 1.5;
        float doorHeight = 0.25;
        
        // Randomly choose horizontal position along the top wall
        doorX = x + (rand() % static_cast<int>(width - doorWidth));
        doorY = y + height;

        auto door = registry.create();
        registry.emplace<Position>(door, Position{doorX, doorY - doorHeight, 0});
        registry.emplace<Shape>(door, Shape{doorWidth, doorHeight});
        Color doorColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 0.0f};
        registry.emplace<Color>(door, doorColor);
        registry.emplace<InteriorPortal>(door, InteriorPortal{room, parent});
        registry.emplace<Collidable>(door);
        registry.emplace<Debug>(door, Debug{doorColor});
        registry.emplace<RenderPriority>(door, RenderPriority{priority - 2});
        if (parent != entt::null) {
            registry.emplace<Inside>(door, Inside{parent});
        }
        // Associate component
        registry.emplace<Associated>(door, Associated{{room}});

        // Add a door texture entitiy using the Teleport Component with .disabled = true
        auto doorTexture = registry.create();
        registry.emplace<Position>(doorTexture, Position{doorX, doorY - 2, 0});
        registry.emplace<Shape>(doorTexture, Shape{1.5, 2});
        Color doorTextureColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
        registry.emplace<Color>(doorTexture, doorTextureColor);
        registry.emplace<Texture>(doorTexture, Texture{"door", 0, 0, 1, 1});
        registry.emplace<RenderPriority>(doorTexture, RenderPriority{-5});
        registry.emplace<Associated>(doorTexture, Associated{{door}});
    }

    // Interior Walls
    // Create the lower wall entity
    // it has no collidable and is Inside the room
    auto lower_wall = registry.create();
    registry.emplace<Position>(lower_wall, Position{x, y + height - height/4});
    registry.emplace<Shape>(lower_wall, Shape{width, height/4});
    Color lowerWallColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 0.9f};
    registry.emplace<Color>(lower_wall, lowerWallColor);
    registry.emplace<RenderPriority>(lower_wall, RenderPriority{-4});
    registry.emplace<Debug>(lower_wall, Debug{lowerWallColor});
    registry.emplace<Inside>(lower_wall, Inside{room});

    // Create the upper wall entity
    // it has no collidable and is Inside the room
    auto upper_wall = registry.create();
    registry.emplace<Position>(upper_wall, Position{x, y - height/4});
    registry.emplace<Shape>(upper_wall, Shape{width, height/4});
    Color upperWallColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
    registry.emplace<Color>(upper_wall, upperWallColor);
    registry.emplace<RenderPriority>(upper_wall, RenderPriority{-4});
    registry.emplace<Debug>(upper_wall, Debug{upperWallColor});
    registry.emplace<Inside>(upper_wall, Inside{room});

    // Create the outer front wall
    // it has collidable and is not Inside the room
    auto front_wall = registry.create();
    registry.emplace<Position>(front_wall, Position{x, y + height - height/4});
    registry.emplace<Shape>(front_wall, Shape{width, height/4});
    Color frontWallColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
    registry.emplace<Color>(front_wall, frontWallColor);
    registry.emplace<RenderPriority>(front_wall, RenderPriority{5});
    registry.emplace<Debug>(front_wall, Debug{frontWallColor});
    registry.emplace<Associated>(front_wall, Associated{{room}});
    
    // Create the upper part of the roof (1/4 height, slightly transparent)
    auto roof_upper = registry.create();
    registry.emplace<Position>(roof_upper, Position{x, y - height/4});
    registry.emplace<Shape>(roof_upper, Shape{width, height/4});
    Color roofUpperColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 0.7f};
    registry.emplace<Color>(roof_upper, roofUpperColor);
    registry.emplace<RenderPriority>(roof_upper, RenderPriority{-4});
    registry.emplace<Debug>(roof_upper, Debug{roofUpperColor});

    // Create the lower part of the roof (3/4 height, opaque)
    auto roof_lower = registry.create();
    registry.emplace<Position>(roof_lower, Position{x, y});
    registry.emplace<Shape>(roof_lower, Shape{width, height * 3/4});
    Color roofLowerColor{static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), static_cast<float>(rand() % 256), 1.0f};
    registry.emplace<Color>(roof_lower, roofLowerColor);
    registry.emplace<RenderPriority>(roof_lower, RenderPriority{-4});
    registry.emplace<Debug>(roof_lower, Debug{roofLowerColor});

    return room;
}

float rand_float(float min = 0.0f, float max = 1.0f) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return static_cast<float>(dis(gen));
}

void createRandomEntityInRoom(entt::registry& registry, entt::entity room) {
    auto roomPosition = registry.get<Position>(room);
    auto roomShape = registry.get<Shape>(room);

    float x = rand_float(roomPosition.x, roomPosition.x - roomShape.size.x);
    float y = rand_float(roomPosition.y, roomPosition.y - roomShape.size.y);
    float w = rand_float(0.5f, 1.5f);
    float h = rand_float(0.5f, 1.5f);

    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{x, y, 0});
    registry.emplace<Shape>(entity, Shape{w, h});
    
    Color color = {rand_float(0.0f, 255.0f), rand_float(0.0f, 255.0f), rand_float(0.0f, 255.0f), 1.0f};
    registry.emplace<Color>(entity, color);
    registry.emplace<Debug>(entity, Debug{color});
    registry.emplace<Collidable>(entity);
    registry.emplace<Moveable>(entity);
    registry.emplace<Movement>(entity, Movement{25, 110, Vector2f{0, 0}, Vector2f{0, 0}, .01, .01, .1});
    registry.emplace<Inside>(entity, Inside{room});
    registry.emplace<RenderPriority>(entity, RenderPriority{2});
    registry.emplace<Test>(entity, Test{"Random Entity in Room"});
    registry.emplace<Associated>(entity, Associated{{room}});
    
    registry.emplace<InteractionAction>(entity, InteractionAction{
        [](entt::registry& registry, entt::entity entity) {
            printf("Entity %d says: I'm a random entity in this room!\n", static_cast<int>(entity));
        }
    });
}

std::pair<Vector2f, Vector2f> createMazeInRoom(entt::registry& registry, entt::entity room) {
    auto roomPosition = registry.get<Position>(room);
    auto roomShape = registry.get<Shape>(room);

    const int cellSize = 1;
    int width = static_cast<int>(roomShape.size.x / cellSize);
    int height = static_cast<int>(roomShape.size.y / cellSize);

    std::vector<std::vector<bool>> maze(height, std::vector<bool>(width, false));
    std::stack<std::pair<int, int>> stack;
    int startY = height / 2;
    int startX = width / 2;
    stack.push({startY, startX});
    maze[startY][startX] = true;

    std::random_device rd;
    std::mt19937 gen(rd());

    while (!stack.empty()) {
        auto [y, x] = stack.top();
        std::vector<std::pair<int, int>> neighbors;

        for (const auto& [dy, dx] : std::vector<std::pair<int, int>>{{-2, 0}, {2, 0}, {0, -2}, {0, 2}}) {
            int ny = y + dy, nx = x + dx;
            if (ny >= 0 && ny < height && nx >= 0 && nx < width && !maze[ny][nx]) {
                neighbors.push_back({ny, nx});
            }
        }

        if (!neighbors.empty()) {
            auto [ny, nx] = neighbors[std::uniform_int_distribution<>(0, neighbors.size() - 1)(gen)];
            maze[ny][nx] = true;
            maze[y + (ny - y) / 2][x + (nx - x) / 2] = true;
            stack.push({ny, nx});
        } else {
            stack.pop();
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!maze[y][x]) {
                auto wall = registry.create();
                float wx = roomPosition.x + x * cellSize;
                float wy = roomPosition.y + y * cellSize;
                registry.emplace<Position>(wall, Position{wx, wy, 0});
                registry.emplace<Shape>(wall, Shape{static_cast<float>(cellSize), static_cast<float>(cellSize)});
                registry.emplace<Color>(wall, Color{100, 100, 100, 1.0f});
                registry.emplace<Debug>(wall, Debug{Color{100, 100, 100, 1.0f}});
                registry.emplace<Collidable>(wall);
                registry.emplace<Inside>(wall, Inside{room});
                registry.emplace<RenderPriority>(wall, RenderPriority{-1});
                registry.emplace<Associated>(wall, Associated{{room}});
            }
        }
    }

    // Return the xy of the start and end
    float startXPos = roomPosition.x + startX * cellSize;
    float startYPos = roomPosition.y + startY * cellSize;
    float endXPos = roomPosition.x + (width - 1) * cellSize;
    float endYPos = roomPosition.y + (height - 1) * cellSize;
    return std::make_pair(Vector2f{startXPos, startYPos}, Vector2f{endXPos, endYPos});
}

void runFactories(entt::registry& registry) {

    // get px, py, pw, ph
    auto playerPos = registry.get<Position>(_player);
    auto playerShape = registry.get<Shape>(_player);
    float px = playerPos.x;
    float py = playerPos.y;
    float pw = playerShape.size.x;
    float ph = playerShape.size.y;

    auto wall1 = registry.create();
    registry.emplace<Position>(wall1, Position{px - 10, py - 10, 0});
    registry.emplace<Shape>(wall1, Shape{10, 1, 2});
    registry.emplace<Color>(wall1, Color{100, 100, 100, 1.0f});
    registry.emplace<Debug>(wall1, Debug{Color{100, 100, 100, 1.0f}});
    registry.emplace<Collidable>(wall1);
    registry.emplace<RenderPriority>(wall1, RenderPriority{-1});



}

// /**
//  * @brief Creates a basic entity with position and shape components.
//  */
// entt::entity createBasicEntity(entt::registry& registry, float x, float y, float w, float h, float z = 0.0f) {
//     auto entity = registry.create();
//     registry.emplace<Position>(entity, Position{x, y, z});
//     registry.emplace<Shape>(entity, Shape{w, h});
//     return entity;
// }

// /**
//  * @brief Creates a debug entity with various components.
//  */
// entt::entity createDebugEntity(entt::registry& registry, float x = 0.0f, float y = 0.0f, float w = 0.0f, float h = 0.0f, bool random = false) {
//     if (random) {
//         x = rand_float(-60.0f, -50.0f);
//         y = rand_float(0.0f, 10.0f);
//         w = rand_float(0.2f, 0.7f);
//         h = rand_float(0.2f, 0.7f);
//     }

//     auto entity = createBasicEntity(registry, x, y, w, h);

//     Color color = {rand_float(0.0f, 255.0f), rand_float(0.0f, 255.0f), rand_float(0.0f, 255.0f), 1.0f};

//     registry.emplace<Color>(entity, color);
//     registry.emplace<Debug>(entity, Debug{color});
//     registry.emplace<Hoverable>(entity);
//     registry.emplace<Interactable>(entity);
//     registry.emplace<Linkable>(entity);

//     registry.emplace<Movement>(entity, Movement{
//         25, 110, Vector2f{0, 0}, Vector2f{0, 0}, .01, .01, .1
//     });
//     registry.emplace<Moveable>(entity);
//     registry.emplace<Collidable>(entity);

//     registry.emplace<InteractionAction>(entity, InteractionAction{
//         [](entt::registry& registry, entt::entity entity) {
//             const int N = 10;
//             std::string randomString;
//             randomString.reserve(N);
//             std::generate_n(std::back_inserter(randomString), N, []() { return static_cast<char>(rand_float(65, 90)); });
//             printf("Entity %d says: %s\n", static_cast<int>(entity), randomString.c_str());
//         }
//     });

//     return entity;
// }

// /**
//  * @brief Creates a player entity.
//  */
// entt::entity createPlayerEntity(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
//     auto entity = createBasicEntity(registry, xOffset, yOffset, 1.0f, 1.0f);
//     registry.emplace<Color>(entity, Color{0, 0, 255, 0.0f});
//     registry.emplace<Player>(entity);
//     registry.emplace<Visible>(entity);
//     registry.emplace<Movement>(entity, Movement{100, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
//     registry.emplace<Moveable>(entity);
//     registry.emplace<Debug>(entity, Debug{Color{0, 0, 255, 0.0f}});
//     registry.emplace<Collidable>(entity);
//     registry.emplace<Teleportable>(entity);
//     return entity;
// }

// /**
//  * @brief Creates a wall entity.
//  */
// void createWall(entt::registry& registry, float x, float y, float z, float width, float height, Color color, entt::entity parent=entt::null) {
//     auto wall = createBasicEntity(registry, x, y, width, height, z);
//     registry.emplace<Color>(wall, color);
//     registry.emplace<Debug>(wall, Debug{color});
//     registry.emplace<Collidable>(wall);
//     if (parent != entt::null) {
//         registry.emplace<Inside>(wall, Inside{parent});
//     }
    
// }



// void runFactories2(entt::registry& registry) {
//     // Large exterior building
//     auto largeBuilding = createRoom(registry, -100, -50, 100, 100, Color{100, 100, 100, 1.0f}, entt::null, false, 52);
//     // Define loop count
//     const int loopCount = 5;

//     // Nested interiors
//     auto currentInterior = largeBuilding;
//     float width = 90.0f;
//     float height = 90.0f;
//     float x = -95.0f;
//     float y = -45.0f;
//     for (int i = 0; i < loopCount; ++i) {
//         Color nestedColor = {
//             250.0f - i * 30.0f,
//             150.0f + i * 20.0f,
//             150.0f + i * 20.0f,
//             1.0f
//         };
//         int doorPosition = i % 4;  // Alternates between 0, 1, 2, 3
//         auto nestedInterior = createRoom(registry, x, y, width, height, nestedColor, currentInterior, true, 52 - (i + 1), doorPosition);
//         registry.emplace<Test>(nestedInterior, Test{"Nested " + std::to_string(i + 1)});
//         if (i < loopCount - 1) {
//             // Add basic wall slightly above the room door
//             float wallWidth = 3.0f;
//             float wallHeight = 0.5f;
//             float wallX = x - 0.8f;  // Align with door x-position
//             float wallY = y + height / 2;
//             Color nestedColor2 = {0, 0, 0, 1.0f};
//             auto wall = createDebugEntity(registry, wallX, wallY, wallWidth, wallHeight);
//             registry.emplace_or_replace<Color>(wall, nestedColor2);
//             registry.remove<Movement>(wall);
//             registry.emplace<Inside>(wall, Inside{nestedInterior});
//             registry.emplace<InteriorColliding>(wall, InteriorColliding{nestedInterior});
//             registry.emplace<RenderPriority>(wall, RenderPriority{2});
//         }

//         // Update for next iteration
//         currentInterior = nestedInterior;
//         width *= 0.5f;
//         height *= 0.5f;
//         x += 5.0f;
//         y += 5.0f;
//     }

//     // Add ~20 very small debug entities in the innermost room
//     for (int i = 0; i < 20; ++i) {
//         float size = rand_float(0.15f, 0.4f);
//         float debugX = rand_float(x, x + width/4 - size);
//         float debugY = rand_float(y, y + height/4 - size);
//         auto smallDebug = createDebugEntity(registry, debugX, debugY, size, size);
//         registry.emplace<Inside>(smallDebug, Inside{currentInterior});
//         registry.emplace<InteriorColliding>(smallDebug, InteriorColliding{currentInterior});
//         registry.emplace<RenderPriority>(smallDebug, RenderPriority{2});
//     }

//     // Player
//     float playerX = x + width / 4;
//     float playerY = y + height / 4;
//     _player = createPlayerEntity(registry, playerX, playerY);
//     registry.emplace<RenderPriority>(_player, RenderPriority{1});
//     registry.emplace<Inside>(_player, Inside{currentInterior});

// }
