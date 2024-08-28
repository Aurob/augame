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

auto createEntity = [](const Position& pos, const Shape& shape, const Color& color,
    const std::string& debugValue, int renderPriority, bool collidable = false) {
    auto entity = registry.create();
    registry.emplace<Position>(entity, pos);
    registry.emplace<Shape>(entity, shape);
    registry.emplace<Color>(entity, color);
    registry.emplace<Debug>(entity, Debug{.value1 = debugValue});
    registry.emplace<RenderPriority>(entity, RenderPriority{renderPriority});
    if (collidable) registry.emplace<Collidable>(entity);
    return entity;
};

void testing_v2() {
    
    // // Walls
    // // front wall
    // Position frontWallPos{0, 35, 0};
    // Shape frontWallShape{{10, 1, 8}};
    // Color frontWallColor{0.2, 0.4, 0.6, .7};
    // createEntity(frontWallPos, frontWallShape, frontWallColor, "wall2", 2, true);

    // // left wall
    // Position leftWallPos{-1, 19, 0};
    // Shape leftWallShape{{1, 16, 8}};
    // Color leftWallColor{0.3, 0.5, 0.7, 1};
    // createEntity(leftWallPos, leftWallShape, leftWallColor, "wall3", 2, true);


    // // 1st floor
    // Position floor1Pos{0, 20, -1};
    // Shape floor1Shape{{15, 15, 0}};
    // Color floor1Color{0.2, 0.8, 0.2, 1};
    // createEntity(floor1Pos, floor1Shape, floor1Color, "floor1", -1);

    // auto createFloorWithRailing = [](int floorNumber, float posX, float posY, float posZ, float sizeX, float sizeY, float sizeZ, float colorR, float colorG, float colorB, float colorA, int renderPriority) {
    //     Position floorPos{posX, posY, posZ};
    //     Shape floorShape{{sizeX, sizeY, sizeZ}};
    //     Color floorColor{colorR, colorG, colorB, colorA};
    //     createEntity(floorPos, floorShape, floorColor, "floor" + std::to_string(floorNumber), renderPriority);

    //     Position railingPos{posX + sizeX / 2, posY, posZ};
    //     Shape railingShape{{0.1f, sizeY, sizeZ}};
    //     Color railingColor{colorR * 0.5f, colorG * 0.5f, colorB * 0.5f, 1.0f};
    //     createEntity(railingPos, railingShape, railingColor, "railing" + std::to_string(floorNumber), renderPriority + 1, true);
    // };

    // // 2nd floor
    // createFloorWithRailing(2, 0, 18, 1, 4, 12, 1, 0.2, 0.5, 0.2, 0.9, 2);

    // // 3rd floor
    // createFloorWithRailing(3, 0, 16, 2, 4, 12, 1, 0.3, 0.4, 0.6, 0.8, 3);

    // // 4th floor
    // createFloorWithRailing(4, 0, 14, 3, 4, 12, 1, 0.4, 0.3, 0.5, 0.7, 4);

    // // 5th floor
    // createFloorWithRailing(5, 0, 12, 4, 4, 12, 1, 0.5, 0.2, 0.4, 0.6, 5);


    Position interiorPos{0, 10, 0};
    Shape interiorShape{{15, 15, 1}};
    Color interiorColor{0.5, 0.5, 0.5, 1.0};
    auto interiorEntity = createEntity(interiorPos, interiorShape, interiorColor, "interior", 1, true);
    registry.emplace<Interior>(interiorEntity);
    registry.emplace<Test>(interiorEntity, Test{"Interior"});

    Position doorPos{2, 25, 0};
    Shape doorShape{{1, .1, 1}};
    Color doorColor{0.6, 0.3, 0.1, 1.0};
    auto doorEntity = createEntity(doorPos, doorShape, doorColor, "door", 2, true);
    registry.emplace<InteriorPortal>(doorEntity, InteriorPortal{interiorEntity, entt::null});
    registry.emplace<Inside>(doorEntity, Inside{interiorEntity});
    registry.emplace<Associated>(doorEntity, Associated{std::vector<entt::entity>{interiorEntity}});
    registry.emplace<Test>(doorEntity, Test{"Door"});


    // 25 small entities with moveable and movement and colidable
    for (int i = 0; i < 25; ++i) {
        float x = -7 + i % 5;
        float y = 12 + i / 2;
        auto entity = createEntity(
            Position{x, y, 0}, 
            Shape{{1, 1, 1}}, 
            Color{0.8, 0.8, 0.8, 1}, 
            "entity" + std::to_string(i), 
            0, 
            true
        );
        registry.emplace<Moveable>(entity);
        registry.emplace<Movement>(entity, Movement{100, 1000, Vector2f{0, 0}, Vector2f{0, 0}, 10, 1, 0});
    }


    // back wall
    Position backWallPos{0, 10, .5};
    Shape backWallShape{{15, 1, 2}};
    Color backWallColor{0.1, 0.2, 0.4, 1.0}; // Darkish blue
    auto e = createEntity(backWallPos, backWallShape, backWallColor, "wall1", 1, true);
    registry.emplace<Inside>(e, Inside{interiorEntity});

    // back wall at y=18
    // Position backWallPosMid1{0, 18, .5};
    // Shape backWallShapeMid1{{6, 1, 2}};
    // Color backWallColorMid1{0.05, 0.1, 0.2, 1.0}; // Darker blue for 2nd floor
    // auto eMid1 = createEntity(backWallPosMid1, backWallShapeMid1, backWallColorMid1, "wallMid1", 1, true);
    // registry.emplace<Inside>(eMid1, Inside{interiorEntity});

    // right wall at y=18
    Position backWallPosMid2{5, 12, .5};
    Shape backWallShapeMid2{{1, 6, 2}};
    Color backWallColorMid2{0.05, 0.1, 0.2, 1.0}; // Darker blue for 2nd floor
    auto eMid2 = createEntity(backWallPosMid2, backWallShapeMid2, backWallColorMid2, "wallMid2", 1, true);
    registry.emplace<Inside>(eMid2, Inside{interiorEntity});

    // back wall at y=24
    Position backWallPos2{0, 24, .5};
    Shape backWallShape2{{15, 1, 2}};
    Color backWallColor2{0.1, 0.2, 0.4, 1.0}; // Darkish blue
    auto e2 = createEntity(backWallPos2, backWallShape2, backWallColor2, "wall2", 1, false);
    registry.emplace<Inside>(e2, Inside{interiorEntity});

    // 2ndfloor wall at y=18
    Position secondFloorWallPos{0, 12, 2.1};
    Shape secondFloorWallShape{{1, 1, 1}};
    Color secondFloorWallColor{0.1, 0.2, 0.3, 1.0}; // Darker blue for 2nd floor
    auto eSecondFloor = createEntity(secondFloorWallPos, secondFloorWallShape, secondFloorWallColor, "wallMid2", 1, true);
    registry.emplace<Inside>(eSecondFloor, Inside{interiorEntity});

    // room
    Position roomPos{9, 10, .1};
    Shape roomShape{{6, 6, 1}};
    Color roomColor{0.15, 0.15, 0.15, 1.0}; // Slightly lighter than 2nd floor
    auto roomEntity = createEntity(roomPos, roomShape, roomColor, "room", 1, true);
    registry.emplace<Inside>(roomEntity, Inside{interiorEntity});
    registry.emplace<Test>(roomEntity, Test{"Room"});

    // room door
    Position roomDoorPos{8.9, 13, .1};
    Shape roomDoorShape{{1, .1, 1}};
    Color roomDoorColor{0.6, 0.3, 0.1, 1.0};
    auto roomDoorEntity = createEntity(roomDoorPos, roomDoorShape, roomDoorColor, "roomDoor", 2, true);
    registry.emplace<InteriorPortal>(roomDoorEntity, InteriorPortal{roomEntity, entt::null});
    registry.emplace<Inside>(roomDoorEntity, Inside{interiorEntity});

    // lower wall of the room
    Position lowerWallPos{9, 15, .1};
    Shape lowerWallShape{{6, 1, 2}};
    Color lowerWallColor{0.1, 0.2, 0.4, 1.0}; // Darkish blue
    auto lowerWallEntity = createEntity(lowerWallPos, lowerWallShape, lowerWallColor, "lowerWall", 1, true);
    registry.emplace<Inside>(lowerWallEntity, Inside{interiorEntity});

}