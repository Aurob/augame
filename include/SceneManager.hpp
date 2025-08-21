#pragma once

#include "lib/entt.hpp"
#include "structs.hpp"
#include <vector>
#include <memory>
#include <string>

class SceneManager {
private:
    std::vector<std::unique_ptr<entt::registry>> scenes;
    std::vector<MetaData> sceneMetadata;
    std::vector<entt::entity> playerEntities;
    int currentSceneIndex = 0;

public:
    SceneManager() = default;
    ~SceneManager() = default;

    entt::registry& getCurrentRegistry() {
        if (scenes.empty()) {
            scenes.push_back(std::make_unique<entt::registry>());
            sceneMetadata.push_back(MetaData{});
            playerEntities.push_back(entt::null);
        }
        return *scenes[currentSceneIndex];
    }

    MetaData& getCurrentMetadata() {
        if (sceneMetadata.empty()) {
            sceneMetadata.push_back(MetaData{});
        }
        return sceneMetadata[currentSceneIndex];
    }

    entt::entity getCurrentPlayer() {
        if (playerEntities.empty()) {
            playerEntities.push_back(entt::null);
        }
        return playerEntities[currentSceneIndex];
    }

    void setCurrentPlayer(entt::entity player) {
        if (playerEntities.empty()) {
            playerEntities.push_back(entt::null);
        }
        playerEntities[currentSceneIndex] = player;
    }

    void addScene(const MetaData& metadata) {
        auto newRegistry = std::make_unique<entt::registry>();
        newRegistry->ctx().emplace<MetaData>(metadata);
        newRegistry->ctx().emplace<entt::entity>(entt::null);
        
        scenes.push_back(std::move(newRegistry));
        sceneMetadata.push_back(metadata);
        playerEntities.push_back(entt::null);
    }

    void switchToScene(int index) {
        if (index >= 0 && index < static_cast<int>(scenes.size())) {
            currentSceneIndex = index;
        }
    }

    void switchToNextScene() {
        if (!scenes.empty()) {
            currentSceneIndex = (currentSceneIndex + 1) % scenes.size();
        }
    }

    void switchToPrevScene() {
        if (!scenes.empty()) {
            currentSceneIndex = (currentSceneIndex - 1 + scenes.size()) % scenes.size();
        }
    }

    size_t getSceneCount() const {
        return scenes.size();
    }

    int getCurrentSceneIndex() const {
        return currentSceneIndex;
    }

    bool hasScenes() const {
        return !scenes.empty();
    }
};

extern SceneManager sceneManager;