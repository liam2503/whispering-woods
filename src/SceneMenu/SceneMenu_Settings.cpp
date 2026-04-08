#include "../SceneMenu.h"
#include "GameEngine.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "Common.h"

void SceneMenu::loadSettings()
{
    m_vecUnlockedLevels.clear();
    std::ifstream infile(getSaveFilePath());
    if (!infile.is_open())
        return;

    std::string line, key, value;
    while (std::getline(infile, line))
    {
        std::stringstream ss(line);
        if (std::getline(ss, key, ',') && std::getline(ss, value))
        {
            try
            {
                if (key == "MusicVolume")
                    m_nMusicVolume = std::stoi(value);
                else if (key == "SFXVolume")
                    m_nSFXVolume = std::stoi(value);
                else if (key == "VoiceVolume")
                    m_nVoiceVolume = std::stoi(value);
                else if (key == "JoystickID")
                    m_pGame->setJoystickID(std::stoi(value));
                else if (key == "SwapABXY")
                {
                    m_pGame->setSwapABXY(std::stoi(value) != 0);
                }
                else if (key.find("KEY_") == 0)
                {
                    m_pGame->setKey(key.substr(4), static_cast<sf::Keyboard::Key>(std::stoi(value)));
                }
                else if (key.find("BTN_") == 0)
                {
                    m_pGame->setControllerButton(key.substr(4), std::stoi(value));
                }
                else if (key == "InternalResX")
                {
                    float rx = std::stof(value);
                    for (size_t i = 0; i < m_vecResolutions.size(); i++) {
                        if (m_vecResolutions[i].x == rx) m_nSelectedResolution = i;
                    }
                    m_pGame->setInternalResolution(rx, m_vecResolutions[m_nSelectedResolution].y);
                }
                else if (key == "InternalResY")
                {
                    m_pGame->setInternalResolution(m_vecResolutions[m_nSelectedResolution].x, std::stof(value));
                }
                else if (key == "Fullscreen")
                {
                    m_bSelectedFullscreen = (std::stoi(value) != 0);
                    m_pGame->setFullscreen(m_bSelectedFullscreen);
                    m_pGame->applyVideoSettings();
                }
            }
            catch (...)
            {
                std::cerr << "Settings Load Error parsing line: " << line << "\n";
            }
        }
        else
        {
            if (!key.empty() && key.find("Assets/Levels") != std::string::npos)
            {
                key.erase(std::remove(key.begin(), key.end(), '\r'), key.end());
                m_vecUnlockedLevels.push_back(key);
            }
        }
    }
    m_pGame->setSwapABXY(m_pGame->getSwapABXY());
}

void SceneMenu::saveSettings()
{
    bool hasBoomerang = false;

    std::ifstream infile(getSaveFilePath());

    if (infile.is_open())
    {
        std::string line;
        while (std::getline(infile, line))
        {
            if (line.find("BoomerangUnlocked,1") == 0)
                hasBoomerang = true;
        }
        infile.close();
    }

    std::ofstream outfile(getSaveFilePath());
    if (outfile.is_open())
    {
        if (hasBoomerang)
            outfile << "BoomerangUnlocked,1\n";
        outfile << "Difficulty," << m_vecDifficulties[m_nSelectedDifficulty] << "\n";
        outfile << "MusicVolume," << m_nMusicVolume << "\n";
        outfile << "SFXVolume," << m_nSFXVolume << "\n";
        outfile << "VoiceVolume," << m_nVoiceVolume << "\n";
        outfile << "JoystickID," << m_pGame->getJoystickID() << "\n";
        outfile << "SwapABXY," << m_pGame->getSwapABXY() << "\n";

        outfile << "KEY_MOVE_LEFT," << m_pGame->getKey("MOVE_LEFT") << "\n";
        outfile << "KEY_MOVE_RIGHT," << m_pGame->getKey("MOVE_RIGHT") << "\n";
        outfile << "KEY_JUMP," << m_pGame->getKey("JUMP") << "\n";
        outfile << "KEY_BURROW," << m_pGame->getKey("BURROW") << "\n";
        outfile << "KEY_MELEE," << m_pGame->getKey("MELEE") << "\n";
        outfile << "KEY_SHOOT," << m_pGame->getKey("SHOOT") << "\n";
        outfile << "KEY_BOOMERANG," << m_pGame->getKey("BOOMERANG") << "\n";
        outfile << "KEY_MENU," << m_pGame->getKey("MENU") << "\n";

        outfile << "BTN_MELEE," << m_pGame->getControllerButton("MELEE") << "\n";
        outfile << "BTN_BOOMERANG," << m_pGame->getControllerButton("BOOMERANG") << "\n";
        outfile << "BTN_SELECT," << m_pGame->getControllerButton("SELECT") << "\n";
        outfile << "BTN_CANCEL," << m_pGame->getControllerButton("CANCEL") << "\n";

        std::sort(m_vecUnlockedLevels.begin(), m_vecUnlockedLevels.end());
        auto last = std::unique(m_vecUnlockedLevels.begin(), m_vecUnlockedLevels.end());
        m_vecUnlockedLevels.erase(last, m_vecUnlockedLevels.end());

        for (const auto &level : m_vecUnlockedLevels)
        {
            outfile << level << "\n";
        }

        outfile << "InternalResX," << m_vecResolutions[m_nSelectedResolution].x << "\n";
        outfile << "InternalResY," << m_vecResolutions[m_nSelectedResolution].y << "\n";
        outfile << "Fullscreen," << m_bSelectedFullscreen << "\n";

    }
}

std::string SceneMenu::getKeyName(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Unknown)
        return "LMB";
    if (key >= sf::Keyboard::A && key <= sf::Keyboard::Z)
        return std::string(1, (char)('A' + key));
    if (key == sf::Keyboard::Space)
        return "Space";
    if (key == sf::Keyboard::Enter)
        return "Enter";
    if (key == sf::Keyboard::Escape)
        return "Esc";
    return "Key " + std::to_string(key);
}