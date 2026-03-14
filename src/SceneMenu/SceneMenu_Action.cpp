#include "../SceneMenu.h"
#include "GameEngine.h"
#include "Action.h"
#include "Components.h"

void SceneMenu::update()
{
    sf::Event event{};
    while (m_pGame->window().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            m_pGame->quit();
    }

    m_entityManager.update();
    m_fPulseTimer += 0.1f;

    if (m_bShowSplash)
    {
        m_fSplashTimer += 0.016f;
        if (m_fSplashTimer >= m_fSplashDuration)
        {
            m_bShowSplash = false;
            s_bHasShownSplash = true;
        }
    }

    sysRender();
}

void SceneMenu::setMenuState(MenuState newState)
{
    m_currentState = newState;
    m_nSelectedMenuIndex = 0;
}

void SceneMenu::navigateMenu(const std::string &action)
{
    if (action == "BACK")
    {
        m_pGame->playSound("return");
        if (m_currentState == START_GAME || m_currentState == OPTIONS_MAIN)
            setMenuState(MAIN_MENU);
        else if (m_currentState == OPTIONS_GAME || m_currentState == OPTIONS_AUDIO || m_currentState == OPTIONS_CONTROLLER)
            setMenuState(OPTIONS_MAIN);
        else if (m_currentState == OPTIONS_REMAP_KEYBOARD)
            setMenuState(OPTIONS_CONTROLLER);
        else if (m_currentState == OPTIONS_GAME || m_currentState == OPTIONS_VIDEO || m_currentState == OPTIONS_AUDIO || m_currentState == OPTIONS_CONTROLLER)
            setMenuState(OPTIONS_MAIN);
        else if (m_currentState == CREDITS_SCREEN || m_currentState == CONTROLS_SCREEN)
            setMenuState(MAIN_MENU);
    }
    else if (action == "START_GAME_MENU")
    {
        setMenuState(START_GAME);
        m_pGame->playSound("ui");
    }
    else if (action == "OPTIONS_MENU")
    {
        setMenuState(OPTIONS_MAIN);
        m_pGame->playSound("ui");
    }
    else if (action == "OPTIONS_GAME")
    {
        setMenuState(OPTIONS_GAME);
        m_pGame->playSound("ui");
    }
    else if (action == "OPTIONS_GAME")
    {
        setMenuState(OPTIONS_GAME);
        m_pGame->playSound("ui");
    }
    else if (action == "OPTIONS_VIDEO")
    {
        setMenuState(OPTIONS_VIDEO);
        m_pGame->playSound("ui");
    }
    else if (action == "OPTIONS_AUDIO")
    {
        setMenuState(OPTIONS_AUDIO);
        m_pGame->playSound("ui");
    }
    else if (action == "OPTIONS_CONTROLLER")
    {
        setMenuState(OPTIONS_CONTROLLER);
        m_pGame->playSound("ui");
    }
}

