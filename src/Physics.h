#pragma once
#include "Entity.h"
#include "Common.h"

class Physics
{
public:
    VectorPP GetOverlap(Entity *a_pEntity1, Entity *a_pEntity2) const;
    VectorPP GetPreviousOverlap(Entity *a_pEntity1, Entity *a_pEntity2);
    VectorPP GetClosestPoint(Entity *a_pEntity, Entity *a_pEntity2);
};