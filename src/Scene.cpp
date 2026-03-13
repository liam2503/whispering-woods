#include "Scene.h"
#include "GameEngine.h"
#include <iostream>

Scene::Scene(GameEngine *gameEngine)
    : m_pGame(gameEngine)
{
}

Scene::~Scene()
{
}

void Scene::setPaused(bool paused)
{
    m_bIsPaused = paused;
}

void Scene::registerAction(const std::string &inputName, const std::string &actionName)
{
    m_actionMap[inputName] = actionName;
}

void Scene::registerAction(int keyCode, const std::string &actionName)
{
    m_actionMap[std::to_string(keyCode)] = actionName;
}

const std::map<std::string, std::string> &Scene::getActionMap() const
{
    return m_actionMap;
}

size_t Scene::width() const
{
    return 1280;
}

size_t Scene::height() const
{
    return 720;
}

size_t Scene::currentFrame() const
{
    return m_nCurrentFrame;
}