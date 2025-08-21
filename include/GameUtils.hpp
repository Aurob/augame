#pragma once
#include "structs.hpp"
#include "SceneManager.hpp"

extern SceneManager sceneManager;

// Utility function to get an entity by name
inline entt::entity getEntityByName(const std::string& name) {
    auto idView = sceneManager.getCurrentRegistry().view<Id>();
    for (auto entity : idView) {
        const auto& id = idView.get<Id>(entity);
        if (id.name == name) {
            return entity;
        }
    }
    return entt::null;
}

// TickAction helper functions for common patterns
namespace TickActions {
    // Increment shader uniform at index
    inline std::function<void(entt::registry&, entt::entity)> incrementShaderUniform(int uniformIndex, float increment = 1.0f) {
        return [uniformIndex, increment](entt::registry& registry, entt::entity entity) {
            if (registry.all_of<CustomShader>(entity)) {
                auto& shader = registry.get<CustomShader>(entity);
                if (shader.uniforms.size() > uniformIndex) {
                    shader.uniforms[uniformIndex] += increment;
                }
            }
        };
    }

    // Animate texture sequence
    inline std::function<void(entt::registry&, entt::entity)> animateTextures() {
        return [](entt::registry& registry, entt::entity entity) {
            if (registry.all_of<TextureAlts>(entity)) {
                auto& textureAlts = registry.get<TextureAlts>(entity);
                auto& currentTextures = textureAlts.alts[textureAlts.current];
                currentTextures.current = (currentTextures.current + 1) % currentTextures.textures.size();
            }
        };
    }

    // Move entity by delta
    inline std::function<void(entt::registry&, entt::entity)> moveBy(float dx, float dy) {
        return [dx, dy](entt::registry& registry, entt::entity entity) {
            if (registry.all_of<Position>(entity)) {
                auto& pos = registry.get<Position>(entity);
                pos.x += dx;
                pos.y += dy;
            }
        };
    }
}

// Helper function to attach TickAction to entity by name
inline void addTickAction(const std::string& entityName, std::function<void(entt::registry&, entt::entity)> action, float interval = 0.19f) {
    entt::entity entity = getEntityByName(entityName);
    if (entity != entt::null) {
        sceneManager.getCurrentRegistry().emplace_or_replace<TickAction>(entity, TickAction{action, interval});
    }
}
