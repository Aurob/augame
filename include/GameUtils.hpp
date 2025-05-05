#pragma once
#include "structs.hpp"

extern entt::registry registry;

Vector3f computeMTV(const Position &aPos, const Shape &aShape, 
                    const Position &bPos, const Shape &bShape)
{
    // Compute min and max for each axis
    float aMinX = aPos.x, aMaxX = aPos.x + aShape.size.x;
    float aMinY = aPos.y, aMaxY = aPos.y + aShape.size.y;
    float aMinZ = aPos.z, aMaxZ = aPos.z + aShape.size.z;

    float bMinX = bPos.x, bMaxX = bPos.x + bShape.size.x;
    float bMinY = bPos.y, bMaxY = bPos.y + bShape.size.y;
    float bMinZ = bPos.z, bMaxZ = bPos.z + bShape.size.z;

    // Compute penetration depths
    float overlapX = std::min(aMaxX, bMaxX) - std::max(aMinX, bMinX);
    float overlapY = std::min(aMaxY, bMaxY) - std::max(aMinY, bMinY);
    float overlapZ = std::min(aMaxZ, bMaxZ) - std::max(aMinZ, bMinZ);

    // If there is no collision, return zero vector
    if (overlapX <= 0 || overlapY <= 0 || overlapZ <= 0)
        return {0.f,0.f,0.f};

    // Choose the axis with the smallest penetration
    if (overlapX < overlapY && overlapX < overlapZ)
    {
        // Determine proper sign from the relative centers
        float sign = (aPos.x + aShape.size.x * 0.5f < bPos.x + bShape.size.x * 0.5f) ? -1.f : 1.f;
        return Vector3f{sign * overlapX, 0.f, 0.f};
    }
    else if (overlapY < overlapZ)
    {
        float sign = (aPos.y + aShape.size.y * 0.5f < bPos.y + bShape.size.y * 0.5f) ? -1.f : 1.f;
        return Vector3f{0.f, sign * overlapY, 0.f};
    }
    else
    {
        float sign = (aPos.z + aShape.size.z * 0.5f < bPos.z + bShape.size.z * 0.5f) ? -1.f : 1.f;
        return Vector3f{0.f, 0.f, sign * overlapZ};
    }
}

Vector3f calculateOverlap(float AxT, float AxB, float BxT, float BxB, 
                          float AyT, float AyB, float ByT, float ByB,
                          float AzT, float AzB, float BzT, float BzB)
{
    float xOverlapAmount = std::min(AxB - BxT, BxB - AxT);
    float yOverlapAmount = std::min(AyB - ByT, ByB - AyT);
    float zOverlapAmount = std::min(AzB - BzT, BzB - AzT);
    return Vector3f{xOverlapAmount, yOverlapAmount, zOverlapAmount};
}

Vector3f calculateMoveDirection(float xOverlapAmount, float yOverlapAmount, float zOverlapAmount,
                                float AxB, float BxT, float BxB, float AxT,
                                float AyB, float ByT, float ByB, float AyT,
                                float AzB, float BzT, float BzB, float AzT)
{
    Vector3f moveDirection{0.f, 0.f, 0.f};
    if (xOverlapAmount < yOverlapAmount && xOverlapAmount < zOverlapAmount)
    {
        moveDirection.x = (AxB - BxT < BxB - AxT) ? -xOverlapAmount : xOverlapAmount;
    }
    else if (yOverlapAmount < zOverlapAmount)
    {
        moveDirection.y = (AyB - ByT < ByB - AyT) ? -yOverlapAmount : yOverlapAmount;
    }
    else
    {
        moveDirection.z = (AzB - BzT < BzB - AzT) ? -zOverlapAmount : zOverlapAmount;
    }
    return moveDirection;
}

Vector3f positionsCollide(const Position &pos1, const Shape &shape1, 
                          const Position &pos2, const Shape &shape2, bool invert)
{
    Vector3f moveDirection{0.f, 0.f, 0.f};
    float Ax = pos1.x, Ay = pos1.y, Az = pos1.z;
    float Bx = pos2.x, By = pos2.y, Bz = pos2.z;

    float Aw = shape1.size.x, Ah = shape1.size.y, Ad = shape1.size.z;
    float Bw = shape2.size.x, Bh = shape2.size.y, Bd = shape2.size.z;

    float AxB = Ax + Aw, AxT = Ax, AyT = Ay, AyB = Ay + Ah, AzT = Az, AzB = Az + Ad;
    float BxB = Bx + Bw, BxT = Bx, ByT = By, ByB = By + Bh, BzT = Bz, BzB = Bz + Bd;

    if (invert) {
        // For inverted collisions (inside the interior)
        if (AxT <= BxT) moveDirection.x = BxT - AxT;
        else if (AxB >= BxB) moveDirection.x = BxB - AxB;

        if (AyT <= ByT) moveDirection.y = ByT - AyT;
        else if (AyB >= ByB) moveDirection.y = ByB - AyB;

        if (AzT <= BzT) moveDirection.z = BzT - AzT;
        else if (AzB >= BzB) moveDirection.z = BzB - AzB;
    } else {
        // For regular collisions (outside the interior)
        bool xOverlap = (AxT < BxB && AxB > BxT);
        bool yOverlap = (AyT < ByB && AyB > ByT);
        bool zOverlap = (AzT < BzB && AzB > BzT);

        if (xOverlap && yOverlap && zOverlap) {
            Vector3f overlap = calculateOverlap(AxT, AxB, BxT, BxB, AyT, AyB, ByT, ByB, AzT, AzB, BzT, BzB);
            moveDirection = calculateMoveDirection(overlap.x, overlap.y, overlap.z,
                                                   AxB, BxT, BxB, AxT,
                                                   AyB, ByT, ByB, AyT,
                                                   AzB, BzT, BzB, AzT);
        }
    }

    return moveDirection;
}


bool basicCollisionCheck(entt::entity e1, entt::entity e2) {
    // Retrieve positions and shapes from the registry
    auto &collidablePos = registry.get<Position>(e1);
    auto &collidableShape = registry.get<Shape>(e1);
    auto &doorPos = registry.get<Position>(e2);
    auto &doorShape = registry.get<Shape>(e2);

    // Calculate AABB for collidable entity
    float collidableMinX = collidablePos.x;
    float collidableMaxX = collidablePos.x + collidableShape.size.x;
    float collidableMinY = collidablePos.y;
    float collidableMaxY = collidablePos.y + collidableShape.size.y;

    // Calculate AABB for door entity
    float doorMinX = doorPos.x;
    float doorMaxX = doorPos.x + doorShape.size.x;
    float doorMinY = doorPos.y;
    float doorMaxY = doorPos.y + doorShape.size.y;

    // Check for AABB collision
    return (collidableMinX < doorMaxX && collidableMaxX > doorMinX) &&
           (collidableMinY < doorMaxY && collidableMaxY > doorMinY);
}