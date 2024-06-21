#pragma once
#include <emscripten.h>
#include "../include/json.hpp"
#include <SDL_opengles2.h>
#include "shaders.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"

using namespace std;

extern int width, height;
extern int tileMinMax[4];
extern bool windowResized;
extern GLfloat playerPosition[2];
extern GLfloat gridSpacingValue;
extern bool ready;
extern entt::entity _player;
extern entt::registry registry;

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

void _js__fetch_configs()
{
    // Fetch the configs from JS
    EM_ASM({
        Module.fetch_configs();
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
        // printf("JSON: %s\n", js_json.dump().c_str());

        if (js_json.contains("shader") && js_json["shader"].is_object())
        { 
            // should contain "name", "vertex", "fragment"
            auto shader = js_json["shader"];

            if (shader.contains("name") && shader["name"].is_string()) {

                bool shader_exists = shaderGLSLMap.find(shader["name"]) != shaderGLSLMap.end();

                if(!shader_exists) {

                    if(shader.contains("vertex") && shader["vertex"].is_string() 
                    && shader.contains("fragment") && shader["fragment"].is_string()) {

                        
                        const GLchar *vertexSource = strdup(shader["vertex"].get<std::string>().c_str());
                        const GLchar *fragmentSource = strdup(shader["fragment"].get<std::string>().c_str());                  
                        std::string fragmentSourceStr = shader["fragment"].get<std::string>();
                        
                        shaderGLSLMap[shader["name"]] = {
                            vertexSource, 
                            fragmentSource
                        };
                        
                        printf("Loaded shader: %s\n", shader["name"].get<std::string>().c_str());                        
                    }
                }
            }
        }

        if (js_json.contains("texture") && js_json["texture"].is_object()) {
            auto texture = js_json["texture"];
            if (texture.contains("name") && texture["name"].is_string()) {
                bool texture_exists = textureMap.find(texture["name"]) != textureMap.end();
                if(!texture_exists) {
                    if(texture.contains("path") && texture["path"].is_string()) {
                        textureMap[texture["name"]] = texture["path"];
                        printf("Loaded texture: %s\n", texture["path"].get<std::string>().c_str());
                    }
                }
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
                                Position &playerPos = registry.get<Position>(_player);
                                playerPos.x = x;
                                playerPos.y = y;
                            }
                        }
                    }
                }
            }
        }
    }
}
