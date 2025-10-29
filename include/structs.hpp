#pragma once

#include <vector>
#include <functional>
#include <any>
#include <optional>
#include <unordered_map>
#include "../include/lib/entt.hpp"
#include <emscripten.h>

using namespace std;

struct context
{
    SDL_Event event;
    int iteration;
    SDL_Window *window;
};

struct Scene 
{
 entt::registry registry;
 entt::entity player;
};

struct Vector2f {
    float x, y;

    Vector2f& operator+=(const Vector2f& other) { x += other.x; y += other.y; return *this; }
    Vector2f& operator-=(const Vector2f& other) { x -= other.x; y -= other.y; return *this; }
    Vector2f& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2f& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

    Vector2f operator+(const Vector2f& other) const { return Vector2f(*this) += other; }
    Vector2f operator-(const Vector2f& other) const { return Vector2f(*this) -= other; }
    Vector2f operator*(float scalar) const { return Vector2f(*this) *= scalar; }
    Vector2f operator/(float scalar) const { return Vector2f(*this) /= scalar; }

    float dot(const Vector2f& other) const { return x * other.x + y * other.y; }
    float length() const { return std::sqrt(x * x + y * y); }
    Vector2f normalized() const { float len = length(); return len > 0 ? *this / len : *this; }
};

struct Vector3f {
    float x, y, z;

    Vector3f& operator+=(const Vector3f& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3f& operator-=(const Vector3f& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vector3f& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector3f& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    Vector3f operator+(const Vector3f& other) const { return Vector3f(*this) += other; }
    Vector3f operator-(const Vector3f& other) const { return Vector3f(*this) -= other; }
    Vector3f operator*(float scalar) const { return Vector3f(*this) *= scalar; }
    Vector3f operator/(float scalar) const { return Vector3f(*this) /= scalar; }

    float dot(const Vector3f& other) const { return x * other.x + y * other.y + z * other.z; }
    Vector3f cross(const Vector3f& other) const { return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x}; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3f normalized() const { float len = length(); return len > 0 ? *this / len : *this; }
};

struct Internal {
    bool internal;
};

struct Id {
    int id;
    string name;
    bool other1;
};

struct Player {};

struct Position {
    float x, y, z;
    float sx, sy, sz;
};

struct Shape {
    Vector3f size{1, 1, 1};
    Vector3f scaled_size;
    Vector3f alt_size{0, 0, 0};
    Vector3f alt_scaled_size;
};

struct Color {
    float r, g, b, a;
    float defaultR, defaultG, defaultB, defaultA;

    Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a), defaultR(r), defaultG(g), defaultB(b), defaultA(a) {}
};

struct Text {
    std::string text;
    float scale;
    bool hide;
    float offsetX;
    float offsetY;
    
    Text(const std::string& text, float scale, bool hide, float offsetX = 0.0f, float offsetY = 0.0f)
        : text(text), scale(scale), hide(hide), offsetX(offsetX), offsetY(offsetY) {}
};

struct Visible {};
struct InView {};

struct Debug {
    Color defaultColor;
    std::string value1;
};
struct Teleport {
    Position origin;
    Position destination;
    bool reverse{false};
    bool disabled{false};
};
struct Teleportable {
    bool refresh;
    bool done{false};
    int timer;
};

struct Hoverable {
    int duration;
};
struct Hovered {
};

struct Interacted {
    entt::entity interactor;
    int interactions;
    bool offset_initialized;
    float offset_x;
    float offset_y;
    float offset_px;
    float offset_py;
};

struct TickAction {
    std::function<void(entt::registry&, entt::entity)> action;
    float interval;
    float time;
};

struct Colliding{
    std::vector<entt::entity> collidables;
    std::vector<Vector3f> overlaps;
};
struct Collidable {
    std::vector<entt::entity> colliding_with;
    bool ignorePlayer;
    bool ignoreCollideAll;
    bool ignoreOnInteract{true};
};
struct Movement {
    float speed{10};
    float default_speed{10};
    float max_speed{110};
    Vector2f velocity{0, 0};
    Vector2f acceleration{0, 0};
    float friction{1};
    float mass{1};
    float restitution{0.5};

