#pragma once

#include <vector>
#include <functional>
#include <any>
#include <optional>
#include <unordered_map>
#include "entt.hpp"

using namespace std;

struct context
{
    SDL_Event event;
    int iteration;
    SDL_Window *window;
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

/**
 * \defgroup client_components Client Components
 * This group includes all components relevant to the client.
 */

struct Internal {
    bool internal;
};

/// \ingroup client_components
struct Id {
    /// @brief include
    int id;
    /// @brief include
    string name;
};

/// \ingroup client_components
/// @brief tag
struct Player {};


/// \ingroup client_components
struct Position {
    /// @brief include
    float x;
    /// @brief include
    float y;
    /// @brief include
    float z;
    float sx;
    float sy;
    float sz;
};


/// \ingroup client_components
struct Shape {
    /// @brief include
    Vector3f size{1, 1, .1};
    Vector3f scaled_size;
};

/// \ingroup client_components
struct Color {
    /// @brief include
    float r; 
    /// @brief include
    float g; 
    /// @brief include
    float b; 
    /// @brief include
    float a; 
    float defaultR;
    float defaultG;
    float defaultB;
    float defaultA;

    Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a), defaultR(r), defaultG(g), defaultB(b), defaultA(a) {}
};

struct Visible {};
struct InView {};
struct AlwaysInView {};

struct Debug {
    Color defaultColor;
    std::string value1;
};
struct RenderDebug {};

struct Teleport {
    Position origin;
    Position destination;
    bool reverse{false};
    bool disabled{false};
};
struct Teleportable {};

struct Hoverable {
    int duration;
};
struct Hovered {
};

struct Interacted {
    entt::entity interactor;
    int interactions;
};

struct InteractionAction {
    std::function<void(entt::registry&, entt::entity, std::optional<entt::entity>)> action;
    bool toggle;
};

struct InteractionActions {
    std::vector<InteractionAction> actions;
};

struct HoverAction {
    std::function<void(entt::registry&, entt::entity)> action;
};

struct CollisionAction {
    std::function<void(entt::registry&, entt::entity)> action;
};

struct TickAction {
    std::function<void(entt::registry&, entt::entity)> action;
    float interval;
    float time;
};

struct Interactable {
    int interactions;
    float radius;
    bool toggleState;
    bool toggle() {
        toggleState = !toggleState;
        return toggleState;
    }
};

struct Draggable {
    float radius;
    float sradius;
};

struct Colliding{
    std::vector<entt::entity> collidables;
    std::vector<Vector3f> overlaps;
};
struct Collidable {
    std::vector<entt::entity> colliding_with;
    bool ignorePlayer;
    bool ignoreCollideAll;
};

struct Movement {
    float speed{10};
    float max_speed{110};
    Vector2f velocity{0, 0};
    Vector2f acceleration{0, 0};
    float friction{1};
    float mass{1};
    float restitution{0.5};
};
struct Moveable {};

struct Rotation {
    float angle{0};
    float angular_velocity{0};
    float angular_acceleration{0};
    float angular_friction{1};
};


struct Linkable {};
struct Linked {
    entt::entity parent;
    float distance;
    bool keepCollisions{false};
};

struct Associated {
    std::vector<entt::entity> entities;
    bool destroy;

    template<typename... Components>
    std::vector<entt::entity> filterByComponents(entt::registry &registry) const {
        std::vector<entt::entity> filteredEntities;
        for (auto entity : entities) {
            if (registry.all_of<Components...>(entity)) {
                filteredEntities.push_back(entity);
            }
        }
        return filteredEntities;
    }
};

struct Flag {
    std::unordered_map<std::string, std::any> flags;
};

// Interiors
struct Interior {
    bool hideInside;
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
};
struct OnInteriorPortal {
    entt::entity portal;
};

// Basic Pathfinding
struct BasicPathfinding {
    entt::entity target;
    Position targetPos;
};

struct RenderPriority {
    int priority;
};

struct Test {
    std::string value;
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

struct TextureGroupPart {
    std::string groupName;
    std::string partName;
    Texture texture;
    int tilex;
    int tiley;
    
};

struct CollideColorAlt {
    Color inactive;
    Color active;
};

struct InteriorPortalTexture {
    entt::entity portal;
};

struct Elevate {
    int direction{1};
};

struct Keys {
    std::unordered_map<SDL_Keycode, bool> keys;
};

struct Cursor {
    Position position;
};

struct Configurable {};

// UI
struct UIElement {
    std::string content;
    bool visible;
    Vector2f offset;
    Vector2f soffset;
};

struct Tone {
    std::string note;
    std::string duration;
    std::string type;
    float volume;
    bool playing{false};
    int iterations{0};
};

struct Puzzle {
    std::vector<entt::entity> pieces;
    bool solved;
};

struct PuzzlePiece {
    bool active;
};