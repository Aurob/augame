#pragma once
#include <emscripten.h>
#include "shaders.hpp"
#include "lib/entt.hpp"
#include "../include/structs.hpp"

using namespace std;

extern entt::entity _player;
extern entt::registry registry;

// key, float value
void _js__kvdata(string k, float v)
{
    // Send a float to JS
    EM_ASM_({ Module.setkv(UTF8ToString($0), $1); }, k.c_str(), v);
}

void _js__speak(string text)
{
    EM_ASM_({ Module.speak(UTF8ToString($0))}, text.c_str());
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

void _js__show_alert(string message) 
{
    EM_ASM({
        Module.show_alert(UTF8ToString($0));
    }, message.c_str());
}


void _js__fetch_configs()
{
    // Fetch the configs from JS
    EM_ASM({
        Module.fetch_configs();
    });
}

void _js__update_color(float r, float g, float b) {
    EM_ASM({
        Module.update_color($0, $1, $2);
    }, r, g, b);
}

void _js__update_user_position(float x, float y) {
    EM_ASM({
        Module.update_user_position($0, $1);
    }, x, y);
}
