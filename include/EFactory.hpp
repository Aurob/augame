
#pragma once
#include "../include/entt.hpp"
#include <vector>
#include <type_traits>

extern entt::entity _player;

/*For creating entities*/
enum struct ComponentType {
    Position,
    Shape,
    Color,
    Validation,
    Visible,
    Debug,
    Teleport,
    Hoverable,
    Hovered,
    Interactable,
    Interacted,
    Collisions
};

entt::entity createDebugEntity(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    float x = static_cast<float>(rand() % 10) + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) + xOffset;
    float y = static_cast<float>(rand() % 10) + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) + yOffset;

    Color color = {static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), 1.0f};

    Shape shape = {0.2f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.5f, 
                   0.2f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.5f};

    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{x, y});
    registry.emplace<Color>(entity, color);
    registry.emplace<Validation>(entity);
    registry.emplace<Shape>(entity, shape);
    registry.emplace<Debug>(entity, Debug{color});
    registry.emplace<Hoverable>(entity);
    registry.emplace<Interactable>(entity);

    registry.emplace<Movement>(entity, Movement{
        static_cast<float>(rand() % 10) + 1, 
        (static_cast<float>(rand() % 10) + 1) * 4, 
        Vector2f{0, 0}, 
        Vector2f{0, 0}, 
        .1, 
        .25
    });
    registry.emplace<Moveable>(entity);
    registry.emplace<Collidable>(entity);

    return entity;
}

entt::entity createPlayerEntity(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{5 + xOffset, 5 + yOffset});
    registry.emplace<Color>(entity, Color{0, 0, 255, 0.0f});
    registry.emplace<Validation>(entity);
    registry.emplace<Shape>(entity, Shape{1, 1});
    registry.emplace<Collisions>(entity);
    registry.emplace<Player>(entity);
    registry.emplace<Visible>(entity);
    registry.emplace<Movement>(entity, Movement{10, 110, Vector2f{0, 0}, Vector2f{0, 0}, .5, .1});
    registry.emplace<Debug>(entity, Debug{Color{0, 0, 255, 0.0f}});
    registry.emplace<Collidable>(entity);
    return entity;
}

void createWall(entt::registry& registry, float x, float y, float width, float height, Color color, float xOffset = 0.0f, float yOffset = 0.0f) {
    auto wall = registry.create();
    registry.emplace<Position>(wall, Position{x + xOffset, y + yOffset});
    registry.emplace<Color>(wall, color);
    registry.emplace<Validation>(wall);
    registry.emplace<Shape>(wall, Shape{width, height});
    registry.emplace<Debug>(wall, Debug{color});
    registry.emplace<Collidable>(wall);
}

void createDoor(entt::registry& registry, float x, float y, Position destination1, Position destination2, bool bidirectional = false, float xOffset = 0.0f, float yOffset = 0.0f) {
    auto door = registry.create();
    registry.emplace<Position>(door, Position{x + xOffset, y + yOffset});
    registry.emplace<Teleport>(door, Teleport{destination1, destination2, bidirectional});
    registry.emplace<Validation>(door);
    registry.emplace<Debug>(door);
    registry.emplace<Shape>(door, Shape{1, 1});
}

void createRoom(entt::registry& registry, float x, float y, float width, float height, Color wallColor, float xOffset = 0.0f, float yOffset = 0.0f) {
    createWall(registry, x, y, width, 1, wallColor, xOffset, yOffset); // Top wall
    createWall(registry, x + width - 1, y, 1, height, wallColor, xOffset, yOffset); // Right wall
    createWall(registry, x, y + height - 1, width, 1, wallColor, xOffset, yOffset); // Bottom wall
    createWall(registry, x, y, 1, height, wallColor, xOffset, yOffset); // Left wall
}

void createDebugBuilding(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    // Create a simple building consisting of 4 walls
    createRoom(registry, 0, 0, 11, 11, Color{77, 88, 99, 1.0f}, xOffset, yOffset);

    // Add 2 teleporter entities to the building
    // createDoor(registry, 5, 11, Position{5, 12}, Position{5, 8}, false, xOffset, yOffset);
    // createDoor(registry, 5, 9, Position{5, 8}, Position{5, 12}, false, xOffset, yOffset);
}

void createDebugTeleporter(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    struct TeleporterData {
        float x;
        float y;
        Position dest1;
        Position dest2;
        bool bidirectional;
    };

    std::vector<TeleporterData> teleporters = {
        // {1, 1, {1, 1}, {9, 1}, false},  // a->b
        // {9, 1, {9, 1}, {1, 9}, false},  // b->c
        // {1, 9, {1, 9}, {9, 9}, false},  // c->d
        // {9, 9, {9, 9}, {1, 1}, false},  // d->a
        {5, 9, {5, 9}, {5, 12}, false},  // e->f
        {5, 11, {5, 11}, {5, 8}, false}  // f->e
    };

    for (const auto& teleporter : teleporters) {
        auto entity = registry.create();
        registry.emplace<Position>(entity, Position{teleporter.x, teleporter.y});
        registry.emplace<Teleport>(entity, Teleport{teleporter.dest1, teleporter.dest2, teleporter.bidirectional});
        registry.emplace<Validation>(entity);
        registry.emplace<Debug>(entity);
        registry.emplace<Shape>(entity, Shape{1, 1});
    }
}

void makeBigThing(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    float x = 15000;
    float y = 15000;

    Color color = {static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), 1.0f};

    Shape shape = {1000, 1000};

    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{x, y});
    registry.emplace<Color>(entity, color);
    registry.emplace<Validation>(entity);
    registry.emplace<Shape>(entity, shape);
    registry.emplace<Debug>(entity, Debug{color});
    registry.emplace<Collidable>(entity);
}


void runFactories(entt::registry& registry) {

    // create player entity
    _player = createPlayerEntity(registry);
    
    for(int i = 0; i < 10; i++) {
        createDebugEntity(registry);
    }

    createDebugBuilding(registry);
    createDebugTeleporter(registry);
    makeBigThing(registry);
}