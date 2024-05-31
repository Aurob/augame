#pragma once
#include <emscripten.h>
#include "../include/json.hpp"
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include <SDL_opengles2.h>
#include "shaders.hpp"

using namespace std;

extern float waterMax;
extern float sandMax;
extern float dirtMax;
extern float grassMax;
extern float stoneMax;
extern float snowMax;
extern float frequency;
extern float amplitude;
extern float persistence;
extern float lacunarity;
extern float scale;
extern int octaves;
extern int width, height;
extern int tileMinMax[4];
extern bool windowResized;
extern GLfloat playerPosition[2];
extern GLfloat gridSpacingValue;
extern bool ready;
// key, float value
void _js__kvdata(string k, float v)
{
    // Send a float to JS
    EM_ASM_({
        Module.setkv(UTF8ToString($0), $1);
    }, k.c_str(), v);
}

void _js__ready()
{
    // Send a ready signal to JS
    EM_ASM({
        Module.ready();
    });
}

void _js__refresh()
{
    // Refresh the UI
    EM_ASM({
        Module.refresh();
    });
}

nlohmann::json str_to_json(string str)
{
    nlohmann::json js_json;
    try
    {
        string trimmed_str = str;
        js_json = nlohmann::json::parse(trimmed_str);
    }
    catch (nlohmann::json::parse_error &e)
    {
        printf("JSON parse error: %s\n", e.what());
        printf("str: %s\n", str.c_str());
    }
    return js_json;
}

void process_component(string key, nlohmann::json value);

