#pragma once
#include "../include/entt.hpp"
#include <vector>
#include <type_traits>
#include <functional>
#include <random>
#include <string>

extern entt::entity _player;

// Random number generator function
float rand_float(float min = 0.0f, float max = 1.0f) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return static_cast<float>(dis(gen));
}

/**
 * @brief Creates a basic entity with position and shape components.
 */
entt::entity createBasicEntity(entt::registry& registry, float x, float y, float w, float h, float z = 0.0f) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{x, y, z});
    registry.emplace<Shape>(entity, Shape{w, h});
    return entity;
}

/**
 * @brief Creates a debug entity with various components.
 */
entt::entity createDebugEntity(entt::registry& registry, float x = 0.0f, float y = 0.0f, float w = 0.0f, float h = 0.0f, bool random = false) {
    if (random) {
        x = rand_float(-60.0f, -50.0f);
        y = rand_float(0.0f, 10.0f);
        w = rand_float(0.2f, 0.7f);
        h = rand_float(0.2f, 0.7f);
    }

    auto entity = createBasicEntity(registry, x, y, w, h);

    Color color = {rand_float(0.0f, 255.0f), rand_float(0.0f, 255.0f), rand_float(0.0f, 255.0f), 1.0f};

    registry.emplace<Color>(entity, color);
    registry.emplace<Debug>(entity, Debug{color});
    registry.emplace<Hoverable>(entity);
    registry.emplace<Interactable>(entity);
    registry.emplace<Linkable>(entity);

    registry.emplace<Movement>(entity, Movement{
        25, 110, Vector2f{0, 0}, Vector2f{0, 0}, .01, .01, .1
    });
    registry.emplace<Moveable>(entity);
    registry.emplace<Collidable>(entity);

    registry.emplace<InteractionAction>(entity, InteractionAction{
        [](entt::registry& registry, entt::entity entity) {
            const int N = 10;
            std::string randomString;
            randomString.reserve(N);
            std::generate_n(std::back_inserter(randomString), N, []() { return static_cast<char>(rand_float(65, 90)); });
            printf("Entity %d says: %s\n", static_cast<int>(entity), randomString.c_str());
        }
    });

    return entity;
}

/**
 * @brief Creates a player entity.
 */
