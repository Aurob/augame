
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>
#include <emscripten.h>

extern float deltaTime;
extern GameState gameState;
extern entt::entity _player;
extern bool windowResized;

void updateAnimations(entt::registry &registry) {
    
    auto animations = registry.view<Textures, TextureAnimation>();
    for(auto e : animations) {
        Textures &t = animations.get<Textures>(e);
        TextureAnimation &anim = animations.get<TextureAnimation>(e);
        
        // Get current time
        double currentTime = emscripten_get_now() / 1000.0;
        
        // Check if animation should advance
        if (!anim.paused && currentTime > anim.timestamp + anim.interval) {
            // Increment texture index
            if (anim.noloop) {
                // When not looping, ensure it doesn't exceed the max index
                t.current = std::min(t.current + 1, static_cast<int>(t.textures.size() - 1));
            } else {
                // Loop around when reaching the end
                t.current = (t.current + 1) % t.textures.size();
            }
            // Reset timestamp
            anim.timestamp = currentTime;
        }
    }

}

