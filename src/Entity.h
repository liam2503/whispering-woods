#pragma once
#include "Components.h"
#include <tuple>
#include <string>

class EntityManager;

// --- Component Registry ---
typedef std::tuple<
    CompTransform,
    CompLifeSpan,
    CompInput,
    CompBoundingBox,
    CompAnimation,
    CompGravity,
    CompState,
    CompHealth,
    CompInvincible,
    CompChangeDirectionTimer,
    CompStatus,
    CompMana,
    CompLine,
    CompFlipScale,
    CompBoomerang,
    CompCutscene,
    CompDamage,
    CompFallingTile,
    CompLayer,
    CompChunk
> ComponentsTuple;

// --- Entity Class ---
class Entity
{
    friend class EntityManager;

private:
    bool m_bIsActive = true;
    std::string m_strTag = "default";
    size_t m_nID = 0;
    ComponentsTuple m_components;

    // Private constructor
    Entity(const size_t &a_nID, const std::string &a_strTag);

public:
    // --- Lifecycle ---
    void destroy();
    bool isActive() const;

    // --- Identifiers ---
    size_t getID() const;
    const std::string &getTag() const;

    // --- Component Management Templates ---
    template <typename T>
    bool hasComponent() const
    {
        return getComponent<T>().has;
    }

    template <typename T, typename... TArgs>
    T &addComponents(TArgs &&...mArgs)
    {
        auto &component = getComponent<T>();
        component = T(std::forward<TArgs>(mArgs)...);
        component.has = true;
        return component;
    }

    template <typename T>
    T &getComponent()
    {
        return std::get<T>(m_components);
    }

    template <typename T>
    const T &getComponent() const
    {
        return std::get<T>(m_components);
    }

    template <typename T>
    void removeComponent()
    {
        getComponent<T>() = T();
    }
};