void SceneMenu::executeMenuAction(const std::string &action)
{
    if (action == "QUIT")
    {
        onEnd();
    }
    else if (action == "START_NEW")
    {
        m_pGame->playSound("apply");

        std::ofstream clearFile("save.csv", std::ios::trunc);
        clearFile.close();

        m_vecUnlockedLevels.clear();

        saveSettings();

        m_pGame->prepareLevelLoad("Assets/Levels/1 - Awakening.txt");
        m_pGame->changeScene("PLAY", nullptr, true);
    }
    else if (action == "LOAD_LEVEL")
    {
        m_pGame->playSound("apply");
        if (!m_vecLevelPaths.empty())
        {
            m_pGame->prepareLevelLoad(m_vecLevelPaths[m_nSelectedLevelIndex]);
            m_pGame->changeScene("PLAY", nullptr, true);
        }
    }
    else if (action == "EDITOR_MODE")
    {
        m_pGame->playSound("apply");
        if (!m_vecLevelPaths.empty())
        {
            m_pGame->prepareLevelLoad(m_vecLevelPaths[m_nSelectedEditorLevelIndex]);
            m_pGame->changeScene("EDITOR", nullptr, true);
        }
    }
    else if (action == "APPLY_SETTINGS" || action == "APPLY_CONTROLLER")
    {
        m_pGame->playSound("apply");
        m_pGame->updateMusicVolume(m_nMusicVolume);
        m_pGame->setSFXVolume(m_nSFXVolume);
        m_pGame->setVoiceVolume(m_nVoiceVolume);
        saveSettings();
        loadMenuConfigurations();
        navigateMenu("BACK");
    }
    else if (action == "REMAP_KEYBOARD")
    {
        m_bRemappingController = false;
        setMenuState(OPTIONS_REMAP_KEYBOARD);
    }
    else if (action == "CREDITS")
    {
        m_pGame->playSound("ui");
        setMenuState(CREDITS_SCREEN);
    }
    else if (action == "CONTROLS")
    {
        m_pGame->playSound("ui");
        setMenuState(CONTROLS_SCREEN);
    }
    else if (action.find("BIND_") == 0)
    {
        m_sRemapAction = action.substr(5);
        m_bWaitingForInput = true;
    }
    else if (action == "APPLY_VIDEO")
    {
        m_pGame->playSound("apply");
        m_pGame->setInternalResolution(m_vecResolutions[m_nSelectedResolution].x, m_vecResolutions[m_nSelectedResolution].y);
        m_pGame->setFullscreen(m_bSelectedFullscreen);
        m_pGame->applyVideoSettings();
        saveSettings();
        initAssets();
        navigateMenu("BACK");
    }
    else
    {
        navigateMenu(action);
    }
}

