#pragma once

#include <vector>

using namespace std;

struct context
{
    SDL_Event event;
    int iteration;
    SDL_Window *window;
};

struct Vector2f {
    float x, y;
};

struct Vector3f {
    float x, y, z;
};

// Entity Components
struct Player {};
struct Position {
    float x;
    float y;
    float sx;
    float sy;
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

struct Hoverable {};
struct Hovered {};

struct Interactable {
    int interactions;
    bool toggle;
};
struct Interacted {
    entt::entity interactor;
};

struct Colliding{
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
};
struct Moveable {};


struct Linkable {};
struct Linked {
    entt::entity parent;
    float distance;
    bool keepCollisions{false};
};

// Interiors
struct Interior {
    Vector2f entryPoint;
}; // consider making this a var on Shape{} instead
struct InteriorColliding {};
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