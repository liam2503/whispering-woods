#pragma once

#include "Scene.h"
#include "GameEngine.h"
#include <vector>
#include <string>
#include <map>
#include <SFML/Graphics.hpp>

struct MenuOption
{
    std::string label;
    std::string actionName;
};

class SceneMenu : public Scene
{
public:
    enum MenuState
    {
        MAIN_MENU,
        START_GAME,
        OPTIONS_MAIN,
        OPTIONS_GAME,
        OPTIONS_VIDEO,
        OPTIONS_AUDIO,
        OPTIONS_CONTROLLER,
        OPTIONS_REMAP_KEYBOARD,
        CREDITS_SCREEN,
        CONTROLS_SCREEN
    };

    SceneMenu(GameEngine *a_pGame);

protected:
    // --- Core Scene Overrides ---
    void init();
    void update() override;
    void onEnd() override;
    void sysDoAction(const Action &a_action) override;
    void sysRender() override;

    // --- Initialization Helpers ---
    void initAssets();              
    void initActions();             
    void loadLevelList();           
    void loadMenuConfigurations();  

    // --- Logic ---
    void setMenuState(MenuState newState);
    void navigateMenu(const std::string &action);
    void executeMenuAction(const std::string &action);
    
    // --- Settings Persistence ---
    void loadSettings();
    void saveSettings();
    std::string getKeyName(sf::Keyboard::Key key); 

private:
    // --- UI Resources ---
    std::string m_strTitle;
    sf::Text m_menuText;      
    sf::Text m_titleText;     
    sf::Text m_subtitleText;
    sf::Text m_developerText;
    
    sf::Sprite m_backgroundSprite;
    sf::Sprite m_creditsSprite;
    sf::Texture m_creditsTexture;
    sf::Sprite m_controlsSprite;
    sf::Texture m_controlsTexture;

    // --- Splash Screen ---
    static bool s_bHasShownSplash;
    bool m_bShowSplash = false;
    float m_fSplashTimer = 0.0f;
    float m_fSplashDuration = 4.0f;
    sf::Texture m_splashTexture;
    sf::Sprite m_splashSprite;

    // --- State Data ---
    MenuState m_currentState = MAIN_MENU;
    std::map<MenuState, std::vector<MenuOption>> m_mapMenuOptions;
    size_t m_nSelectedMenuIndex = 0;
    float m_fPulseTimer = 0.0f;

    // --- Selection Data ---
    std::vector<std::string> m_vecLevelNames;
    std::vector<std::string> m_vecLevelPaths;
    std::vector<std::string> m_vecUnlockedLevels;
    size_t m_nSelectedLevelIndex = 0;
    size_t m_nSelectedEditorLevelIndex = 0;

    std::vector<std::string> m_vecDifficulties = {"Easy", "Normal", "Hard"};
    size_t m_nSelectedDifficulty = 1;
    
    int m_nMusicVolume = 100;
    int m_nSFXVolume = 100;
    int m_nVoiceVolume = 100;

    // --- Video Settings Data ---
    std::vector<sf::Vector2f> m_vecResolutions = {{1280, 720}};
    size_t m_nSelectedResolution = 1;
    bool m_bSelectedFullscreen = false;

    // --- Remapping State ---
    bool m_bWaitingForInput = false;
    bool m_bRemappingController = false;
    std::string m_sRemapAction = "";

    // --- Constants ---
    static const int CHAR_SIZE_TITLE = 48;
    static const int CHAR_SIZE_SUBTITLE = 36;
    static const int CHAR_SIZE_ITEM_MAIN = 48;
    static const int CHAR_SIZE_ITEM_SUB = 36;
    static const int CHAR_SIZE_ITEM_SMALL = 24;
    static const int CHAR_SIZE_FOOTER = 20;
};