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

// Logging function
void log_message(const string& message) {
    static stringstream log_buffer;
    log_buffer << message << "\n";
    
    // Print the buffer when it reaches a certain size or when explicitly flushed
    if (log_buffer.str().length() > 1000) {
        printf("%s", log_buffer.str().c_str());
        log_buffer.str("");
        log_buffer.clear();
    }
}

// Flush the log buffer
void flush_log() {
    static stringstream log_buffer;
    if (!log_buffer.str().empty()) {
        printf("%s", log_buffer.str().c_str());
        log_buffer.str("");
        log_buffer.clear();
    }
}

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
        log_message("JSON parse error: " + string(e.what()));
        log_message("str: " + str);
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
        log_message("Loading JSON");
        nlohmann::json js_json = str_to_json(str);
        if (js_json.contains("world")) {
            if (js_json["world"].contains("width") && js_json["world"]["width"].is_number()) {
                width = js_json["world"]["width"];
                windowResized = true;
            }
            if (js_json["world"].contains("height") && js_json["world"]["height"].is_number()) {
                height = js_json["world"]["height"];
                windowResized = true;
            }

            // zoom
            if (js_json["world"].contains("zoom") && js_json["world"]["zoom"].is_number()) {
                float zoom = js_json["world"]["zoom"];
                if (zoom == -1) {
                    gridSpacingValue /= 1.08f;
                } else if (zoom == 1) {
                    gridSpacingValue *= 1.08f;
                }
            }
        } 
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
                        
                        log_message("Loaded shader: " + shader["name"].get<std::string>());                        
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
                        log_message("Loaded texture: " + texture["path"].get<std::string>());
                        
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

                    // Check for Action
                    if (_el.contains("Action") && _el["Action"].is_object()) {
                        auto &action = _el["Action"];
                        if (action.contains("player") && action["player"].is_array()) {
                            auto &playerActions = action["player"];
                            for (const auto &act : playerActions) {
                                if (act == "interact") {
                                    if (is_player) {
                                        // Trigger interaction for player
                                        // You might want to implement an interaction system or event
                                        log_message("Player interaction triggered");
                                        keys[SDL_BUTTON_LEFT] = true;
                                    }
                                }
                            }
                        }
                    }

                    // Check for Position:x:y
                    if (_el.contains("Position") && _el["Position"].is_object())
                    {
                        auto &position = _el["Position"];
                        if (position.contains("x") && position["x"].is_number() && position.contains("y") && position["y"].is_number())
                        {
                            float x = position["x"];
                            float y = position["y"];
                            log_message("Position: " + to_string(x) + ", " + to_string(y));

                            if (is_player)
                            {
                                Position &playerPos = registry.get<Position>(_player);
                                playerPos.x = x;
                                playerPos.y = y;
                            }
                        }
                    }

                    // Check for MobileMovement:w:a:s:d
                    if (_el.contains("MobileMovement") && _el["MobileMovement"].is_object())
                    {
                        auto &mobileMovement = _el["MobileMovement"];
                        if (mobileMovement.contains("w") && mobileMovement["w"].is_number() && mobileMovement.contains("a") && mobileMovement["a"].is_number() && mobileMovement.contains("s") && mobileMovement["s"].is_number() && mobileMovement.contains("d") && mobileMovement["d"].is_number())
                        {
                            float w = mobileMovement["w"];
                            float a = mobileMovement["a"];
                            float s = mobileMovement["s"];
                            float d = mobileMovement["d"];

                            if (is_player)
                            {
                                log_message("MobileMovement: " + to_string(w) + ", " + to_string(a) + ", " + to_string(s) + ", " + to_string(d));
                                keys[SDLK_a] = a;
                                keys[SDLK_w] = w;
                                keys[SDLK_s] = s;
                                keys[SDLK_d] = d;
                            }
                        }
                    }
                }
            }
        }
        
        // Flush the log buffer at the end of processing
        flush_log();
    }
}
