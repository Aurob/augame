#pragma once
#include "shaders.hpp"
#include "lib/entt.hpp"
#include "../include/structs.hpp"
#include "../include/Systems/ViewSystems.hpp"
#include "../include/SceneManager.hpp"
#include "../include/EntityFactory.hpp"
#include "lib/physics.hpp"
#include <vector>
#include <unordered_map>
#include "lib/json.hpp"
#include <GL/gl.h>

using namespace std;

extern GameState gameState;
extern MetaData metaData;
extern bool ready;
extern bool first_start;
extern entt::entity _player;
extern SceneManager sceneManager;
extern p2d::Physics physics;

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

// Function declarations
void createShader(GLuint &shaderProgram, std::string program_name);

void load_json_to_registry(const nlohmann::json& str, entt::registry& targetRegistry, MetaData& targetMetadata);
void load_json_to_registry(char *str, entt::registry& targetRegistry, MetaData& targetMetadata);

// Helper functions for entity management
entt::entity findEntityById(entt::registry& registry, int id);
void removeEntityById(entt::registry& registry, int id);
void updateEntityComponents(entt::registry& registry, entt::entity entity, const nlohmann::json& components);

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
        load_json_to_registry(str, sceneManager.getCurrentRegistry(), sceneManager.getCurrentMetadata());
    }

    void createSceneFromJson(char *str)
    {
        MetaData sceneMetadata;
        sceneManager.addScene(sceneMetadata);
        int sceneIndex = sceneManager.getSceneCount() - 1;
        sceneManager.switchToScene(sceneIndex);
        load_json_to_registry(str, sceneManager.getCurrentRegistry(), sceneManager.getCurrentMetadata());
        // Switch back to first scene after loading all scenes
        if (sceneManager.getSceneCount() > 1) {
            sceneManager.switchToScene(0);
            auto& firstRegistry = sceneManager.getCurrentRegistry();
            auto playerView = firstRegistry.view<Player>();
            _player = playerView.empty() ? entt::null : playerView.front();
            sceneManager.setCurrentPlayer(_player);
        }
    }

    void switchToNextScene()
    {
        // printf("Switching to next scene...\n");
        sceneManager.switchToNextScene();
        
        // Update global player reference safely
        auto& currentRegistry = sceneManager.getCurrentRegistry();
        auto playerView = currentRegistry.view<Player>();
        _player = playerView.empty() ? entt::null : playerView.front();
        
        // Ensure player has ALL required components
        if (_player != entt::null) {
            // Run makePlayer to ensure all components
            makePlayer(currentRegistry);
        }
        
        // Store the player entity in the scene manager for consistency
        sceneManager.setCurrentPlayer(_player);
        
        // Reset to start menu for new scene
        gameState.gameState = -1;
        
        printf("Switched to scene %d/%zu\n", sceneManager.getCurrentSceneIndex() + 1, sceneManager.getSceneCount());
    }

    void switchToPrevScene()
    {
        // printf("Switching to prev scene...\n");
        sceneManager.switchToPrevScene();

        // Update global player reference safely
        auto& currentRegistry = sceneManager.getCurrentRegistry();
        auto playerView = currentRegistry.view<Player>();
        _player = playerView.empty() ? entt::null : playerView.front();

        // Ensure player has ALL required components
        if (_player != entt::null) {
            // Run makePlayer to ensure all components
            makePlayer(currentRegistry);
        }

        // Store the player entity in the scene manager for consistency
        sceneManager.setCurrentPlayer(_player);

        // Reset to start menu for new scene
        gameState.gameState = -1;

        printf("Switched to scene %d/%zu\n", sceneManager.getCurrentSceneIndex() + 1, sceneManager.getSceneCount());
    }

    void addEntityToScene(int sceneIndex, char* entityConfigStr)
    {
        if (sceneIndex < 0 || sceneIndex >= static_cast<int>(sceneManager.getSceneCount())) {
            printf("Error: Invalid scene index %d (valid range: 0-%zu)\n", sceneIndex, sceneManager.getSceneCount() - 1);
            return;
        }

        // Store current scene index
        int originalSceneIndex = sceneManager.getCurrentSceneIndex();

        // Switch to target scene temporarily
        sceneManager.switchToScene(sceneIndex);

        // Load the single entity config into the target scene
        load_json_to_registry(entityConfigStr, sceneManager.getCurrentRegistry(), sceneManager.getCurrentMetadata());

        // printf("Added entity to scene %d\n", sceneIndex);

        // Switch back to original scene
        sceneManager.switchToScene(originalSceneIndex);
    }
}

