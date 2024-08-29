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

// key, float value
void _js__kvdata(string k, float v)
{
    // Send a float to JS
    EM_ASM_({ Module.setkv(UTF8ToString($0), $1); }, k.c_str(), v);
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
    }
    return js_json;
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

                        // Check for Position:x:y
                        if (_el.contains("Position") && _el["Position"].is_object())
                        {
                            auto &position = _el["Position"];
                            if (position.contains("x") && position["x"].is_number() && position.contains("y") && position["y"].is_number())
                            {
                                float x = position["x"];
                                float y = position["y"];

                                Position &playerPos = registry.get<Position>(_player);
                                playerPos.x = x;
                                playerPos.y = y;
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

                            // Id
                            if (components.contains("Id")) 
                            {
                                registry.emplace<Id>(entity, components["Id"].get<int>());
                            }

                            if (components.contains("Position") && components["Position"].is_object())
                            {
                                auto &pos = components["Position"];
                                registry.emplace<Position>(entity, pos["x"], pos["y"], pos["z"]);
                            }

                            if (components.contains("Shape") && components["Shape"].is_object())
                            {
                                auto &shape = components["Shape"];
                                registry.emplace<Shape>(entity, shape["size"][0], shape["size"][1], shape["size"][2]);
                            }

                            if (components.contains("Color") && components["Color"].is_object())
                            {
                                auto &color = components["Color"];
                                registry.emplace<Color>(entity, color["r"], color["g"], color["b"], color["a"]);
                            }

                            if (components.contains("RenderPriority") && components["RenderPriority"].is_object())
                            {
                                auto &renderPriority = components["RenderPriority"];
                                registry.emplace<RenderPriority>(entity, renderPriority["priority"]);
                            }

                            if (components.contains("Collidable") && components["Collidable"].is_boolean())
                            {
                                registry.emplace<Collidable>(entity);
                            }

                            if (components.contains("Interior") && components["Interior"].is_object())
                            {
                                bool hideInside = components["Interior"]["hideInside"];
                                registry.emplace<Interior>(entity, Interior{hideInside});
                            }

                            if (components.contains("Test") && components["Test"].is_object())
                            {
                                auto &test = components["Test"];
                                registry.emplace<Test>(entity, test["value"]);
                            }

                            if (components.contains("InteriorPortal") && components["InteriorPortal"].is_object())
                            {
                                auto &interiorPortal = components["InteriorPortal"];
                                auto portalAId = interiorPortal["A"].get<int>();
                                auto portalBId = interiorPortal["B"].get<int>();

                                auto view = registry.view<Id>();
                                entt::entity portalA = entt::null;
                                entt::entity portalB = entt::null;

                                for (auto entity : view)
                                {
                                    int entity_id = view.get<Id>(entity).id;

                                    if(entity_id == portalAId) portalA = entity;
                                    else if(entity_id == portalBId) portalB = entity;
                                    
                                    if((portalA != entt::null || portalAId == -1) 
                                        && (portalB != entt::null || portalBId == -1))
                                        break;
                                }
                                registry.emplace<InteriorPortal>(entity, InteriorPortal{portalA, portalB});
                            }
                            if (components.contains("Inside") && components["Inside"].is_object())
                            {
                                auto &inside = components["Inside"];
                                auto interiorEntityId = inside["interiorEntity"].get<int>();

                                auto view = registry.view<Id>();
                                entt::entity interiorEntity = entt::null;
                                for (auto entity : view)
                                {
                                    if (view.get<Id>(entity).id == interiorEntityId)
                                    {
                                        interiorEntity = entity;
                                        break;
                                    }
                                }

                                if (inside.contains("showOutside") && inside["showOutside"].is_boolean()) {
                                    registry.emplace<Inside>(entity, Inside{interiorEntity, inside["showOutside"].get<bool>()});
                                } else {
                                    registry.emplace<Inside>(entity, Inside{interiorEntity, false});
                                }
                            }

                            if (components.contains("Moveable") && components["Moveable"].is_object())
                            {
                                registry.emplace<Moveable>(entity);
                            }

                            if (components.contains("Movement") && components["Movement"].is_object())
                            {
                                auto &movement = components["Movement"];
                                registry.emplace<Movement>(entity, Movement{
                                    movement["speed"].get<float>(),
                                    movement["maxSpeed"].get<float>(),
                                    {movement["acceleration"]["x"].get<float>(), movement["acceleration"]["y"].get<float>()},
                                    {movement["velocity"]["x"].get<float>(), movement["velocity"]["y"].get<float>()},
                                    movement["friction"].get<float>(),
                                    movement["mass"].get<float>(),
                                    movement["drag"].get<float>()
                                });
                            }

                            // Hoverable
                            if (components.contains("Hoverable") && components["Hoverable"].is_boolean())
                            {
                                registry.emplace<Hoverable>(entity);
                            }

                            // Interactable
                            if (components.contains("Interactable") && components["Interactable"].is_boolean())
                            {
                                registry.emplace<Interactable>(entity);
                            }

                            // Texture
                            if (components.contains("Texture") && components["Texture"].is_object())
                            {
                                auto &texture = components["Texture"];
                                std::string textureName = texture["name"];
                                float scalex = texture.value("scalex", 1.0f);
                                float scaley = texture.value("scaley", 1.0f);
                                float x = texture.value("x", 0.0f);
                                float y = texture.value("y", 0.0f);
                                float w = texture.value("w", 1.0f);
                                float h = texture.value("h", 1.0f);
                                registry.emplace<Texture>(entity, textureName, scalex, scaley, x, y, w, h);

                                printf("Texture: %s\n", textureName.c_str());
                            }
                        }
                    }
                }
            }
        }
    }
}
