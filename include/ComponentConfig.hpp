#pragma once
#include <emscripten.h>
#include "../include/json.hpp"
#include <SDL_opengles2.h>
#include "shaders.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"
#include <sstream>

using namespace std;

extern int width, height;
extern bool windowResized;
extern float gridSpacingValue;
extern bool ready;
extern entt::entity _player;
extern entt::registry registry;
extern unordered_map<int, bool> keys;

void process_entity(bool method, const nlohmann::json &entity_json, entt::entity entity)
{
    printf("Processing entity %d %d %zu\n", method, entity, entity_json.size());

    // Check Position
    if (entity_json.find("Position") != entity_json.end())
    {
        auto position = entity_json["Position"];
        float x = 0.0f, y = 0.0f, z = 0.0f;
        bool has_x = position.find("x") != position.end();
        bool has_y = position.find("y") != position.end();
        bool has_z = position.find("z") != position.end();

        if (has_x) x = position["x"];
        if (has_y) y = position["y"];
        if (has_z) z = position["z"];

        registry.emplace<Position>(entity, x, y, z);
        printf("Added Position component: x=%f, y=%f, z=%f\n", x, y, z);
    }

    // Check Shape
    if (entity_json.find("Shape") != entity_json.end())
    {
        auto shape = entity_json["Shape"];
        Vector3f size{10, 10, 10}; // Default size
        bool has_size = shape.find("size") != shape.end();

        if (has_size)
        {
            auto size_json = shape["size"];
            if (size_json.is_array() && size_json.size() == 3)
            {
                size.x = size_json[0];
                size.y = size_json[1];
                size.z = size_json[2];
            }
        }

        registry.emplace<Shape>(entity, size);
        printf("Added Shape component: size.x=%f, size.y=%f, size.z=%f\n", size.x, size.y, size.z);
    }

    // Check Color
    if (entity_json.find("Color") != entity_json.end())
    {
        auto color = entity_json["Color"];
        float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
        bool has_r = color.find("r") != color.end();
        bool has_g = color.find("g") != color.end();
        bool has_b = color.find("b") != color.end();
        bool has_a = color.find("a") != color.end();

        if (has_r) r = color["r"];
        if (has_g) g = color["g"];
        if (has_b) b = color["b"];
        if (has_a) a = color["a"];

        registry.emplace<Color>(entity, r, g, b, a);
        printf("Added Color component: r=%f, g=%f, b=%f, a=%f\n", r, g, b, a);
    }   

    registry.emplace<Debug>(entity, Color(1.0f, 0.0f, 0.0f, 1.0f));
}