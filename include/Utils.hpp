#pragma once
#include "structs.hpp"

extern entt::registry registry;

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