void SceneMenu::sysDoAction(const Action &action)
{
    if (m_currentState == CREDITS_SCREEN)
    {
        if (action.getName() == "LEFT_CLICK" ||
            action.getName() == "RIGHT_CLICK" ||
            action.getName() == "MOUSE_MOVE")
        {
            return;
        }

        if (action.getState() == "START")
        {
            navigateMenu("BACK");
            m_pGame->playSound("return");
        }
        return;
    }

    if (action.getState() == "START")
    {
        if (m_bShowSplash)
            return;

        if (m_currentState == CREDITS_SCREEN || m_currentState == CONTROLS_SCREEN)
        {
            navigateMenu("BACK");
            return;
        }

        if (m_bWaitingForInput)
        {
            if (!m_bRemappingController)
            {
                for (int k = 0; k < sf::Keyboard::KeyCount; k++)
                {
                    if (sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(k)))
                    {
                        m_pGame->setKey(m_sRemapAction, static_cast<sf::Keyboard::Key>(k));
                        m_bWaitingForInput = false;
                        loadMenuConfigurations();
                        return;
                    }
                }
            }
            return;
        }

        const auto &currentMenu = m_mapMenuOptions.at(m_currentState);

        if (action.getName() == "UP")
        {
            m_pGame->playSound("ui");
            if (m_nSelectedMenuIndex > 0)
                m_nSelectedMenuIndex--;
            else
                m_nSelectedMenuIndex = currentMenu.size() - 1;
        }
        else if (action.getName() == "DOWN")
        {
            m_pGame->playSound("ui");
            m_nSelectedMenuIndex = (m_nSelectedMenuIndex + 1) % currentMenu.size();
        }
        else if (action.getName() == "PLAY")
        {
            const std::string &currentAction = currentMenu.at(m_nSelectedMenuIndex).actionName;
            executeMenuAction(currentAction);
        }
        else if (action.getName() == "BACK")
        {
            if (m_bWaitingForInput)
                m_bWaitingForInput = false;
            else
                navigateMenu("BACK");
        }
        else if (action.getName() == "LEFT" || action.getName() == "RIGHT")
        {
            m_pGame->playSound("ui");
            const std::string &currentAction = currentMenu.at(m_nSelectedMenuIndex).actionName;

            if (m_currentState == OPTIONS_CONTROLLER && currentAction == "CYCLE_CONTROLLER")
            {
                int currentID = m_pGame->getJoystickID();
                if (action.getName() == "LEFT")
                    currentID = (currentID - 1 < 0) ? 7 : currentID - 1;
                else
                    currentID = (currentID + 1 > 7) ? 0 : currentID + 1;

                m_pGame->setJoystickID(currentID);
                loadMenuConfigurations();
            }
            else if (m_currentState == START_GAME && currentAction == "LOAD_LEVEL")
            {
                if (m_vecLevelNames.size() > 1)
                {
                    if (action.getName() == "LEFT")
                        m_nSelectedLevelIndex = (m_nSelectedLevelIndex == 0) ? m_vecLevelNames.size() - 1 : m_nSelectedLevelIndex - 1;
                    else
                        m_nSelectedLevelIndex = (m_nSelectedLevelIndex + 1) % m_vecLevelNames.size();
                }
            }
            else if (m_currentState == START_GAME && currentAction == "EDITOR_MODE")
            {
                if (m_vecLevelNames.size() > 1)
                {
                    if (action.getName() == "LEFT")
                        m_nSelectedEditorLevelIndex = (m_nSelectedEditorLevelIndex == 0) ? m_vecLevelNames.size() - 1 : m_nSelectedEditorLevelIndex - 1;
                    else
                        m_nSelectedEditorLevelIndex = (m_nSelectedEditorLevelIndex + 1) % m_vecLevelNames.size();
                }
            }
            else if (m_currentState == OPTIONS_GAME && currentAction == "DIFF_CHANGE")
            {
                if (action.getName() == "LEFT")
                    m_nSelectedDifficulty = (m_nSelectedDifficulty == 0) ? m_vecDifficulties.size() - 1 : m_nSelectedDifficulty - 1;
                else
                    m_nSelectedDifficulty = (m_nSelectedDifficulty + 1) % m_vecDifficulties.size();
            }
            else if (m_currentState == OPTIONS_CONTROLLER && currentAction == "TOGGLE_SWAP_ABXY")
            {
                bool currentState = m_pGame->getSwapABXY();
                m_pGame->setSwapABXY(!currentState);
                initActions();
            }
            else if (m_currentState == OPTIONS_AUDIO)
            {
                int *pVolume = nullptr;
                if (currentAction == "VOL_MUSIC")
                    pVolume = &m_nMusicVolume;
                else if (currentAction == "VOL_SFX")
                    pVolume = &m_nSFXVolume;
                else if (currentAction == "VOL_VOICE")
                    pVolume = &m_nVoiceVolume;

                if (pVolume)
                {
                    if (action.getName() == "LEFT")
                        *pVolume = std::max(0, *pVolume - 10);
                    else
                        *pVolume = std::min(100, *pVolume + 10);

                    if (currentAction == "VOL_MUSIC")
                    {
                        m_pGame->updateMusicVolume(m_nMusicVolume);
                    }
                    else if (currentAction == "VOL_SFX")
                    {
                        m_pGame->setSFXVolume(m_nSFXVolume);
                    }
                    else if (currentAction == "VOL_VOICE")
                    {
                        m_pGame->setVoiceVolume(m_nVoiceVolume);
                    }
                }
            }
            else if (m_currentState == OPTIONS_VIDEO && currentAction == "CYCLE_RES")
            {
                if (action.getName() == "LEFT")
                    m_nSelectedResolution = (m_nSelectedResolution == 0) ? m_vecResolutions.size() - 1 : m_nSelectedResolution - 1;
                else
                    m_nSelectedResolution = (m_nSelectedResolution + 1) % m_vecResolutions.size();
            }
            else if (m_currentState == OPTIONS_VIDEO && currentAction == "TOGGLE_FULLSCREEN")
            {
                m_bSelectedFullscreen = !m_bSelectedFullscreen;
            }
        }
    }
}

void SceneMenu::onEnd()
{
    m_pGame->quit();
}