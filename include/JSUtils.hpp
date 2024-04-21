#pragma once
#include <emscripten.h>
#include "../include/json.hpp"
#include <map>
#include <functional>
#include <iostream>
#include <sstream>

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
    void load_json(char *str)
    {
        nlohmann::json js_json = str_to_json(str);
        // printf("Loaded JSON: %s\n", js_json.dump().c_str());
        // Process options
        if (js_json.contains("option") && js_json["option"].is_string() && js_json.contains("value"))
        {
            std::string option_name = js_json["option"];
            auto value = js_json["value"];
            if (option_name == "waterMax" && value.is_number())
            {
                waterMax = value.get<float>();
            }
            else if (option_name == "sandMax" && value.is_number())
            {
                sandMax = value.get<float>();
            }
            else if (option_name == "dirtMax" && value.is_number())
            {
                dirtMax = value.get<float>();
            }
            else if (option_name == "grassMax" && value.is_number())
            {
                grassMax = value.get<float>();
            }
            else if (option_name == "stoneMax" && value.is_number())
            {
                stoneMax = value.get<float>();
            }
            else if (option_name == "snowMax" && value.is_number())
            {
                snowMax = value.get<float>();
            }
            else if (option_name == "frequency" && value.is_number())
            {
                frequency = value.get<float>();
            }
            else if (option_name == "amplitude" && value.is_number())
            {
                amplitude = value.get<float>();
            }
            else if (option_name == "persistence" && value.is_number())
            {
                persistence = value.get<float>();
            }
            else if (option_name == "lacunarity" && value.is_number())
            {
                lacunarity = value.get<float>();
            }
            else if (option_name == "scale" && value.is_number())
            {
                scale = value.get<float>();
            }
            else if (option_name == "octaves" && value.is_number())
            {
                octaves = static_cast<int>(value.get<float>());
            }
            else if (option_name == "refresh")
            {
                _js__refresh();
            }
            else
            {
                printf("Unknown option: %s\n", option_name.c_str());
            }
        }
    }
}