    Movement(float _speed = 10, float _mass = 1, float _restitution = 0.5, float _friction = 0.5) 
        : speed(_speed), default_speed(_speed), mass(_mass), restitution(_restitution), friction(_friction){}
};

struct Moveable {};

struct Rotation {
    float angle{0};
    float angular_velocity{0};
    float angular_acceleration{0};
    float angular_friction{1};
};

// Interiors
struct Interior {
    bool showInside;
};
struct InteriorColliding {
    entt::entity interior;
};
struct Inside {
    entt::entity interior;
    bool showOutside;
};
struct InteriorPortal {
    entt::entity A;
    entt::entity B;
    bool locked;
    entt::entity key;
};
struct OnInteriorPortal {
    entt::entity portal;
    int timeout;
};

struct RenderPriority {
    int priority;
};

struct Texture {
    std::string name;
    float x, y, w, h;
    float scalex{1}, scaley{1};
};

struct Textures {
    std::vector<Texture> textures;
    int current;
    Texture metadata;
};

struct TextureAlts {
    std::unordered_map<std::string, Textures> alts;
    std::string current;
};

struct TextureAnimation {
    double timestamp{emscripten_get_now() / 1000.0};  // Current time position in the animation (in seconds)
    float interval{1.0f};                            // Total duration of the animation in seconds
    bool paused{false};                              // Whether the animation is currently paused
    float currentTime{0.0f};                         // Current time in the animation cycle
    bool noloop{false};
};

struct TextureGroupPart {
    std::string groupName;
    std::string partName;
    Texture texture;
    int tilex;
    int tiley;
    
};

struct Keys {
    std::unordered_map<SDL_Keycode, bool> keys;
};

struct Cursor {
    Position position;
    bool firstdown; // Used to check if this is the first time the mouse is down after being up
    bool firstup; // same but reversed
    int downtime;
    Vector2f vec2_dist;
};

struct Test { std::string value; };

struct Interactable {
    int interactions;
    float radius;
    bool toggleState;
    bool allowDiffInterior;
    bool toggle() {
        toggleState = !toggleState;
        return toggleState;
    }
};

struct Flag {
    std::string name;
    int id;
};

struct Static {};

struct Camera {
    float gridSpacing = 1024.0f;
    float defaultGSV = 16.0f;
    Vector2f offset = {0.0f, 0.0f};
    Vector2f topLeftTile = {0.0f, 0.0f};
    int priority = 0;
    float radius = 0.0f;
    bool important = false;
};

struct MetaData {
    std::string scene = "";
    std::string title = "";
    std::string description = "";
    std::string author = "";
    std::string font = "HomeVideo-Regular.ttf";
    std::string terrain = "terrain"; // shader name or color for outside rendering
    std::vector<float> terrain_color = {0.0f, 0.0f, 0.0f}; // parsed color if terrain is color
    std::vector<float> terrain_bounds = {}; // terrain bounds: {minX, minY, maxX, maxY}. Empty = infinite
    std::string void_bg = ""; // color for inside rendering (empty = black)
    std::vector<float> void_color = {0.0f, 0.0f, 0.0f}; // parsed void color
    std::string start_menu = "Start"; // path to start menu text file
    std::string pause_menu = "Paused"; // path to pause menu text file
    std::string str_seed = ""; // original seed as string (can be words, sentences, etc.)
    int seed = 0;              // numeric seed, converted from str_seed
    std::unordered_map<int, std::string> slides; // gameState -> text content mapping

};

struct GameState {
    int width = 1024;
    int height = 1024;
    float deltaTime = 0.0f;
    bool ready = false;
    int seed = 0;
    int gameState = -2;
    bool playerCameraMode = false; // true for player camera, false for priority camera

    int intro_scenes = 3;
    context* ctx = nullptr;
    bool active = true;

};

struct world {
};

struct CustomShader {
    std::string shaderName;
    std::vector<float> uniforms;
    int uniformCount;
    
    CustomShader(const std::string& name = "", int count = 0) 
        : shaderName(name), uniformCount(count) {
        uniforms.resize(count, 0.0f);
    }
};