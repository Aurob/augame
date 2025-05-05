
#pragma once

#include "../lib/entt.hpp"
#include "../structs.hpp"
#include <SDL2/SDL.h>

extern float deltaTime;
extern int width, height;
extern GLfloat toplefttile[2];
extern GLfloat offsetValue[2];
extern float gridSpacingValue;
extern float defaultGSV;
extern entt::entity _player;
extern bool windowResized;

void updateOther(entt::registry &registry) {

    // auto interiors = registry.view<InteriorPortal, PhysicsBodyRect>();
    // for(auto e : interiors) {
    //     auto &interior = registry.get<InteriorPortal>(e);
    //     auto &body = registry.get<PhysicsBodyRect>(e).body;
    
    //     if(interior.locked) {
    //         if(body->ignore) {
    //             body->ignore = false;
    //         }
    //     }
    //     else {
    //         if(!body->ignore) {
    //             body->ignore = true;
    //         }
    //     }
    // }
}

// void checkCollisions(entt::registry &registry, entt::entity entity, Position &entityPosition, Shape &entityShape, std::vector<entt::entity> &_collidables, std::vector<Vector3f> &overlaps)
// {
//     auto collidables = registry.view<Collidable, Position, Shape, InView>();

//     for (auto _entity : collidables)
//     {
//         if (_entity == entity)
//             continue;

//         Position &_entityPosition = registry.get<Position>(_entity);
//         Shape &_entityShape = registry.get<Shape>(_entity);

//         bool skipCollide = false;
//         bool invert = false;

//         if (registry.any_of<Inside>(entity) && !registry.any_of<OnInteriorPortal>(entity))
//         {
//             auto interior = registry.get<Inside>(entity).interior;
//             if (interior == _entity)
//             {
//                 invert = true;
//             }
//         }

//         // If Collidable.ignorePlayer or Collidable.ignoreCollideAll is true, skip collision
//         if (registry.any_of<Collidable>(_entity))
//         {
//             auto &collidable = registry.get<Collidable>(_entity);
//             if (collidable.ignoreCollideAll || (collidable.ignorePlayer && entity == _player))
//             {
//                 continue;
//             }
//         }

//         Vector3f overlap = positionsCollide(entityPosition, entityShape, _entityPosition, _entityShape, invert);

//         if (overlap.x != 0 || overlap.y != 0 || overlap.z != 0)
//         {
//             if (registry.any_of<Interior>(_entity))
//             {
//                 if (registry.any_of<Inside>(entity))
//                 {
//                     auto interior = registry.get<Inside>(entity).interior;
//                     if (interior == _entity)
//                     {
//                         if (registry.any_of<OnInteriorPortal>(entity))
//                         {
//                             skipCollide = true;
//                         }
//                     }
//                     else
//                     {
//                         auto entityInside = registry.any_of<Inside>(entity) ? registry.get<Inside>(entity).interior : entt::null;
//                         auto _entityInside = registry.any_of<Inside>(_entity) ? registry.get<Inside>(_entity).interior : entt::null;
//                         if (entityInside != _entityInside)
//                         {
//                             skipCollide = true;
//                         }
//                     }
//                 }
//                 if (registry.any_of<OnInteriorPortal>(entity))
//                 {
//                     auto portal = registry.get<OnInteriorPortal>(entity).portal;
//                     auto portalInterior = registry.get<InteriorPortal>(portal).A;
//                     if (portalInterior == _entity)
//                     {
//                         skipCollide = true;
//                     }
//                 }
//             }

//             if (registry.any_of<InteriorPortal>(_entity))
//             {
//                 auto &interiorPortal = registry.get<InteriorPortal>(_entity);
//                 if(!interiorPortal.locked && !registry.any_of<OnInteriorPortal>(entity))
//                 {
//                     registry.emplace_or_replace<OnInteriorPortal>(entity, OnInteriorPortal{_entity});
//                     auto currentInterior = registry.any_of<Inside>(entity) ? registry.get<Inside>(entity).interior : entt::null;
//                     auto newInterior = (interiorPortal.A != currentInterior) ? interiorPortal.A : interiorPortal.B;
//                     if (newInterior != entt::null)
//                     {
//                         registry.emplace_or_replace<Inside>(entity, Inside{newInterior});
//                     }
//                     else
//                     {
//                         registry.remove<Inside>(entity);
//                     }
//                 }
//                 skipCollide = true;
//             }

//             if (!skipCollide)
//             {
//                 _collidables.push_back(_entity);
//                 overlaps.push_back(overlap);
//             }
//         }
//         else
//         {
//             if (registry.any_of<InteriorPortal>(_entity) && registry.any_of<OnInteriorPortal>(entity))
//             {
//                 auto portal = registry.get<OnInteriorPortal>(entity);
//                 if (portal.portal == _entity)
//                 {
//                     registry.remove<OnInteriorPortal>(entity);

//                     auto &interiorPortal = registry.get<InteriorPortal>(_entity);
//                     if (!registry.any_of<Inside>(entity))
//                     {
//                         registry.remove<OnInteriorPortal>(entity);
//                     }
//                     else
//                     {
//                         auto interior = registry.get<Inside>(entity).interior;
//                         if (interior != interiorPortal.A && interior != interiorPortal.B ||
//                             interiorPortal.A == entt::null || interiorPortal.B == entt::null)
//                         {
//                             registry.remove<OnInteriorPortal>(entity);
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

// void updateCollisions(entt::registry &registry)
// {
//     registry.sort<Collidable>([&](const entt::entity lhs, const entt::entity rhs)
//                               { return registry.any_of<Player>(lhs) && !registry.any_of<Player>(rhs); });

//     auto collidables = registry.view<Collidable, Position, Shape, InView>();

//     for (auto entity : collidables)
//     {
//         std::vector<entt::entity> _collidables;
//         std::vector<Vector3f> overlaps;

//         Shape &entityShape = registry.get<Shape>(entity);
//         Position &entityPosition = registry.get<Position>(entity);

//         checkCollisions(registry, entity, entityPosition, entityShape, _collidables, overlaps);

//         if (!_collidables.empty())
//         {
//             if (registry.any_of<Colliding>(entity))
//             {
//                 auto &colliding = registry.get<Colliding>(entity);
//             }
//             else
//             {
//                 registry.emplace<Colliding>(entity, Colliding{_collidables, overlaps});
//             }
            
//             if (registry.any_of<Movement>(entity)) {

//                 // offset entity position by overlaps
//                 for (auto &overlap : overlaps)
//                 {
//                     entityPosition.x += overlap.x;
//                     entityPosition.y += overlap.y;
//                 }
//             }

//             // Apply movement to colliding entities
//             for (auto _entity : _collidables)
//             {
//                 if (registry.any_of<Movement>(_entity) && registry.any_of<Movement>(entity))
//                 {
//                     auto &movement = registry.get<Movement>(entity);
//                     auto &_movement = registry.get<Movement>(_entity);

//                     if (registry.any_of<Moveable>(_entity))
//                     {
//                         _movement.velocity += movement.velocity * 0.5f;
//                         _movement.acceleration += movement.acceleration * 0.5f;
//                     }
//                 }

//                 if (registry.any_of<Collidable>(_entity))
//                 {
//                     auto &collidable = registry.get<Collidable>(_entity);
//                     if (!collidable.ignoreCollideAll)
//                     {
//                         collidable.colliding_with.push_back(entity);
//                     }
//                 }

//             }
//         }
//         else
//         {
//             registry.remove<Colliding>(entity);
//         }
//     }
// }