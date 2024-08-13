#pragma once

#include <vector>
#include <functional>
#include <any>
#include <unordered_map>

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

// Entity Components
struct Player {};
struct Position {
    float x;
    float y;
    float z;
    float sx;
    float sy;
    float sz;
};

struct Shape {
    Vector3f size{10, 10, 10};
    Vector3f scaled_size;
};

struct Color {
    float r;
    float g;
    float b;
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
};

struct Teleport {
    Position origin;
    Position destination;
    bool reverse{false};
};
struct Teleportable {};

struct Hoverable {};
struct Hovered {};

struct Interacted {
    entt::entity interactor;
};

struct InteractionAction {
    std::function<void(entt::registry&, entt::entity)> action;
};

struct Interactable {
    int interactions;
    bool toggle;
};

// Utility functions for common actions
namespace InteractionActions {
    template<typename Component>
    InteractionAction AddComponent() {
        return {[](entt::registry& registry, entt::entity entity) {
            registry.emplace<Component>(entity);
        }};
    }

    template<typename Component>
    InteractionAction RemoveComponent() {
        return {[](entt::registry& registry, entt::entity entity) {
            registry.remove<Component>(entity);
        }};
    }

    template<typename Component, typename Func>
    InteractionAction ModifyComponent(Func&& modifier) {
        return {[modifier = std::forward<Func>(modifier)](entt::registry& registry, entt::entity entity) {
            if (auto* component = registry.try_get<Component>(entity)) {
                modifier(*component);
            }
        }};
    }

    // Example: Add random color action
    InteractionAction AddRandomColor() {
        return ModifyComponent<Color>([](Color& color) {
            color = {static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), static_cast<float>(rand() % 255), 1.0f};
        });
    }
}

struct Colliding{
    std::vector<entt::entity> collidables;
    std::vector<Vector2f> overlaps;
};
struct Collidable {
    std::vector<entt::entity> colliding_with;
    bool ignorePlayer;
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

// Interiors
struct Interior {}; // consider making this a var on Shape{} instead
struct InteriorColliding {
    entt::entity interior;
};
struct Inside {
    entt::entity interior;
};
struct InteriorPortal {
    entt::entity A;
    entt::entity B;
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
    std::string name;
};
