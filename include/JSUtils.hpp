#pragma once
#include <emscripten.h>
#include "../include/json.hpp"
#include <SDL_opengles2.h>
#include "shaders.hpp"
#include "../include/entt.hpp"
#include "../include/structs.hpp"
#include <sstream>
#include <functional>    // For lambda functions

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

void _js__log(string str)
{
    // Send a log to JS
    EM_ASM_({ console.log(UTF8ToString($0)); }, str.c_str());
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

                    // print group name and part count
                    printf("Group: %s, Parts: %zu\n", groupName.c_str(), textureParts.size());
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

                            // Id
                            try
                            {
                                if (components.contains("Id"))
                                {
                                    registry.emplace<Id>(entity, components["Id"].get<int>());
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Id component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Position") && components["Position"].is_object())
                                {
                                    auto &pos = components["Position"];
                                    registry.emplace<Position>(entity, pos["x"], pos["y"], pos["z"]);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Position component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Shape") && components["Shape"].is_object())
                                {
                                    auto &shape = components["Shape"];
                                    registry.emplace<Shape>(entity, shape["size"][0], shape["size"][1], shape["size"][2]);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Shape component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Color") && components["Color"].is_object())
                                {
                                    auto &color = components["Color"];
                                    registry.emplace<Color>(entity, color["r"], color["g"], color["b"], color["a"]);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Color component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("RenderPriority") && components["RenderPriority"].is_object())
                                {
                                    auto &renderPriority = components["RenderPriority"];
                                    registry.emplace<RenderPriority>(entity, renderPriority["priority"]);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding RenderPriority component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Collidable") && components["Collidable"].is_boolean())
                                {
                                    registry.emplace<Collidable>(entity);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Collidable component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Interior") && components["Interior"].is_object())
                                {
                                    bool hideInside = components["Interior"]["hideInside"];
                                    registry.emplace<Interior>(entity, Interior{hideInside});
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Interior component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Test") && components["Test"].is_object())
                                {
                                    auto &test = components["Test"];
                                    registry.emplace<Test>(entity, test["value"]);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Test component: %s\n", e.what());
                            }

                            try
                            {
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

                                        if (entity_id == portalAId)
                                            portalA = entity;
                                        else if (entity_id == portalBId)
                                            portalB = entity;

                                        if ((portalA != entt::null || portalAId == -1) && (portalB != entt::null || portalBId == -1))
                                            break;
                                    }
                                    registry.emplace<InteriorPortal>(entity, InteriorPortal{portalA, portalB});
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding InteriorPortal component: %s\n", e.what());
                            }

                            try
                            {
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

                                    if (inside.contains("showOutside") && inside["showOutside"].is_boolean())
                                    {
                                        registry.emplace<Inside>(entity, Inside{interiorEntity, inside["showOutside"].get<bool>()});
                                    }
                                    else
                                    {
                                        registry.emplace<Inside>(entity, Inside{interiorEntity, false});
                                    }
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Inside component: %s\n", e.what());
                            }

                            try
                            {
                                if (components.contains("Moveable") && components["Moveable"].is_object())
                                {
                                    registry.emplace<Moveable>(entity);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Moveable component: %s\n", e.what());
                            }

                            try
                            {
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
                                                                           movement["drag"].get<float>()});
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Movement component: %s\n", e.what());
                            }

                            // Hoverable
                            try
                            {
                                if (components.contains("Hoverable") && components["Hoverable"].is_boolean())
                                {
                                    registry.emplace<Hoverable>(entity);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Hoverable component: %s\n", e.what());
                            }

                            // Interactable
                            try
                            {
                                if (components.contains("Interactable") && components["Interactable"].is_boolean())
                                {
                                    registry.emplace<Interactable>(entity);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Interactable component: %s\n", e.what());
                            }

                            // Texture
                            try
                            {
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
                                    registry.emplace<Texture>(entity, textureName, x, y, w, h, scalex, scaley);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Texture component: %s\n", e.what());
                            }

                            // TextureGroupPart
                            try
                            {
                                if (components.contains("TextureGroupPart") && components["TextureGroupPart"].is_object())
                                {
                                    auto &textureGroupPart = components["TextureGroupPart"];
                                    std::string groupName = textureGroupPart["groupName"];
                                    std::string partName = textureGroupPart["partName"];
                                    if (textureGroupMap.find(groupName) != textureGroupMap.end() && textureGroupMap[groupName].find(partName) != textureGroupMap[groupName].end())
                                    {
                                        printf("Adding %d to %s\n", static_cast<int>(entity), groupName.c_str());
                                        registry.emplace<TextureGroupPart>(entity, TextureGroupPart{groupName, partName});
                                    }
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding TextureGroupPart component: %s\n", e.what());
                            }

                            // Configurable
                            //  - if has this flag, add an InteractionAction that logs its Id
                            try
                            {
                                if (components.contains("Configurable") && components["Configurable"].is_boolean())
                                {
                                    printf("Adding Configurable component to entity %d\n", static_cast<int>(entity));
                                    registry.emplace<Configurable>(entity);
                                    registry.emplace<InteractionAction>(entity, InteractionAction{
                                        [](entt::registry& registry, entt::entity entity, std::optional<entt::entity> optEntity) {
                                            _js__log("Configurable entity: " + std::to_string(registry.get<Id>(entity).id));
                                        },
                                    true});
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Configurable component: %s\n", e.what());
                            }

                            // Teleporter
                            try
                            {
                                if (components.contains("Teleporter") && components["Teleporter"].is_object())
                                {
                                    printf("Adding Teleporter component to entity %d\n", static_cast<int>(entity));
                                    auto &teleporter = components["Teleporter"];
                                    Position destination{teleporter["destination"]["x"], teleporter["destination"]["y"], teleporter["destination"]["z"]};
                                    registry.emplace<Teleport>(entity, Teleport{.destination = destination});
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Teleporter component: %s\n", e.what());
                            }

                            // Teleportable
                            try
                            {
                                if (components.contains("Teleportable") && components["Teleportable"].is_boolean())
                                {
                                    printf("Adding Teleportable component to entity %d\n", static_cast<int>(entity));
                                    registry.emplace<Teleportable>(entity);
                                }
                            }
                            catch (const std::exception &e)
                            {
                                printf("Error adding Teleportable component: %s\n", e.what());
                            }
                        }
                    }
                }
            }
        }
    }
}
