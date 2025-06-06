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

void _js__play_tone(string note, string duration, float volume = 0.5, string type = "sine")
{
    // Play a tone
    EM_ASM_({
        Module.play_tone(UTF8ToString($0), UTF8ToString($1), $2, UTF8ToString($3));
    }, note.c_str(), duration.c_str(), volume, type.c_str());
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

enum LogLevel {
    CONSOLE = 1,       // Output to console
    WARN = 2,          // Output to console as a warning
    ERROR = 4,         // Output to console as an error
    INFO = 512,        // Output to console as info
    DEBUG = 256,       // Output to console as debug
    JS_STACK = 16,     // Add a JS stack trace to the message
    NO_PATHS = 64,     // Omit file paths in stack traces
};

void emlog(const char* msg, LogLevel level = CONSOLE) {
    // Supports log levels:
    // LogLevel::CONSOLE - Standard output (default)
    // LogLevel::WARN - Warnings 
    // LogLevel::ERROR - Errors
    // LogLevel::DEBUG - Debug
    // LogLevel::INFO - Info
    // LogLevel::JS_STACK - Add a JS stack trace
    // LogLevel::NO_PATHS - Omit file paths in stack traces
    // Can combine with | for multiple flags
    
    // Use emscripten_log which will be compiled to call emscriptenLog internally
    // This ensures proper warning coloring based on the flags
    int flags = static_cast<int>(level);
    
    // Make sure CONSOLE flag is set if any output is desired
    if (!(flags & CONSOLE) && (flags & (WARN | ERROR | INFO | DEBUG))) {
        flags |= CONSOLE;
    }
    
    emscripten_log(flags, "%s", msg);
}