entt::entity createPlayerEntity(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    auto entity = createBasicEntity(registry, xOffset, yOffset, 1.0f, 1.0f);
    registry.emplace<Color>(entity, Color{0, 0, 255, 0.0f});
    registry.emplace<Player>(entity);
    registry.emplace<Visible>(entity);
    registry.emplace<Movement>(entity, Movement{100, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
    registry.emplace<Moveable>(entity);
    registry.emplace<Debug>(entity, Debug{Color{0, 0, 255, 0.0f}});
    registry.emplace<Collidable>(entity);
    registry.emplace<Teleportable>(entity);
    return entity;
}

/**
 * @brief Creates a wall entity.
 */
void createWall(entt::registry& registry, float x, float y, float z, float width, float height, Color color, entt::entity parent=entt::null) {
    auto wall = createBasicEntity(registry, x, y, width, height, z);
    registry.emplace<Color>(wall, color);
    registry.emplace<Debug>(wall, Debug{color});
    registry.emplace<Collidable>(wall);
    if (parent != entt::null) {
        registry.emplace<Inside>(wall, Inside{parent});
    }
    
}

/**
 * @brief Creates a room entity with optional door.
 */
entt::entity createRoom(entt::registry& registry, float x, float y, float width, float height, Color color, entt::entity parent = entt::null, bool createDoor = false, int priority = 52, int doorPosition = 0) {
    auto room = createBasicEntity(registry, x, y, width, height);
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
        switch (doorPosition) {
            case 0: // Left
                doorX = x - 0.8f;
                doorY = y + height / 2 - 1;
                break;
            case 1: // Top
                doorX = x + width / 2 - 0.5f;
                doorY = y + height - 0.8f;
                break;
            case 2: // Right
                doorX = x + width - 0.2f;
                doorY = y + height / 2 - 1;
                break;
            case 3: // Bottom
                doorX = x + width / 2 - 0.5f;
                doorY = y - 1.2f;
                break;
            default:
                doorX = x - 0.8f;
                doorY = y + height / 2 - 1;
        }
        auto door = createBasicEntity(registry, doorX, doorY, 1, 2);
        registry.emplace<Color>(door, Color{0, 255, 0, 1.0f});
        registry.emplace<InteriorPortal>(door, InteriorPortal{room, parent});
        registry.emplace<Collidable>(door);
        registry.emplace<Debug>(door, Debug{Color{0, 255, 0, 1.0f}});
        registry.emplace<RenderPriority>(door, RenderPriority{priority - 2});
        if (parent != entt::null) {
            registry.emplace<Inside>(door, Inside{parent});
        }
    }

    return room;
}

void runFactories2(entt::registry& registry) {
    // Large exterior building
    auto largeBuilding = createRoom(registry, -100, -50, 100, 100, Color{100, 100, 100, 1.0f}, entt::null, false, 52);
    // Define loop count
    const int loopCount = 5;

    // Nested interiors
    auto currentInterior = largeBuilding;
    float width = 90.0f;
    float height = 90.0f;
    float x = -95.0f;
    float y = -45.0f;
    for (int i = 0; i < loopCount; ++i) {
        Color nestedColor = {
            250.0f - i * 30.0f,
            150.0f + i * 20.0f,
            150.0f + i * 20.0f,
            1.0f
        };
        int doorPosition = i % 4;  // Alternates between 0, 1, 2, 3
        auto nestedInterior = createRoom(registry, x, y, width, height, nestedColor, currentInterior, true, 52 - (i + 1), doorPosition);
        registry.emplace<Test>(nestedInterior, Test{"Nested " + std::to_string(i + 1)});
        if (i < loopCount - 1) {
            // Add basic wall slightly above the room door
            float wallWidth = 3.0f;
            float wallHeight = 0.5f;
            float wallX = x - 0.8f;  // Align with door x-position
            float wallY = y + height / 2;
            Color nestedColor2 = {0, 0, 0, 1.0f};
            auto wall = createDebugEntity(registry, wallX, wallY, wallWidth, wallHeight);
            registry.emplace_or_replace<Color>(wall, nestedColor2);
            registry.remove<Movement>(wall);
            registry.emplace<Inside>(wall, Inside{nestedInterior});
            registry.emplace<InteriorColliding>(wall, InteriorColliding{nestedInterior});
            registry.emplace<RenderPriority>(wall, RenderPriority{2});
        }

        // Update for next iteration
        currentInterior = nestedInterior;
        width *= 0.5f;
        height *= 0.5f;
        x += 5.0f;
        y += 5.0f;
    }

    // Add ~20 very small debug entities in the innermost room
    for (int i = 0; i < 20; ++i) {
        float size = rand_float(0.15f, 0.4f);
        float debugX = rand_float(x, x + width/4 - size);
        float debugY = rand_float(y, y + height/4 - size);
        auto smallDebug = createDebugEntity(registry, debugX, debugY, size, size);
        registry.emplace<Inside>(smallDebug, Inside{currentInterior});
        registry.emplace<InteriorColliding>(smallDebug, InteriorColliding{currentInterior});
        registry.emplace<RenderPriority>(smallDebug, RenderPriority{2});
    }

    // Player
    float playerX = x + width / 4;
    float playerY = y + height / 4;
    _player = createPlayerEntity(registry, playerX, playerY);
    registry.emplace<RenderPriority>(_player, RenderPriority{1});
    registry.emplace<Inside>(_player, Inside{currentInterior});

}

void runFactories(entt::registry& registry) {
    // Player
    float px = 35.0f, py = 35.0f, pz = 0.0f;
    float pw = 1.0f, ph = 1.0f;
    auto player = registry.create();
    registry.emplace<Player>(player);
    registry.emplace<Position>(player, Position{px, py, pz});
    registry.emplace<Shape>(player, Shape{pw, ph});    
    registry.emplace<Color>(player, Color{0, 0, 255, 0.0f});
    registry.emplace<Movement>(player,
        Movement{100, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
    registry.emplace<Moveable>(player);
    registry.emplace<Collidable>(player);
    registry.emplace<Teleportable>(player);
    registry.emplace<RenderPriority>(player, RenderPriority{1});
    _player = player;

    // Containing Walls
    for (int i = 0; i < 4; ++i) {
        auto wall = registry.create();
        float x, y, width, height;

        if (i == 1 || i == 2) {
            // Vertical walls
            x = -40 + (i - 1) * 80; // Adjust position for vertical walls
            y = -40;
            width = .5;
            height = 80.5; // 4x larger
        } else {
            // Horizontal walls
            x = -40;
            y = -40 + (i / 2) * 80; // Corrected position for horizontal walls
            width = 80.5; // 4x larger
            height = .5;
        }

        registry.emplace<Position>(wall, Position{x, y, 0});
        registry.emplace<Shape>(wall, Shape{width, height});
        registry.emplace<Color>(wall, Color{0, 0, 0, 0.0f}); // Black color
        registry.emplace<Debug>(wall, Debug{Color{0, 0, 0, 1.0f}}); // Black debug color
        registry.emplace<Collidable>(wall);
        registry.emplace<RenderPriority>(wall, RenderPriority{1});
    }

    // Place simple entities along each wall
    for (int i = 0; i < 4; ++i) {
        float x, y, width, height;

        if (i == 1 || i == 2) {
            // Vertical walls
            x = -40 + (i - 1) * 80; // Adjust position for vertical walls
            y = -40;
            width = .5;
            height = 80.5; // 4x larger
        } else {
            // Horizontal walls
            x = -40;
            y = -40 + (i / 2) * 80; // Corrected position for horizontal walls
            width = 80.5; // 4x larger
            height = .5;
        }

        for (int j = 0; j < 10; ++j) {
            auto simpleEntity = registry.create();
            float entityX, entityY;

            if (i == 1 || i == 2) {
                // Vertical walls
                entityX = x + (i == 1 ? -5.0f : -5.0f); // Slightly outside the wall
                entityY = y + j * (height / 10.0f) - 2; // Adjusted position for vertical walls
            } else {
                // Horizontal walls
                entityX = x + j * (width / 10.0f) - 2; // Adjusted position for horizontal walls
                entityY = y + (i == 0 ? 0.0f : 0.0f) - 10; // Slightly outside the wall
            }   
            registry.emplace<Position>(simpleEntity, Position{entityX, entityY, 0});
            registry.emplace<Shape>(simpleEntity, Shape{10, 10});
            registry.emplace<Color>(simpleEntity, Color{255, 0, 0, 1.0f}); // Red color
            registry.emplace<RenderPriority>(simpleEntity, RenderPriority{1});
        }
    }
}