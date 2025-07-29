#pragma once
#include "shaders.hpp"
#include "lib/entt.hpp"
#include "../include/structs.hpp"
#include <vector>
#include <unordered_map>
#include "lib/json.hpp"

using namespace std;

extern bool windowResized;
extern GameState gameState;
extern bool ready;
extern bool first_start;
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

    void reload()
    {
        ready = false;
        first_start = false;
    }

    void load_json(char *str)
    {
        nlohmann::json js_json = str_to_json(str);
        if (js_json.contains("log")) {
            if (js_json["log"].is_string()) {
                emlog(js_json["log"].get<std::string>().c_str());
            } else if (js_json["log"].is_object()) {
                std::string message;
                LogLevel level = LogLevel::CONSOLE;
                
                // Merge all key-value pairs into a message
                for (auto& [key, value] : js_json["log"].items()) {
                    if (key == "level") {
                        if (value.is_string()) {
                            std::string levelStr = value.get<std::string>();
                            if (levelStr == "WARN") level = LogLevel::WARN;
                            else if (levelStr == "ERROR") level = LogLevel::ERROR;
                            else if (levelStr == "DEBUG") level = LogLevel::DEBUG;
                            else if (levelStr == "INFO") level = LogLevel::INFO;
                        } else if (value.is_number()) {
                            level = static_cast<LogLevel>(value.get<int>());
                        }
                    } else {
                        if (!message.empty()) message += ", ";
                        if (!key.empty()) {
                            // Get the string value without quotes
                            std::string valueStr;
                            if (value.is_string()) {
                                valueStr = value.get<std::string>();
                            } else {
                                valueStr = value.dump();
                            }
                            message += key + ": " + valueStr;
                        } else {
                            // Get the string value without quotes
                            if (value.is_string()) {
                                message += value.get<std::string>();
                            } else {
                                message += value.dump();
                            }
                        }
                    }
                }
                
                emlog(message.c_str(), level);
            }
        }
        if (js_json.contains("world"))
        {
            if (js_json["world"].contains("width") && js_json["world"]["width"].is_number())
            {
                // width = js_json["world"]["width"];
                windowResized = true;
            }
            if (js_json["world"].contains("height") && js_json["world"]["height"].is_number())
            {
                // height = js_json["world"]["height"];
                windowResized = true;
            }

            // zoom
            if (js_json["world"].contains("zoom") && js_json["world"]["zoom"].is_number())
            {
                float zoom = js_json["world"]["zoom"];
                // Find camera for zoom adjustments
                auto cameraView = registry.view<Camera>();
                if(cameraView.begin() != cameraView.end()) {
                    auto& camera = registry.get<Camera>(cameraView.front());
                    if (zoom == -1)
                    {
                        camera.gridSpacing /= 1.08f;
                    }
                    else if (zoom == 1)
                    {
                        camera.gridSpacing *= 1.08f;
                    }
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

                        printf("Created shader map %s\n", shader["name"].get<std::string>().c_str());
                    }
                }
            }
        }

        // Loads predefined texture paths from config
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

            std::unordered_map<entt::entity, entt::entity> needsPlaceInside{};
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

                        registry.emplace<RenderPriority>(entity);

                        if (_el.contains("Components") && _el["Components"].is_object())
                        {
                            auto &components = _el["Components"];

                            // Id
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Id") && components["Id"].is_object())
                                {
                                    auto &idComponent = components["Id"];
                                    if (idComponent.contains("id") && idComponent["id"].is_number() && 
                                        idComponent.contains("name") && idComponent["name"].is_string())
                                    {
                                        int id = idComponent["id"];
                                        std::string name = idComponent["name"];
                                        registry.emplace<Id>(entity, Id{id, name});
                                    }
                                }
                            }, "Id");

                            // Player 
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Player") && components["Player"].is_boolean())
                                {
                                    // Check if there's already an entity with Player component
                                    auto view = registry.view<Player>();
                                    if (view.empty()) {
                                        registry.emplace<Player>(entity);
                                    }
                                }
                            }, "Player");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Position") && components["Position"].is_object())
                                {
                                    auto &pos = components["Position"];
                                    registry.emplace<Position>(entity, pos["x"], pos["y"], pos["z"]);
                                }
                            }, "Position");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Shape") && components["Shape"].is_object())
                                {
                                    auto &shape = components["Shape"];
                                    registry.emplace<Shape>(entity, shape["size"][0], shape["size"][1], shape["size"][2]);
                                }
                            }, "Shape");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Color") && components["Color"].is_object())
                                {
                                    auto &color = components["Color"];
                                    registry.emplace<Color>(entity, color["r"], color["g"], color["b"], color["a"]);
                                }
                            }, "Color");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("RenderPriority") && components["RenderPriority"].is_object())
                                {
                                    auto &renderPriority = components["RenderPriority"];
                                    registry.emplace_or_replace<RenderPriority>(entity, renderPriority["priority"]);
                                }
                            }, "RenderPriority");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Collidable") && components["Collidable"].is_boolean())
                                {
                                    registry.emplace<Collidable>(entity);
                                }
                            }, "Collidable");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Interior") && components["Interior"].is_object())
                                {
                                    bool hideInside = components["Interior"]["hideInside"];
                                    registry.emplace<Interior>(entity, Interior{hideInside});
                                }
                            }, "Interior");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("InteriorPortal") && components["InteriorPortal"].is_object())
                                {
                                    auto &interiorPortal = components["InteriorPortal"];
                                    auto portalAId = interiorPortal["A"].get<int>();
                                    auto portalBId = interiorPortal["B"].get<int>();
                                    bool locked = interiorPortal["locked"].get<bool>();
                                    auto keyId = interiorPortal["key"].get<int>();

                                    auto view = registry.view<Id>();
                                    entt::entity portalA = entt::null;
                                    entt::entity portalB = entt::null;
                                    entt::entity key = entt::null;

                                    for (auto entity : view)
                                    {
                                        int entity_id = view.get<Id>(entity).id;

                                        if (entity_id == portalAId)
                                            portalA = entity;
                                        else if (entity_id == portalBId)
                                            portalB = entity;
                                        else if (entity_id == keyId)
                                            key = entity;

                                        if ((portalA != entt::null || portalAId == -1) && 
                                            (portalB != entt::null || portalBId == -1) &&
                                            (key != entt::null || keyId == -1))
                                            break;
                                    }

                                    registry.emplace<InteriorPortal>(entity, InteriorPortal{portalA, portalB, locked, key});
                                }
                            }, "InteriorPortal");
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Inside") && components["Inside"].is_object())
                                {
                                    auto &inside = components["Inside"];
                                    int interiorEntityId = inside["interiorEntity"].get<int>();
                                    
                                    // Look up the entity with the matching Id component
                                    entt::entity interiorEntity = entt::null;
                                    auto view = registry.view<Id>();
                                    for (auto e : view) {
                                        if (view.get<Id>(e).id == interiorEntityId) {
                                            interiorEntity = e;
                                            break;
                                        }
                                    }
                                    
                                    // Only add to needsPlaceInside if we found the interior entity
                                    if (interiorEntity != entt::null) {
                                        needsPlaceInside[entity] = interiorEntity;
                                    }
                                }
                            }, "Inside");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Moveable") && components["Moveable"].is_object())
                                {
                                    registry.emplace<Moveable>(entity);
                                }
                            }, "Moveable");

                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Movement") && components["Movement"].is_object())
                                {
                                    auto &movement = components["Movement"];
                                    registry.emplace<Movement>(entity, Movement{
                                        movement["speed"].get<float>(),
                                        movement["mass"].get<float>(),
                                        movement["restitution"].get<float>(),
                                        movement["friction"].get<float>()});
                                }
                            }, "Movement");

                            // Hoverable
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Hoverable") && components["Hoverable"].is_boolean())
                                {
                                    registry.emplace<Hoverable>(entity);
                                }
                            }, "Hoverable");


                            // Interactable
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Interactable"))
                                {
                                    float radius = 0.5f;
                                    bool toggleState = false;
                                    
                                    if (components["Interactable"].is_object())
                                    {
                                        auto &interactable = components["Interactable"];
                                        if (interactable.contains("radius") && interactable["radius"].is_number())
                                            radius = interactable["radius"];
                                        if (interactable.contains("toggleState") && interactable["toggleState"].is_boolean())
                                            toggleState = interactable["toggleState"];
                                    }
                                    
                                    registry.emplace<Interactable>(entity, 0, radius, toggleState);
                                }
                            }, "Interactable");

                            // Texture
                            safe_emplace(registry, entity, [&]() {
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
                            }, "Texture");


                            // Test
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Test") && components["Test"].is_object())
                                {
                                    auto &test = components["Test"];
                                    registry.emplace<Test>(entity, test["value"]);
                                }
                            }, "Test");

                            // TextureGroupPart
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("TextureGroupPart") && components["TextureGroupPart"].is_object())
                                {
                                    auto &textureGroupPart = components["TextureGroupPart"];
                                    std::string groupName = textureGroupPart["groupName"];
                                    std::string partName = textureGroupPart["partName"];
                                    int tilex = textureGroupPart.value("tilex", 0);
                                    int tiley = textureGroupPart.value("tiley", 0);
                                    if (textureGroupMap.find(groupName) != textureGroupMap.end() && textureGroupMap[groupName].find(partName) != textureGroupMap[groupName].end())
                                    {
                                        registry.emplace<TextureGroupPart>(entity, TextureGroupPart{groupName, partName, {}, tilex, tiley});
                                    }
                                }
                            }, "TextureGroupPart");

                            // Teleporter
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Teleporter") && components["Teleporter"].is_object())
                                {
                                    auto &teleporter = components["Teleporter"];
                                    Position destination{teleporter["destination"]["x"], teleporter["destination"]["y"], teleporter["destination"]["z"]};
                                    registry.emplace<Teleport>(entity, Teleport{.destination = destination});
                                }
                            }, "Teleporter");

                            // Teleportable
                            safe_emplace(registry, entity, [&]() {
                                if (components.contains("Teleportable") && components["Teleportable"].is_boolean())
                                {
                                    registry.emplace<Teleportable>(entity);
                                }
                            }, "Teleportable");

                            // Text
                            safe_emplace(registry, entity, [&]() {
                                if(components.contains("Text") && components["Text"].is_object()) {
                                    auto &text = components["Text"];
                                    bool hide = text["hide"] == 1 || text["hide"] == true;
                                    registry.emplace<Text>(entity, text["text"], text["scale"], hide);
                                }
                            }, "Text");


                            // World
                            safe_emplace(registry, entity, [&]() {
                                if(components.contains("World") && components["World"].is_boolean()) {
                                    printf("World\n");
                                }
                            }, "World");
                        }

                        if (registry.all_of<Position, Shape>(entity)) {
                            registry.emplace<PhysicsBodyRect>(entity);
                        }

                        // Process entities that need to be placed inside other entities
                        for (const auto& [entity_to_place, interior_entity] : needsPlaceInside)
                        {
                            if (registry.valid(entity_to_place) && registry.valid(interior_entity))
                            {
                                registry.emplace_or_replace<Inside>(entity_to_place, Inside{interior_entity, false});
                            }
                        }

                    }
                }
            }
        }
    
        if (js_json.contains("font") && js_json["font"].is_array()) {

        }
    }
}
