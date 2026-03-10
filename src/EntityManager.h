#pragma once

#include "Entity.h"
#include <vector>
#include <map>
#include <memory>

typedef std::vector<Entity *> EntityVec;

class EntityManager
{
private:
    EntityVec m_vecEntities;
    EntityVec m_vecEntitiesToAdd;
    std::map<std::string, EntityVec> m_mapEntities;
    size_t m_nTotalEntities = 0;

    void removeDeadEntities(EntityVec &a_vecEntities);

public:
    EntityManager();
    ~EntityManager();

    void update();
    
    // --- Factory ---
    Entity *addEntity(const std::string &a_strTag);

    // --- Accessors ---
    EntityVec &getEntities();
    EntityVec &getEntities(const std::string &a_strType);
    const EntityVec &getEntities(const std::string &a_strType) const;
};