void load_json_to_registry(const nlohmann::json& js_json, entt::registry& targetRegistry, MetaData& targetMetadata)
{
    std::string str = js_json.dump();
    load_json_to_registry(const_cast<char*>(str.c_str()), targetRegistry, targetMetadata);
}

void load_json_to_registry(char *str, entt::registry& targetRegistry, MetaData& targetMetadata)
{
        nlohmann::json js_json = str_to_json(str);
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

                        // Compile the dynamic shader immediately
                        createShader(shaderProgramMap[shader["name"]], shader["name"]);
                        // printf("Created and compiled dynamic shader %s\n", shader["name"].get<std::string>().c_str());
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
                        std::string texName = texture["name"];
                        std::string texPath = texture["path"];
                        textureMap[texName] = texPath;
                        // printf("Texture loaded: Name='%s', Path='%s'\n", texName.c_str(), texPath.c_str());
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
                    
                    // printf("Loading texture group: %s %s %zu parts\n", groupName.c_str(), group["name"].get<std::string>().c_str(), group["parts"].size());

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

        // Handle meta data
        if (js_json.contains("meta") && js_json["meta"].is_object())
        {
            auto meta = js_json["meta"];
            if (meta.contains("scene") && meta["scene"].is_string()) {
                targetMetadata.scene = meta["scene"];
                // printf("Meta: Scene set to %s\n", targetMetadata.scene.c_str());
            }
            if (meta.contains("title") && meta["title"].is_string()) {
                targetMetadata.title = meta["title"];
            }
            if (meta.contains("description") && meta["description"].is_string()) {
                targetMetadata.description = meta["description"];
            }
            if (meta.contains("author") && meta["author"].is_string()) {
                targetMetadata.author = meta["author"];
            }
            if (meta.contains("font") && meta["font"].is_string()) {
                targetMetadata.font = meta["font"];
            }
            if (meta.contains("world")) {
                if (meta["world"].is_string()) {
                    targetMetadata.terrain = meta["world"];
                } else if (meta["world"].is_array() && meta["world"].size() >= 3) {
                    // Color array [r, g, b] from JavaScript
                    targetMetadata.terrain_color[0] = meta["world"][0];
                    targetMetadata.terrain_color[1] = meta["world"][1];
                    targetMetadata.terrain_color[2] = meta["world"][2];
                    targetMetadata.terrain = "color"; // Mark as color instead of shader
                }
            }
            if (meta.contains("void")) {
                if (meta["void"].is_string()) {
                    targetMetadata.void_bg = meta["void"];
                } else if (meta["void"].is_array() && meta["void"].size() >= 3) {
                    // Color array [r, g, b] from JavaScript
                    targetMetadata.void_color[0] = meta["void"][0];
                    targetMetadata.void_color[1] = meta["void"][1];
                    targetMetadata.void_color[2] = meta["void"][2];
                    targetMetadata.void_bg = "color"; // Mark as color
                }
            }
            if (meta.contains("terrain_bounds") && meta["terrain_bounds"].is_array()) {
                // Parsing is now done client-side, expect array directly
                if (meta["terrain_bounds"].size() == 4) {
                    targetMetadata.terrain_bounds = {
                        meta["terrain_bounds"][0].get<float>(),
                        meta["terrain_bounds"][1].get<float>(),
                        meta["terrain_bounds"][2].get<float>(),
                        meta["terrain_bounds"][3].get<float>()
                    };
                    // printf("Meta: Terrain bounds set to [%.1f, %.1f, %.1f, %.1f]\n",
                    //        targetMetadata.terrain_bounds[0], targetMetadata.terrain_bounds[1],
                    //        targetMetadata.terrain_bounds[2], targetMetadata.terrain_bounds[3]);
                }
            }
            if (meta.contains("start_menu") && meta["start_menu"].is_string()) {
                targetMetadata.start_menu = meta["start_menu"];
            }
            if (meta.contains("pause_menu") && meta["pause_menu"].is_string()) {
                targetMetadata.pause_menu = meta["pause_menu"];
            }
            if (meta.contains("slides") && meta["slides"].is_object()) {
                auto slides = meta["slides"];
                targetMetadata.slides.clear(); // Clear existing slides
                for (auto it = slides.begin(); it != slides.end(); ++it) {
                    try {
                        int slideId = std::stoi(it.key());
                        if (it.value().is_string()) {
                            targetMetadata.slides[slideId] = it.value().get<std::string>();
                            // printf("Meta: Slide %d set to '%s'\n", slideId, targetMetadata.slides[slideId].c_str());
                        }
                    } catch (const std::exception& e) {
                        printf("Error parsing slide ID '%s': %s\n", it.key().c_str(), e.what());
                    }
                }
            }

            // Converts the string seed into an int before storing
            if (meta.contains("seed") && meta["seed"].is_string()) {
                std::string str_seed = meta["seed"];
                unsigned int hash = 0;
                for (char c : str_seed) {
                    hash = hash * 31 + static_cast<unsigned char>(c);
                }
                targetMetadata.seed = static_cast<int>(hash);
            }
        }

        if (js_json.contains("Entities") && js_json["Entities"].is_array())
        {

            std::unordered_map<entt::entity, entt::entity> needsPlaceInside{};
            // Store portals that need key assignment after all entities are loaded
            std::vector<std::tuple<entt::entity, int, int, int, bool>> pendingPortalKeys{};
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
                                        auto &playerKeys = targetRegistry.get<Keys>(_player).keys;
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

                                Position &playerPos = targetRegistry.get<Position>(_player);
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
                                auto &keys = targetRegistry.get<Keys>(_player).keys;
                                keys[SDLK_a] = mobileMovement["a"];
                                keys[SDLK_w] = mobileMovement["w"];
                                keys[SDLK_s] = mobileMovement["s"];
                                keys[SDLK_d] = mobileMovement["d"];
                            }
                        }
                    }

                    else if (_el.contains("New") && _el["New"].is_boolean())
                    {
                        bool isNew = _el["New"].get<bool>();
                        entt::entity entity = entt::null;

                        // Get entity ID if provided
                        int entityId = -1;
                        if (_el.contains("Components") && _el["Components"].is_object() &&
                            _el["Components"].contains("Id") && _el["Components"]["Id"].is_object() &&
                            _el["Components"]["Id"].contains("id") && _el["Components"]["Id"]["id"].is_number()) {
                            entityId = _el["Components"]["Id"]["id"];
                        }

                        if (isNew) {
                            // New: true - Remove existing entity with same ID, then create new
                            if (entityId != -1) {
                                removeEntityById(targetRegistry, entityId);
                            }
                            entity = targetRegistry.create();
                            // printf("Created new entity with id %d\n", entityId);
                        } else {
                            // New: false - Update existing entity
                            if (entityId != -1) {
                                entity = findEntityById(targetRegistry, entityId);
                                if (entity != entt::null) {
                                    // printf("Updating existing entity with id %d\n", entityId);
                                    // Update components using helper function
                                    if (_el.contains("Components") && _el["Components"].is_object()) {
                                        updateEntityComponents(targetRegistry, entity, _el["Components"]);
                                    }
                                    // Skip to next entity
                                    goto next_entity;
                                } else {
                                    // printf("Entity with id %d not found, creating new one\n", entityId);
                                    entity = targetRegistry.create();
                                }
                            } else {
                                // printf("No ID provided for update, creating new entity\n");
                                entity = targetRegistry.create();
                            }
                        }

                        targetRegistry.emplace<RenderPriority>(entity);

                        if (_el.contains("Components") && _el["Components"].is_object())
                        {
                            auto &components = _el["Components"];

                            // Id
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Id") && components["Id"].is_object())
                                {
                                    auto &idComponent = components["Id"];
                                    if (idComponent.contains("id") && idComponent["id"].is_number() && 
                                        idComponent.contains("name") && idComponent["name"].is_string())
                                    {
                                        int id = idComponent["id"];
                                        std::string name = idComponent["name"];
                                        targetRegistry.emplace<Id>(entity, Id{id, name});
                                    }
                                }
                            }, "Id");

                            // Player 
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Player") && components["Player"].is_boolean())
                                {
                                    // Check if there's already an entity with Player component
                                    auto view = targetRegistry.view<Player>();
                                    if (view.empty()) {
                                        targetRegistry.emplace<Player>(entity);
                                    }
                                }
                            }, "Player");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Position") && components["Position"].is_object())
                                {
                                    auto &pos = components["Position"];
                                    targetRegistry.emplace<Position>(entity, pos["x"], pos["y"], pos["z"]);
                                }
                            }, "Position");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Shape") && components["Shape"].is_object())
                                {
                                    auto &shape = components["Shape"];
                                    targetRegistry.emplace<Shape>(entity, shape["size"][0], shape["size"][1], shape["size"][2]);
                                }
                            }, "Shape");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Color") && components["Color"].is_object())
                                {
                                    auto &color = components["Color"];
                                    targetRegistry.emplace<Color>(entity, color["r"], color["g"], color["b"], color["a"]);
                                }
                            }, "Color");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("RenderPriority") && components["RenderPriority"].is_object())
                                {
                                    auto &renderPriority = components["RenderPriority"];
                                    targetRegistry.emplace_or_replace<RenderPriority>(entity, renderPriority["priority"]);
                                }
                            }, "RenderPriority");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Collidable") && components["Collidable"].is_boolean())
                                {
                                    targetRegistry.emplace<Collidable>(entity);
                                }
                            }, "Collidable");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Interior") && components["Interior"].is_object())
                                {
                                    bool showInside = components["Interior"]["showInside"];
                                    targetRegistry.emplace<Interior>(entity, Interior{showInside});
                                }
                            }, "Interior");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("InteriorPortal") && components["InteriorPortal"].is_object())
                                {
                                    auto &interiorPortal = components["InteriorPortal"];
                                    auto portalAId = interiorPortal["A"].get<int>();
                                    auto portalBId = interiorPortal["B"].get<int>();
                                    bool locked = interiorPortal["locked"].get<bool>();
                                    auto keyId = interiorPortal["key"].get<int>();

                                    auto view = targetRegistry.view<Id>();
                                    entt::entity portalA = entt::null;
                                    entt::entity portalB = entt::null;
                                    entt::entity key = entt::null;

                                    // Try to find A and B immediately
                                    for (auto e : view)
                                    {
                                        int entity_id = view.get<Id>(e).id;

                                        if (entity_id == portalAId)
                                            portalA = e;
                                        else if (entity_id == portalBId)
                                            portalB = e;

                                        if ((portalA != entt::null || portalAId == -1) &&
                                            (portalB != entt::null || portalBId == -1))
                                            break;
                                    }

                                    // Create the portal component now (without key)
                                    targetRegistry.emplace<InteriorPortal>(entity, InteriorPortal{portalA, portalB, locked, entt::null});

                                    // Defer key lookup if needed
                                    if (keyId != -1) {
                                        pendingPortalKeys.push_back(std::make_tuple(entity, portalAId, portalBId, keyId, locked));
                                    }
                                }
                            }, "InteriorPortal");
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Inside") && components["Inside"].is_object())
                                {
                                    auto &inside = components["Inside"];
                                    int interiorEntityId = inside["interiorEntity"].get<int>();
                                    
                                    // Look up the entity with the matching Id component
                                    entt::entity interiorEntity = entt::null;
                                    auto view = targetRegistry.view<Id>();
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

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Moveable") && components["Moveable"].is_object())
                                {
                                    targetRegistry.emplace<Moveable>(entity);
                                }
                            }, "Moveable");

                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Movement") && components["Movement"].is_object())
                                {
                                    auto &movement = components["Movement"];
                                    targetRegistry.emplace<Movement>(entity, Movement{
                                        movement["speed"].get<float>(),
                                        movement["mass"].get<float>(),
                                        movement["restitution"].get<float>(),
                                        movement["friction"].get<float>()});
                                }
                            }, "Movement");

                            // Hoverable
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Hoverable") && components["Hoverable"].is_boolean())
                                {
                                    targetRegistry.emplace<Hoverable>(entity);
                                }
                            }, "Hoverable");


                            // Interactable
                            safe_emplace(targetRegistry, entity, [&]() {
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
                                    
                                    targetRegistry.emplace<Interactable>(entity, 0, radius, toggleState);
                                }
                            }, "Interactable");

                            // Texture
                            safe_emplace(targetRegistry, entity, [&]() {
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
                                    targetRegistry.emplace<Texture>(entity, textureName, x, y, w, h, scalex, scaley);
                                }
                            }, "Texture");


                            // Test
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Test") && components["Test"].is_object())
                                {
                                    auto &test = components["Test"];
                                    targetRegistry.emplace<Test>(entity, test["value"]);
                                }
                            }, "Test");

                            // TextureGroupPart
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("TextureGroupPart") && components["TextureGroupPart"].is_object())
                                {
                                    auto &textureGroupPart = components["TextureGroupPart"];
                                    std::string groupName = textureGroupPart["groupName"];
                                    std::string partName = textureGroupPart["partName"];
                                    int tilex = textureGroupPart.value("tilex", 0);
                                    int tiley = textureGroupPart.value("tiley", 0);
                                    if (textureGroupMap.find(groupName) != textureGroupMap.end() && textureGroupMap[groupName].find(partName) != textureGroupMap[groupName].end())
                                    {
                                        targetRegistry.emplace<TextureGroupPart>(entity, TextureGroupPart{groupName, partName, {}, tilex, tiley});
                                    }
                                }
                            }, "TextureGroupPart");

                            // Teleporter
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Teleporter") && components["Teleporter"].is_object())
                                {
                                    auto &teleporter = components["Teleporter"];
                                    Position destination{teleporter["destination"]["x"], teleporter["destination"]["y"], teleporter["destination"]["z"]};
                                    targetRegistry.emplace<Teleport>(entity, Teleport{.destination = destination});
                                }
                            }, "Teleporter");

                            // Teleportable
                            safe_emplace(targetRegistry, entity, [&]() {
                                if (components.contains("Teleportable") && components["Teleportable"].is_boolean())
                                {
                                    targetRegistry.emplace<Teleportable>(entity);
                                }
                            }, "Teleportable");

                            // Text
                            safe_emplace(targetRegistry, entity, [&]() {
                                if(components.contains("Text") && components["Text"].is_object()) {
                                    auto &text = components["Text"];
                                    bool hide = text["hide"] == 1 || text["hide"] == true;
                                    float offsetX = text.value("offsetX", 0.0f);
                                    float offsetY = text.value("offsetY", 0.0f);
                                    targetRegistry.emplace<Text>(entity, text["text"], text["scale"], hide, offsetX, offsetY);
                                }
                            }, "Text");


                            // World
                            safe_emplace(targetRegistry, entity, [&]() {
                                if(components.contains("World") && components["World"].is_boolean()) {
                                }
                            }, "World");

                            // Camera
                            safe_emplace(targetRegistry, entity, [&]() {
                                if(components.contains("Camera") && components["Camera"].is_object()) {
                                    auto &camera = components["Camera"];
                                    Camera cameraComponent;
                                    if (camera.contains("gridSpacing") && camera["gridSpacing"].is_number()) {
                                        cameraComponent.gridSpacing = camera["gridSpacing"];
                                    }
                                    if (camera.contains("defaultGSV") && camera["defaultGSV"].is_number()) {
                                        cameraComponent.defaultGSV = camera["defaultGSV"];
                                    }
                                    if (camera.contains("priority") && camera["priority"].is_number()) {
                                        cameraComponent.priority = camera["priority"];
                                    }
                                    if (camera.contains("radius") && camera["radius"].is_number()) {
                                        cameraComponent.radius = camera["radius"];
                                    }
                                    if (camera.contains("important") && camera["important"].is_boolean()) {
                                        cameraComponent.important = camera["important"];
                                    }
                                    targetRegistry.emplace<Camera>(entity, cameraComponent);
                                }
                            }, "Camera");

                            // CustomShader
                            safe_emplace(targetRegistry, entity, [&]() {
                                if(components.contains("CustomShader") && components["CustomShader"].is_object()) {
                                    auto &customShader = components["CustomShader"];
                                    if (customShader.contains("shaderName") && customShader["shaderName"].is_string() &&
                                        customShader.contains("uniformCount") && customShader["uniformCount"].is_number()) {
                                        
                                        std::string shaderName = customShader["shaderName"];
                                        int uniformCount = customShader["uniformCount"];
                                        
                                        CustomShader shaderComponent(shaderName, uniformCount);
                                        
                                        if (customShader.contains("uniforms") && customShader["uniforms"].is_array()) {
                                            auto &uniformsArray = customShader["uniforms"];
                                            for (int i = 0; i < uniformCount && i < uniformsArray.size(); i++) {
                                                if (uniformsArray[i].is_number()) {
                                                    shaderComponent.uniforms[i] = uniformsArray[i].get<float>();
                                                }
                                            }
                                        }
                                        
                                        targetRegistry.emplace<CustomShader>(entity, shaderComponent);
                                    }
                                }
                            }, "CustomShader");
                        }

                        if (targetRegistry.all_of<Position, Shape>(entity)) {
                            targetRegistry.emplace<PhysicsBodyRect>(entity);
                        }

                        next_entity:; // Label for goto
                    }
                }
            }

            // Process entities that need to be placed inside other entities
            for (const auto& [entity_to_place, interior_entity] : needsPlaceInside)
            {
                if (targetRegistry.valid(entity_to_place) && targetRegistry.valid(interior_entity))
                {
                    targetRegistry.emplace_or_replace<Inside>(entity_to_place, Inside{interior_entity, false});
                }
            }

            // Process deferred InteriorPortal key assignments
            // Now that all entities are loaded, we can look up keys by ID
            for (const auto& [portalEntity, portalAId, portalBId, keyId, locked] : pendingPortalKeys)
            {
                if (!targetRegistry.valid(portalEntity)) continue;

                // Find the key entity by ID
                entt::entity keyEntity = entt::null;
                auto idView = targetRegistry.view<Id>();
                for (auto e : idView)
                {
                    if (idView.get<Id>(e).id == keyId)
                    {
                        keyEntity = e;
                        break;
                    }
                }

                // Update the InteriorPortal component with the key
                if (targetRegistry.all_of<InteriorPortal>(portalEntity))
                {
                    auto& portal = targetRegistry.get<InteriorPortal>(portalEntity);
                    portal.key = keyEntity;

                    if (keyEntity == entt::null && keyId != -1)
                    {
                        printf("Warning: InteriorPortal could not find key entity with ID %d\n", keyId);
                    }
                    else if (keyEntity != entt::null)
                    {
                        // printf("InteriorPortal assigned key entity ID %d\n", keyId);
                    }
                }
            }
        }
    
        if (js_json.contains("font") && js_json["font"].is_array()) {

        }
    }

