#include "Physics.h"
#include "Components.h"
#include "VectorPP.h"
#include <cmath>

VectorPP Physics::GetOverlap(Entity *a_pEntity1, Entity *a_pEntity2) const
{

    VectorPP entity1Pos = a_pEntity1->getComponent<CompTransform>().vPosition;
    VectorPP entity2Pos = a_pEntity2->getComponent<CompTransform>().vPosition;

    VectorPP entity1HalfSize = a_pEntity1->getComponent<CompBoundingBox>().vHalfSize;
    VectorPP entity2HalfSize = a_pEntity2->getComponent<CompBoundingBox>().vHalfSize;

    float deltaX = std::abs(entity2Pos.x - entity1Pos.x);
    float deltaY = std::abs(entity2Pos.y - entity1Pos.y);

    float overlapX = entity1HalfSize.x + entity2HalfSize.x - deltaX;
    float overlapY = entity1HalfSize.y + entity2HalfSize.y - deltaY;

    return VectorPP(overlapX, overlapY);
}

VectorPP Physics::GetPreviousOverlap(Entity *a_pEntity1, Entity *a_pEntity2)
{

    VectorPP entity1PrevPos = a_pEntity1->getComponent<CompTransform>().vPrevPos;
    VectorPP entity2PrevPos = a_pEntity2->getComponent<CompTransform>().vPrevPos;

    VectorPP entity1HalfSize = a_pEntity1->getComponent<CompBoundingBox>().vHalfSize;
    VectorPP entity2HalfSize = a_pEntity2->getComponent<CompBoundingBox>().vHalfSize;

    float deltaX = std::abs(entity2PrevPos.x - entity1PrevPos.x);
    float deltaY = std::abs(entity2PrevPos.y - entity1PrevPos.y);

    float overlapX = entity1HalfSize.x + entity2HalfSize.x - deltaX;
    float overlapY = entity1HalfSize.y + entity2HalfSize.y - deltaY;

    return VectorPP(overlapX, overlapY);
}

VectorPP Physics::GetClosestPoint(Entity *a_pEntity, Entity *a_pEntity2)
{

    VectorPP position = a_pEntity->getComponent<CompTransform>().vPosition;

    if (!a_pEntity2->hasComponent<CompLine>())
    {
        return position;
    }

    auto &line = a_pEntity2->getComponent<CompLine>();
    VectorPP pointA(
        line.verts[0].position.x,
        line.verts[0].position.y);
    VectorPP pointB(
        line.verts[1].position.x,
        line.verts[1].position.y);

    VectorPP ab = pointB - pointA;

    float abLenSq = ab.x * ab.x + ab.y * ab.y;

    if (abLenSq < 0.0001f)
    {
        // Just in case it's not even a real line
        return pointA;
    }

    VectorPP ap = position - pointA;

    float t = (ap.x * ab.x + ap.y * ab.y) / abLenSq;

    if (t < 0.f)
        t = 0.0f;
    else if (t > 1.f)
        t = 1.0f;

    return pointA + (ab * t);
}