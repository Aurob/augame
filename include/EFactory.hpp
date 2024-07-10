
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
entt::entity createBasicEntity(entt::registry& registry, float x, float y, float w, float h) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{x, y});
    registry.emplace<Shape>(entity, Shape{w, h});
    return entity;
}
entt::entity createDebugEntity(entt::registry& registry, float x = 0.0f, float y = 0.0f, float w = 0.0f, float h = 0.0f, bool random = false) {

    if (random) {
        x = static_cast<float>(rand() % 10) + static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        y = static_cast<float>(rand() % 10) + static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        w = 0.2f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.5f;
        h = 0.2f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.5f;
    }

    auto entity = createBasicEntity(registry, x, y, w, h);

    Color color = {static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), 1.0f};

    registry.emplace<Color>(entity, color);
    registry.emplace<Validation>(entity);
    registry.emplace<Debug>(entity, Debug{color});
    registry.emplace<Hoverable>(entity);
    registry.emplace<Interactable>(entity);
    registry.emplace<Linkable>(entity);

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
    registry.emplace<Movement>(entity, Movement{25, 110, Vector2f{0, 0}, Vector2f{0, 0}, 4, .1});
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

void createDynamicBuilding(entt::registry& registry, 
                           const std::vector<std::tuple<float, float, float, float>>& walls,
                           const std::vector<std::tuple<float, float, float, float, Position, Position, bool>>& teleporters,
                           Color wallColor = Color{77, 88, 99, 1.0f},
                           float xOffset = 0.0f, float yOffset = 0.0f) {
    
    // Create walls
    for (const auto& [x, y, width, height] : walls) {
        createWall(registry, x, y, width, height, wallColor, xOffset, yOffset);
    }

    // Create teleporters
    for (const auto& [x, y, width, height, dest1, dest2, bidirectional] : teleporters) {
        auto door = registry.create();
        registry.emplace<Position>(door, Position{x + xOffset, y + yOffset});
        registry.emplace<Teleport>(door, Teleport{dest1, dest2, bidirectional});
        registry.emplace<Validation>(door);
        registry.emplace<Debug>(door);
        registry.emplace<Shape>(door, Shape{width, height});
    }
}

void createDebugBuilding(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    std::vector<std::tuple<float, float, float, float>> walls = {
        {0, 0, 11, 1},    // Top wall
        {10, 0, 1, 11},   // Right wall
        {0, 10, 11, 1},   // Bottom wall
        {0, 0, 1, 11}     // Left wall
    };

    std::vector<std::tuple<float, float, float, float, Position, Position, bool>> teleporters = {
        {5, 11, 1, 1, Position{5, 12}, Position{5, 8}, false},
        {5, 9, 1, 1, Position{5, 8}, Position{5, 12}, false}
    };

    createDynamicBuilding(registry, walls, teleporters, Color{77, 88, 99, 1.0f}, xOffset, yOffset);

    // Make another building next to the first one
    std::vector<std::tuple<float, float, float, float>> walls2 = {
        {11, 0, 11, 1},  // Top wall
        {21, 0, 1, 13},  // Right wall
        {11, 0, 1, 11},  // Left wall
        {10, 11, 1, 1}, 
        {12, 12, 10, 1}, 
        {13, 10, 9, 1}, 
        {12, 13, .1, .6},
        {14, 13, .1, .6},
        {12, 13.6, 2, .1}

        
    };

    createDynamicBuilding(registry, walls2, {}, Color{77, 88, 99, 1.0f}, xOffset, yOffset);


    // Create 2 debug entities and add the Linked component and link them    
    float x1 = 11;
    float y1 = 11;
    float w1 = 6;
    float h1 = 1;
    auto entity1 = createDebugEntity(registry, x1, y1, w1, h1);
    registry.remove<Moveable>(entity1);
    registry.remove<Linkable>(entity1);
    registry.remove<Interactable>(entity1);
    registry.remove<Color>(entity1);
    registry.emplace<Color>(entity1, Color{55, 66, 77, 1.0f});
    auto &movement = registry.get<Movement>(entity1);
    movement.friction = 10;

    float x2 = 12.1;
    float y2 = 13;
    float w2 = 1;
    float h2 = .5;
    auto entity2 = createDebugEntity(registry, x2, y2, w2, h2);

    registry.emplace<Linked>(entity1, Linked{entity2, .1});
    // remove color and re-add with 66, 77, 88
    registry.remove<Color>(entity2);
    auto &color2 = registry.emplace<Color>(entity2, Color{100, 22, 33, 1.0f});
    auto movement2 = registry.get<Movement>(entity2);
    movement2.friction = 10;

}

void createDebugTeleporter(entt::registry& registry, float xOffset = 0.0f, float yOffset = 0.0f) {
    std::vector<std::tuple<float, float, float, float, Position, Position, bool>> teleporters = {
        {5, 9, 1, 1, Position{5, 9}, Position{5, 12}, false},
        {5, 11, 1, 1, Position{5, 11}, Position{5, 8}, false}
    };

    createDynamicBuilding(registry, {}, teleporters, Color{}, xOffset, yOffset);
}



void runFactories(entt::registry& registry) {

    // create player entity
    _player = createPlayerEntity(registry);

    
    for(int i = 0; i < 10; i++) {
        createDebugEntity(registry, 0, 0, 0, 0, true);
    }

    createDebugBuilding(registry);
    createDebugTeleporter(registry);
}