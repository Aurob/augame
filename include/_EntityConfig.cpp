#pragma once
#include "shaders.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"
#include "../include/json.hpp"

using namespace std;

extern int width, height;
extern bool windowResized;
extern float gridSpacingValue;
extern bool ready;
extern entt::entity _player;
extern entt::registry registry;

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
    }
    return js_json;
}

template <typename Func>
void safe_emplace(entt::registry& registry, entt::entity entity, Func func, const char* componentName)
{
    try
    {
        func();
    }
    catch (const std::exception &e)
    {
        printf("Error adding %s component: %s\n", componentName, e.what());
    }
}

extern "C"
{
    EMSCRIPTEN_KEEPALIVE

    void isready()
    {
        ready = true;
    }

    void load_json(char *str)
    {
        nlohmann::json js_json = str_to_json(str);
        if (js_json.contains("world"))
        {
            if (js_json["world"].contains("width") && js_json["world"]["width"].is_number())
            {
                width = js_json["world"]["width"];
                windowResized = true;
            }
            if (js_json["world"].contains("height") && js_json["world"]["height"].is_number())
            {
                height = js_json["world"]["height"];
                windowResized = true;
            }

            // zoom
            if (js_json["world"].contains("zoom") && js_json["world"]["zoom"].is_number())
            {
                float zoom = js_json["world"]["zoom"];
                if (zoom == -1)
                {
                    gridSpacingValue /= 1.08f;
                }
                else if (zoom == 1)
                {
                    gridSpacingValue *= 1.08f;
                }
            }
        }
        if (js_json.contains("shader") && js_json["shader"].is_object())
        {
            // should contain "name", "vertex", "fragment"
            auto shader = js_json["shader"];

            if (shader.contains("name") && shader["name"].is_string())
            {

                bool shader_exists = shaderGLSLMap.find(shader["name"]) != shaderGLSLMap.end();

                if (!shader_exists)
                {

                    if (shader.contains("vertex") && shader["vertex"].is_string() && shader.contains("fragment") && shader["fragment"].is_string())
                    {

                        const GLchar *vertexSource = strdup(shader["vertex"].get<std::string>().c_str());
                        const GLchar *fragmentSource = strdup(shader["fragment"].get<std::string>().c_str());
                        std::string fragmentSourceStr = shader["fragment"].get<std::string>();

                        shaderGLSLMap[shader["name"]] = {
                            vertexSource,
                            fragmentSource};
                    }
                }
            }
        }

        if (js_json.contains("texture") && js_json["texture"].is_object())
        {
            auto texture = js_json["texture"];
            if (texture.contains("name") && texture["name"].is_string())
            {
                // printf("Texture: %s\n", texture["name"].get<std::string>().c_str());
                bool texture_exists = textureMap.find(texture["name"]) != textureMap.end();
                if (!texture_exists)
                {
                    if (texture.contains("path") && texture["path"].is_string())
                    {
                        textureMap[texture["name"]] = texture["path"];
                    }
                }
            }
        }

        if (js_json.contains("textureGroups") && js_json["textureGroups"].is_array())
        {
            for (const auto &group : js_json["textureGroups"])
            {
                if (group.contains("name") && group["name"].is_string() && group.contains("parts") && group["parts"].is_array())
                {
                    std::string groupName = group["name"];
                    std::unordered_map<std::string, Texture> textureParts;
                    for (const auto &part : group["parts"])
                    {
                        if (part.contains("name") && part["name"].is_string() &&
                            part.contains("x") && part["x"].is_number() &&
                            part.contains("y") && part["y"].is_number() &&
                            part.contains("w") && part["w"].is_number() &&
                            part.contains("h") && part["h"].is_number())
                        {
                            Texture texturePart;
                            texturePart.name = part["name"];
                            texturePart.x = part["x"];
                            texturePart.y = part["y"];
                            texturePart.w = part["w"];
                            texturePart.h = part["h"];
                            textureParts[part["name"]] = texturePart;
                        }
                    }
                    textureGroupMap[groupName] = textureParts;
                }
            }
        }

        if (js_json.contains("Entities") && js_json["Entities"].is_array())
        {
            for (const auto &_el : js_json["Entities"])
            {
                if (_el.is_object())
                {

                    // Check if the Player key is present
                    if (_el.contains("Player") && _el["Player"].is_boolean())
                    {

                        // Check for Action
                        if (_el.contains("Action") && _el["Action"].is_object())
                        {
                            auto &action = _el["Action"];
                            if (action.contains("player") && action["player"].is_array())
                            {
                                auto &playerActions = action["player"];
                                for (const auto &act : playerActions)
                                {
                                    if (act == "interact")
                                    {
                                        // Trigger interaction for player
                                        auto &playerKeys = registry.get<Keys>(_player).keys;
                                        playerKeys[SDL_BUTTON_LEFT] = true;
                                    }
                                }
                            }
                        }

                        // Check for Position:x:y:z
                        if (_el.contains("Position") && _el["Position"].is_object())
                        {
                            auto &position = _el["Position"];
                            if (position.contains("x") && position["x"].is_number() && 
                                position.contains("y") && position["y"].is_number() && 
                                position.contains("z") && position["z"].is_number())
                            {
                                float x = position["x"];
                                float y = position["y"];
                                float z = position["z"];

                                Position &playerPos = registry.get<Position>(_player);
                                playerPos.x = x;
                                playerPos.y = y;
                                playerPos.z = z;
                            }
                        }

                        // Check for MobileMovement:w:a:s:d
                        if (_el.contains("MobileMovement") && _el["MobileMovement"].is_object())
                        {
                            auto &mobileMovement = _el["MobileMovement"];
                            if (mobileMovement.contains("w") && mobileMovement["w"].is_number() && mobileMovement.contains("a") && mobileMovement["a"].is_number() && mobileMovement.contains("s") && mobileMovement["s"].is_number() && mobileMovement.contains("d") && mobileMovement["d"].is_number())
                            {
                                auto &keys = registry.get<Keys>(_player).keys;
                                keys[SDLK_a] = mobileMovement["a"];
                                keys[SDLK_w] = mobileMovement["w"];
                                keys[SDLK_s] = mobileMovement["s"];
                                keys[SDLK_d] = mobileMovement["d"];
                            }
                        }
                    }

                    else if (_el.contains("New") && _el["New"].is_boolean())
                    {
                        // Create a new entity
                        entt::entity entity = registry.create();

                        if (_el.contains("Components") && _el["Components"].is_object())
                        {
                            auto &components = _el["Components"];

                            safe_emplace(registry, entity, [&]() {
    if (components.contains("struct_id") && components["struct_id"].is_object())
    {
        auto &component = components["struct_id"];
        if (component.contains("id") && component["id"].is_number())
        {
            int id = component["id"];
        }
        if (component.contains("name") && component["name"].is_string())
        {
            string name = component["name"];
        }
        registry.emplace<struct_id>(entity, id, name);
    }
}, "struct_id");

safe_emplace(registry, entity, [&]() {
    if (components.contains("struct_player") && components["struct_player"].is_object())
    {
        auto &component = components["struct_player"];
        registry.emplace<struct_player>(entity, );
    }
}, "struct_player");

safe_emplace(registry, entity, [&]() {
    if (components.contains("struct_position") && components["struct_position"].is_object())
    {
        auto &component = components["struct_position"];
        if (component.contains("x") && component["x"].is_number())
        {
            float x = component["x"];
        }
        if (component.contains("y") && component["y"].is_number())
        {
            float y = component["y"];
        }
        if (component.contains("z") && component["z"].is_number())
        {
            float z = component["z"];
        }
        registry.emplace<struct_position>(entity, x, y, z);
    }
}, "struct_position");

safe_emplace(registry, entity, [&]() {
    if (components.contains("struct_shape") && components["struct_shape"].is_object())
    {
        auto &component = components["struct_shape"];
        if (component.contains("size"))
        {
            Vector3f size = component["size"];
        }
        registry.emplace<struct_shape>(entity, size);
    }
}, "struct_shape");

safe_emplace(registry, entity, [&]() {
    if (components.contains("struct_color") && components["struct_color"].is_object())
    {
        auto &component = components["struct_color"];
        if (component.contains("r") && component["r"].is_number())
        {
            float r = component["r"];
        }
        if (component.contains("g") && component["g"].is_number())
        {
            float g = component["g"];
        }
        if (component.contains("b") && component["b"].is_number())
        {
            float b = component["b"];
        }
        if (component.contains("a") && component["a"].is_number())
        {
            float a = component["a"];
        }
        registry.emplace<struct_color>(entity, r, g, b, a);
    }
}, "struct_color");


                        }
                    }
                }
            }
        }
    }
}
