#pragma once

#include "Action.h"
#include "EntityManager.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

class GameEngine;

class Scene
{
protected:
    GameEngine *m_pGame = nullptr;
    EntityManager m_entityManager;
    std::map<std::string, std::string> m_actionMap;
    bool m_bIsPaused = false;
    bool m_bHasEnded = false;
    size_t m_nCurrentFrame = 0;

    virtual void onEnd() = 0;
    void setPaused(bool a_bPaused);

public:
    Scene(GameEngine *a_pGame);
    virtual ~Scene();

    // --- Main Loop ---
    virtual void update() = 0;
    virtual void sysDoAction(const Action &a_action) = 0;
    virtual void sysRender() = 0;

    // --- Input Registration ---
    void registerAction(const std::string &inputName, const std::string &actionName);
    void registerAction(int keyCode, const std::string &actionName);
    const std::map<std::string, std::string> &getActionMap() const;

    // --- Helpers ---
    size_t width() const;
    size_t height() const;
    size_t currentFrame() const;
};