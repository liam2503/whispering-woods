#include "../SceneMenu.h"
#include "GameEngine.h"
#include "Assets.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

// Static initialization
bool SceneMenu::s_bHasShownSplash = false;

SceneMenu::SceneMenu(GameEngine *game)
    : Scene(game)
{
    init();
}

void SceneMenu::init()
{
    loadSettings();
    initAssets();
    initActions();
    loadLevelList();
    loadMenuConfigurations();

    m_fPulseTimer = 0.0f;

    m_pGame->updateMusicVolume(m_nMusicVolume);
    m_pGame->setSFXVolume(m_nSFXVolume);
    m_pGame->setVoiceVolume(m_nVoiceVolume);
    m_pGame->requestMusic("Assets/Music/Title_Theme.ogg", (float)m_nMusicVolume);

    setMenuState(MAIN_MENU);
}

void SceneMenu::initAssets()
{
    // --- Version Loading ---
    std::string versionNum = "Unknown";
    std::ifstream versionFile("version.txt");
    if (versionFile.is_open())
    {
        versionFile >> versionNum;
        versionFile.close();
    }

    // --- Text Setup ---
    m_menuText.setFont(m_pGame->getAssets().getFont("Harnold"));
    m_menuText.setCharacterSize(CHAR_SIZE_ITEM_MAIN);

    // Main Title
    m_strTitle = "The Whispering Wood";
    m_titleText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_titleText.setString(m_strTitle);
    m_titleText.setCharacterSize(CHAR_SIZE_TITLE);
    m_titleText.setFillColor(sf::Color(0xFFffae00));

    // Sub-menu Titles
    m_subtitleText.setFont(m_pGame->getAssets().getFont("Harnold"));
    m_subtitleText.setCharacterSize(CHAR_SIZE_SUBTITLE);
    m_subtitleText.setFillColor(sf::Color::White);

    // Developer Footer Text
    m_developerText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_developerText.setString("Developed by Team Kitsune     Version " + versionNum);
    m_developerText.setCharacterSize(CHAR_SIZE_FOOTER);
    m_developerText.setFillColor(sf::Color::White);

    // --- Graphics Setup ---
    const sf::Texture &bgTexture = m_pGame->getAssets().getTexture("TexMenuBG");
    m_backgroundSprite.setTexture(bgTexture);

    // Scale background to fit window
    sf::Vector2u texSize = bgTexture.getSize();
    float scaleX = (float)width() / texSize.x;
    float scaleY = (float)height() / texSize.y;
    m_backgroundSprite.setScale(scaleX, scaleY);

    // Load static screens (Credits/Controls)
    if (m_creditsTexture.loadFromFile("Assets/Images/Menu/Credits.png"))
    {
        m_creditsSprite.setTexture(m_creditsTexture);
        sf::Vector2u credSize = m_creditsTexture.getSize();
        m_creditsSprite.setScale((float)width() / credSize.x, (float)height() / credSize.y);
    }

    if (m_controlsTexture.loadFromFile("Assets/Images/Menu/Controls.png"))
    {
        m_controlsSprite.setTexture(m_controlsTexture);
        sf::Vector2u ctrlSize = m_controlsTexture.getSize();
        m_controlsSprite.setScale((float)width() / ctrlSize.x, (float)height() / ctrlSize.y);
    }

    // Handle Splash Screen Logic
    if (!s_bHasShownSplash)
    {
        if (m_splashTexture.loadFromFile("Assets/Images/Menu/logo_bg.png"))
        {
            m_splashSprite.setTexture(m_splashTexture);
            sf::Vector2u splashSize = m_splashTexture.getSize();
            m_splashSprite.setScale((float)width() / splashSize.x, (float)height() / splashSize.y);

            m_bShowSplash = true;
            m_fSplashTimer = 0.0f;
        }
        else
        {
            s_bHasShownSplash = true;
        }
    }
}

void SceneMenu::initActions()
{
    // Keyboard Navigation
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::Up, "UP");
    registerAction(sf::Keyboard::Down, "DOWN");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Left, "LEFT");
    registerAction(sf::Keyboard::Right, "RIGHT");
    registerAction(sf::Keyboard::Enter, "PLAY");
    registerAction(sf::Keyboard::Escape, "BACK");

    // Controller Navigation
    registerAction("LS_UP", "UP");
    registerAction("LS_DOWN", "DOWN");
    registerAction("LS_LEFT", "LEFT");
    registerAction("LS_RIGHT", "RIGHT");
    registerAction("DPAD_UP", "UP");
    registerAction("DPAD_DOWN", "DOWN");
    registerAction("DPAD_LEFT", "LEFT");
    registerAction("DPAD_RIGHT", "RIGHT");

    // Controller Buttons
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("SELECT")), "PLAY");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("CANCEL")), "BACK");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("MENU")), "BACK");
}

