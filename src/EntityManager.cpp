#include "EntityManager.h"
#include "Entity.h"

EntityManager::EntityManager() {}

EntityManager::~EntityManager()
{
    // Clear all entities on shutdown
    for (Entity *pEntity : m_vecEntities)
    {
        delete pEntity;
    }
    m_vecEntities.clear();
    m_mapEntities.clear();
}

// Helper function: Removes pointers from a list, but DOES NOT delete memory
void EntityManager::removeDeadEntities(EntityVec &a_vecEntities)
{
    a_vecEntities.erase(
        std::remove_if(a_vecEntities.begin(), a_vecEntities.end(),
            [](const Entity *entity) { 
                // We can safely check isActive() here because the memory is still valid
                return !entity->isActive(); 
            }),
        a_vecEntities.end());
}

void EntityManager::update()
{
    // 1. Add new entities to the lists
    for (auto &e : m_vecEntitiesToAdd)
    {
        m_vecEntities.push_back(e);
        m_mapEntities[e->getTag()].push_back(e);
    }
    m_vecEntitiesToAdd.clear();

    // 2. Clean up the MAPS first
    // We must remove pointers from the specific tag maps while the objects are still alive
    // so we can read their isActive() status without crashing.
    for (auto &[tag, entityVec] : m_mapEntities)
    {
        removeDeadEntities(entityVec);
    }

    // 3. Clean up the MAIN VECTOR and DELETE MEMORY
    // We cannot use removeDeadEntities here because we need to delete the memory too.
    auto it = m_vecEntities.begin();
    while (it != m_vecEntities.end())
    {
        Entity* e = *it;
        if (!e->isActive())
        {
            // Delete the memory
            delete e;
            // Remove the pointer from the main vector
            it = m_vecEntities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

Entity *EntityManager::addEntity(const std::string &a_strTag)
{
    // Assuming m_nTotalEntities is defined in your header
    Entity *entity = new Entity(m_nTotalEntities++, a_strTag);

    m_vecEntitiesToAdd.push_back(entity);

    return entity;
}

EntityVec &EntityManager::getEntities()
{
    return m_vecEntities;
}

EntityVec &EntityManager::getEntities(const std::string &a_strTag)
{
    return m_mapEntities[a_strTag];
}

// Fixed const correctness
const EntityVec &EntityManager::getEntities(const std::string &a_strTag) const
{
    auto it = m_mapEntities.find(a_strTag);
    if (it != m_mapEntities.end())
    {
        return it->second;
    }
    
    static EntityVec emptyVector;
    return emptyVector;
}