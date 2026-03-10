#include "Entity.h"

Entity::Entity(const size_t &id, const std::string &entityTag)
    : m_strTag(entityTag), m_nID(id)
{
}

bool Entity::isActive() const
{
  return m_bIsActive;
}

void Entity::destroy()
{
  m_bIsActive = false;
}

size_t Entity::getID() const
{
  return m_nID;
}

const std::string &Entity::getTag() const
{
  return m_strTag;
}