// Helper function implementations
entt::entity findEntityById(entt::registry& registry, int id)
{
    auto view = registry.view<Id>();
    for (auto entity : view) {
        if (view.get<Id>(entity).id == id) {
            return entity;
        }
    }
    return entt::null;
}

void removeEntityById(entt::registry& registry, int id)
{
    entt::entity entity = findEntityById(registry, id);
    if (entity != entt::null) {
        registry.destroy(entity);
        // printf("Removed entity with id %d\n", id);
    }
}

void updateEntityComponents(entt::registry& registry, entt::entity entity, const nlohmann::json& components)
{
    // Position
    safe_emplace(registry, entity, [&]() {
        if (components.contains("Position") && components["Position"].is_object())
        {
            auto &pos = components["Position"];
            if (registry.all_of<Position>(entity)) {
                auto& position = registry.get<Position>(entity);
                position.x = pos["x"];
                position.y = pos["y"];
                position.z = pos["z"];
            } else {
                registry.emplace<Position>(entity, pos["x"], pos["y"], pos["z"]);
            }
        }
    }, "Position");

    // Shape
    safe_emplace(registry, entity, [&]() {
        if (components.contains("Shape") && components["Shape"].is_object())
        {
            auto &shape = components["Shape"];
            if (registry.all_of<Shape>(entity)) {
                auto& shapeComp = registry.get<Shape>(entity);
                shapeComp.size.x = shape["size"][0];
                shapeComp.size.y = shape["size"][1];
                shapeComp.size.z = shape["size"][2];
            } else {
                registry.emplace<Shape>(entity, shape["size"][0], shape["size"][1], shape["size"][2]);
            }
        }
    }, "Shape");

    // Color
    safe_emplace(registry, entity, [&]() {
        if (components.contains("Color") && components["Color"].is_object())
        {
            auto &color = components["Color"];
            if (registry.all_of<Color>(entity)) {
                auto& colorComp = registry.get<Color>(entity);
                colorComp.r = color["r"];
                colorComp.g = color["g"];
                colorComp.b = color["b"];
                colorComp.a = color["a"];
            } else {
                registry.emplace<Color>(entity, color["r"], color["g"], color["b"], color["a"]);
            }
        }
    }, "Color");

    // Text
    safe_emplace(registry, entity, [&]() {
        if (components.contains("Text") && components["Text"].is_object()) {
            auto &text = components["Text"];
            bool hide = text["hide"] == 1 || text["hide"] == true;
            float offsetX = text.value("offsetX", 0.0f);
            float offsetY = text.value("offsetY", 0.0f);

            if (registry.all_of<Text>(entity)) {
                auto& textComp = registry.get<Text>(entity);
                textComp.text = text["text"];
                textComp.scale = text["scale"];
                textComp.hide = hide;
                textComp.offsetX = offsetX;
                textComp.offsetY = offsetY;
            } else {
                registry.emplace<Text>(entity, text["text"], text["scale"], hide, offsetX, offsetY);
            }
        }
    }, "Text");

    // Movement
    safe_emplace(registry, entity, [&]() {
        if (components.contains("Movement") && components["Movement"].is_object())
        {
            auto &movement = components["Movement"];
            if (registry.all_of<Movement>(entity)) {
                auto& movementComp = registry.get<Movement>(entity);
                movementComp.speed = movement["speed"].get<float>();
                movementComp.mass = movement["mass"].get<float>();
                movementComp.restitution = movement["restitution"].get<float>();
                movementComp.friction = movement["friction"].get<float>();
            } else {
                registry.emplace<Movement>(entity, Movement{
                    movement["speed"].get<float>(),
                    movement["mass"].get<float>(),
                    movement["restitution"].get<float>(),
                    movement["friction"].get<float>()});
            }
        }
    }, "Movement");

    // Add other components as needed - focusing on most commonly updated ones
    // Can extend this with more components as needed

    // Ensure physics body is updated if Position and Shape exist
    if (registry.all_of<Position, Shape>(entity)) {
        if (!registry.all_of<PhysicsBodyRect>(entity)) {
            registry.emplace<PhysicsBodyRect>(entity);
        }
    }
}