void SceneMenu::loadLevelList()
{
    // Ensure default unlocked state for Level 1
    std::string level1Path = "Assets/Levels/1 - Awakening.txt";
    std::replace(level1Path.begin(), level1Path.end(), '\\', '/');

    m_vecLevelNames.clear();
    m_vecLevelPaths.clear();
    std::string searchPath = "Assets/Levels";

    if (fs::exists(searchPath) && fs::is_directory(searchPath))
    {
        for (const auto &entry : fs::directory_iterator(searchPath))
        {
            std::string filename = entry.path().filename().string();
            if (entry.path().extension() == ".txt" && filename.find("CS_") != 0)
            {
                std::string fullPath = entry.path().string();
                std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                bool isUnlocked = (fullPath == level1Path);
                // Check against unlocked levels loaded from save file
                if (!isUnlocked)
                {
                    for (const auto &unlocked : m_vecUnlockedLevels)
                    {
                        std::string standardUnlocked = unlocked;
                        std::replace(standardUnlocked.begin(), standardUnlocked.end(), '\\', '/');
                        if (fullPath == standardUnlocked)
                        {
                            isUnlocked = true;
                            break;
                        }
                    }
                }

                if (isUnlocked)
                {
                    m_vecLevelPaths.push_back(entry.path().string());
                    m_vecLevelNames.push_back(entry.path().stem().string());
                }
            }
        }
    }

    if (m_vecLevelNames.empty())
    {
        m_vecLevelNames.push_back("No Levels Found");
        m_vecLevelPaths.push_back("");
    }
}

void SceneMenu::loadMenuConfigurations()
{
    m_mapMenuOptions[MAIN_MENU] = {
        {"Start Game", "START_GAME_MENU"},
        {"Options", "OPTIONS_MENU"},
        {"Credits", "CREDITS"},
        {"Quit Game", "QUIT"}};

    m_mapMenuOptions[START_GAME] = {
        {"Start New Game", "START_NEW"},
        {"Select Level:", "LOAD_LEVEL"},
        {"Level Editor", "EDITOR_MODE"},
        {"Return", "BACK"}};

    m_mapMenuOptions[OPTIONS_MAIN] = {
        {"Game Settings", "OPTIONS_GAME"},
        {"Video Settings", "OPTIONS_VIDEO"},
        {"Audio Settings", "OPTIONS_AUDIO"},
        {"Controller Settings", "OPTIONS_CONTROLLER"},
        {"Return", "BACK"}};

        m_mapMenuOptions[OPTIONS_VIDEO] = {
        {"Resolution:", "CYCLE_RES"},
        {"Fullscreen:", "TOGGLE_FULLSCREEN"},
        {"Apply", "APPLY_VIDEO"},
        {"Return", "BACK"}};

    m_mapMenuOptions[OPTIONS_GAME] = {
        {"Game Difficulty:", "DIFF_CHANGE"},
        {"Apply", "APPLY_SETTINGS"},
        {"Return", "BACK"}};

    m_mapMenuOptions[OPTIONS_AUDIO] = {
        {"Music Volume:", "VOL_MUSIC"},
        {"SFX Volume:", "VOL_SFX"},
        {"Voice Volume:", "VOL_VOICE"},
        {"Apply", "APPLY_SETTINGS"},
        {"Return", "BACK"}};

    std::string cName = std::to_string(m_pGame->getJoystickID()) + " (" + m_pGame->getJoystickName() + ")";
    m_mapMenuOptions[OPTIONS_CONTROLLER] = {
        {"Controller: < " + cName + " >", "CYCLE_CONTROLLER"},
        {"Swap A/B & X/Y:", "TOGGLE_SWAP_ABXY"},
        {"Remap Keyboard", "REMAP_KEYBOARD"},
        {"Apply", "APPLY_CONTROLLER"},
        {"Return", "BACK"}};

    m_mapMenuOptions[OPTIONS_REMAP_KEYBOARD] = {
        {"Left: " + getKeyName(m_pGame->getKey("MOVE_LEFT")), "BIND_MOVE_LEFT"},
        {"Right: " + getKeyName(m_pGame->getKey("MOVE_RIGHT")), "BIND_MOVE_RIGHT"},
        {"Jump: " + getKeyName(m_pGame->getKey("JUMP")), "BIND_JUMP"},
        {"Burrow: " + getKeyName(m_pGame->getKey("BURROW")), "BIND_BURROW"},
        {"Melee: " + getKeyName(m_pGame->getKey("MELEE")), "BIND_MELEE"},
        {"Shoot: " + getKeyName(m_pGame->getKey("SHOOT")), "BIND_SHOOT"},
        {"Boomerang: " + getKeyName(m_pGame->getKey("BOOMERANG")), "BIND_BOOMERANG"},
        {"Menu: " + getKeyName(m_pGame->getKey("MENU")), "BIND_MENU"},
        {"Return", "BACK"}};

    m_mapMenuOptions[CREDITS_SCREEN] = {};
    m_mapMenuOptions[CONTROLS_SCREEN] = {};
}