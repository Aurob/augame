
#pragma once
#include "../include/entt.hpp"
#include <vector>
#include <type_traits>

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

entt::entity createDebugEntity(entt::registry& registry) {

        float x = static_cast<float>(rand() % 10) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float y = static_cast<float>(rand() % 10) + static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

        Color color = {static_cast<float>(rand() % 255),
            static_cast<float>(rand() % 255),
            static_cast<float>(rand() % 255),
            1.0f};

        // rand shape from .1 to .2
        Shape shape = {
            0.1f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.1f, 
            0.1f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.1f};

        auto entity = registry.create();
        registry.emplace<Position>(entity, Position{x, y});
        registry.emplace<Color>(entity, color);
        registry.emplace<Validation>(entity);
        registry.emplace<Shape>(entity, shape);
        registry.emplace<Debug>(entity, Debug{color});
        registry.emplace<Hoverable>(entity);
        registry.emplace<Interactable>(entity);
        registry.emplace<Movement>(entity, Movement{10, 110, Vector2f{0, 0}, Vector2f{0, 0}, .01, 1});
        registry.emplace<Collidable>(entity);

    return entity;
}

entt::entity createPlayerEntity(entt::registry& registry) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{10, 0});
    registry.emplace<Color>(entity, Color{0, 0, 255, 0.0f});
    registry.emplace<Validation>(entity);
    registry.emplace<Shape>(entity, Shape{1, 1});
    registry.emplace<Collisions>(entity);
    registry.emplace<Player>(entity);
    registry.emplace<Visible>(entity);
    registry.emplace<Movement>(entity);
    registry.emplace<Debug>(entity, Debug{Color{0, 0, 255, 0.0f}});
    registry.emplace<Collidable>(entity);
    return entity;
}

void createDebugBuilding(entt::registry& registry) {
    // Create a simple building consisting of 4 walls
    // add 2 teleporter entities to the building

    // Create the building
    auto wall1 = registry.create();
    registry.emplace<Position>(wall1, Position{1, 0});
    registry.emplace<Color>(wall1, Color{255, 0, 0, 1.0f});
    registry.emplace<Validation>(wall1);
    registry.emplace<Shape>(wall1, Shape{11, 1});
    registry.emplace<Debug>(wall1, Debug{Color{255, 0, 0, 0.0f}});
    registry.emplace<Collidable>(wall1);

    auto wall2 = registry.create();
    registry.emplace<Position>(wall2, Position{10, 0});
    registry.emplace<Color>(wall2, Color{255, 0, 0, 1.0f});
    registry.emplace<Validation>(wall2);
    registry.emplace<Shape>(wall2, Shape{1, 10});
    registry.emplace<Debug>(wall2, Debug{Color{255, 0, 0, 0.0f}});
    registry.emplace<Collidable>(wall2);

    auto wall3 = registry.create();
    registry.emplace<Position>(wall3, Position{1, 10});
    registry.emplace<Color>(wall3, Color{255, 0, 0, 1.0f});
    registry.emplace<Validation>(wall3);
    registry.emplace<Shape>(wall3, Shape{11, 1});
    registry.emplace<Debug>(wall3, Debug{Color{255, 0, 0, 0.0f}});
    registry.emplace<Collidable>(wall3);

    auto wall4 = registry.create();
    registry.emplace<Position>(wall4, Position{0, 0});
    registry.emplace<Color>(wall4, Color{255, 0, 0, 1.0f});
    registry.emplace<Validation>(wall4);
    registry.emplace<Shape>(wall4, Shape{1, 10});
    registry.emplace<Debug>(wall4, Debug{Color{255, 0, 0, 0.0f}});
    registry.emplace<Collidable>(wall4);



}

void createDebugTeleporter(entt::registry& registry) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, Position{5, 11});
    registry.emplace<Teleport>(entity, Teleport{Position{5, 12}, Position{5, 8}});
    registry.emplace<Validation>(entity);
    registry.emplace<Debug>(entity);
    registry.emplace<Shape>(entity, Shape{1, 1});

    // // Teleport 2
    auto entity2 = registry.create();
    registry.emplace<Position>(entity2, Position{5, 9});
    registry.emplace<Teleport>(entity2, Teleport{Position{5, 8}, Position{5, 12}});
    registry.emplace<Validation>(entity2);
    registry.emplace<Debug>(entity2);
    registry.emplace<Shape>(entity2, Shape{1, 1});

}