extern "C"
{
    EMSCRIPTEN_KEEPALIVE

    void isready()
    {
        ready = true;
    }

    void load_json(char *str)
    {
        printf("Loading JSON\n");
        nlohmann::json js_json = str_to_json(str);
        // printf("Loaded JSON: %s\n", js_json.dump().c_str());
        // Process config
        if (js_json.contains("shaders") && js_json["shaders"].is_array())
        {
            // printf("Shaders: %s\n", js_json["shaders"].dump().c_str());
            for (const auto& shader : js_json["shaders"])
            {
                // printf("Shader: %s\n", shader.dump().c_str());
                if (shader.is_object() && shader.contains("vertex") && shader["vertex"].is_string() && shader.contains("fragment") && shader["fragment"].is_string())
                {

                    // printf("Shader: %s\n", shader.dump().c_str());
                    if(shader.contains("type") && shader["type"].is_string()) {
                        if(shader["type"] == "1") {
                            vertexSource = strdup(shader["vertex"].get<std::string>().c_str());
                            fragmentSource = strdup(shader["fragment"].get<std::string>().c_str());
                        }
                        else if(shader["type"] == "2") {
                            vertexSource2 = strdup(shader["vertex"].get<std::string>().c_str());
                            fragmentSource2 = strdup(shader["fragment"].get<std::string>().c_str());
                        }
                        else if(shader["type"] == "3") {
                            vertexSourceTexture = strdup(shader["vertex"].get<std::string>().c_str());
                            fragmentSourceTexture = strdup(shader["fragment"].get<std::string>().c_str());

                            // set texturePath if 'texture' key is present
                            if(shader.contains("texture") && shader["texture"].is_string()) {
                                texturePath = strdup(shader["texture"].get<std::string>().c_str());
                            }
                        }
                    }
                }
                // printf("Shader: %s\n", shader.dump().c_str());
            // }
            }
        }
        if (js_json.contains("option") && js_json["option"].is_string() && js_json.contains("value"))
        {
            auto value = js_json["value"];
            std::map<std::string, std::function<void()>> option_map = {
                {"waterMax", [&]() { waterMax = value.get<float>(); }},
                {"sandMax", [&]() { sandMax = value.get<float>(); }},
                {"dirtMax", [&]() { dirtMax = value.get<float>(); }},
                {"grassMax", [&]() { grassMax = value.get<float>(); }},
                {"stoneMax", [&]() { stoneMax = value.get<float>(); }},
                {"snowMax", [&]() { snowMax = value.get<float>(); }},
                {"frequency", [&]() { frequency = value.get<float>(); }},
                {"amplitude", [&]() { amplitude = value.get<float>(); }},
                {"persistence", [&]() { persistence = value.get<float>(); }},
                {"lacunarity", [&]() { lacunarity = value.get<float>(); }},
                {"scale", [&]() { scale = value.get<float>(); }},
                {"octaves", [&]() { octaves = static_cast<int>(value.get<float>()); }},
                {"refresh", [&]() { _js__refresh(); }}
            };

            std::string option_name = js_json["option"];
            
            auto it = option_map.find(option_name);
            if (it != option_map.end() && (option_name != "refresh" ? value.is_number() : true))
            {
                it->second();
            }
            else
            {
                printf("Unknown option: %s\n", option_name.c_str());
            }
        }
        
        if (js_json.contains("World") && js_json["World"].is_object()) {
            auto &world = js_json["World"];
            if (world.contains("Window")) {
                auto &window = world["Window"];
                width = window.value("width", width);
                height = window.value("height", height);
                windowResized = window.contains("width") || window.contains("height");
                if (windowResized) printf("Window: %d, %d\n", width, height);
            }
            if (world.contains("Tiles")) {
                auto &tiles = world["Tiles"];
                tileMinMax[0] = tiles.value("minx", tileMinMax[0]);
                tileMinMax[1] = tiles.value("miny", tileMinMax[1]);
                tileMinMax[2] = tiles.value("maxx", tileMinMax[2]);
                tileMinMax[3] = tiles.value("maxy", tileMinMax[3]);
            }
            // Scale
            if (world.contains("Scale")) {
                auto &scale = world["Scale"];
                gridSpacingValue = scale.value("scale", gridSpacingValue);
            }
        }

        if (js_json.contains("Entities") && js_json["Entities"].is_array())
        {
            for (auto &_el : js_json["Entities"])
            {
                // entt::entity entity;
                if (_el.is_object())
                {
                    bool is_player = false;

                    // Check if the Player key is present
                    if (_el.contains("Player") && _el["Player"].is_boolean())
                    {
                        is_player = _el["Player"];
                    }

                    // Check for Position:x:y
                    if (_el.contains("Position") && _el["Position"].is_object())
                    {
                        auto &position = _el["Position"];
                        if (position.contains("x") && position["x"].is_number() && position.contains("y") && position["y"].is_number())
                        {
                            float x = position["x"];
                            float y = position["y"];
                            printf("Position: %f, %f\n", x, y);

                            if (is_player)
                            {
                                playerPosition[0] = x;
                                playerPosition[1] = y;
                            }
                        }
                    }

                    // Check for Shape:w:h
                    if (_el.contains("Shape") && _el["Shape"].is_object())
                    {
                        auto &shape = _el["Shape"];
                        if (shape.contains("w") && shape["w"].is_number() && shape.contains("h") && shape["h"].is_number())
                        {
                            float w = shape["w"];
                            float h = shape["h"];
                            printf("Shape: %f, %f\n", w, h);
                        }
                    }

                    // Check for Movement:speed:max_speed:friction:mass
                    if (_el.contains("Movement") && _el["Movement"].is_object())
                    {
                        auto &movement = _el["Movement"];
                        if (movement.contains("speed") && movement["speed"].is_number() 
                        && movement.contains("max_speed") && movement["max_speed"].is_number() 
                        && movement.contains("friction") && movement["friction"].is_number() 
                        && movement.contains("mass") && movement["mass"].is_number())
                        {
                            float speed = movement["speed"];
                            float max_speed = movement["max_speed"];
                            float friction = movement["friction"];
                            float mass = movement["mass"];
                            printf("Movement: %f, %f, %f, %f\n", speed, max_speed, friction, mass);
                        }
                    }
                }
            }
        }
    }
}
