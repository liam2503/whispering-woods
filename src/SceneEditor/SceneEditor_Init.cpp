#include "../SceneEditor.h"
#include "GameEngine.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

SceneEditor::SceneEditor(GameEngine *gameEngine, const std::string &a_strLevelPath)
    : Scene(gameEngine), m_strLevelPath(a_strLevelPath)
{
    init();
}


void SceneEditor::init()
{
    sf::Vector2u windowSize = m_pGame->window().getSize();
    m_worldView.setSize((float)windowSize.x, (float)windowSize.y);
    m_worldView.setCenter((float)windowSize.x / 2.0f, (float)windowSize.y / 2.0f);

    m_hudView = m_pGame->window().getDefaultView();
    m_pGame->window().setMouseCursorVisible(false);

    if (m_cursorTexture.loadFromFile("Assets/Images/Animations/cursor.png"))
    {
        m_cursorSprite.setTexture(m_cursorTexture);
        m_cursorSprite.setOrigin(0.0f, 0.0f);
        m_cursorLoaded = true;
    }

    m_vMousePos = VectorPP(windowSize.x / 2.0f, windowSize.y / 2.0f);
    m_cursorSprite.setPosition(m_vMousePos.x, m_vMousePos.y);

    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("MENU")), "QUIT");
    registerAction(sf::Keyboard::W, "CAM_UP");
    registerAction(sf::Keyboard::A, "CAM_LEFT");
    registerAction(sf::Keyboard::S, "CAM_DOWN");
    registerAction(sf::Keyboard::D, "CAM_RIGHT");

    registerAction("DPAD_UP", "CAM_UP");
    registerAction("DPAD_LEFT", "CAM_LEFT");
    registerAction("DPAD_DOWN", "CAM_DOWN");
    registerAction("DPAD_RIGHT", "CAM_RIGHT");

    registerAction("RS_UP", "CAM_UP");
    registerAction("RS_LEFT", "CAM_LEFT");
    registerAction("RS_DOWN", "CAM_DOWN");
    registerAction("RS_RIGHT", "CAM_RIGHT");

    registerAction("LEFT_CLICK", "LEFT_CLICK");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("SELECT")), "LEFT_CLICK");

    registerAction("RIGHT_CLICK", "RIGHT_CLICK");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("CANCEL")), "RIGHT_CLICK");

    registerAction(sf::Keyboard::E, "TOGGLE_ERASER");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("ERASER")), "TOGGLE_ERASER");

    m_cameraSpeed = 15.0f;

    m_gridText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_gridText.setCharacterSize(12);

    m_statusText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_statusText.setCharacterSize(32);
    m_statusText.setFillColor(sf::Color(0xFFffae00));

    m_selectorText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_selectorText.setCharacterSize(18);
    m_selectorText.setFillColor(sf::Color::White);

    if (m_editorControllerTexture.loadFromFile("Assets/Images/controller2.png"))
    {
        m_editorControllerSprite.setTexture(m_editorControllerTexture);
        m_editorControllerSprite.setScale(0.2f, 0.2f);
        
        sf::Vector2u texSize = m_editorControllerTexture.getSize();
        m_editorControllerSprite.setPosition(40.0f, 660.0f);
    }

    populateSelectors();
    loadLevel(m_strLevelPath);
}

void SceneEditor::populateSelectors()
{
    m_bgOptions = {
        "Dark_Cave",
        "Silent_Woods",
        "Mysterious_Forest",
        "Somewhere_Dangerous"};

    m_musicOptions.clear();
    std::string musicPath = "Assets/Music";
    if (fs::exists(musicPath))
    {
        for (const auto &entry : fs::directory_iterator(musicPath))
        {
            if (entry.is_regular_file())
                m_musicOptions.push_back(entry.path().filename().string());
        }
    }
    if (m_musicOptions.empty())
        m_musicOptions.push_back("Title_Theme.ogg");

    m_levelOptions.clear();
    std::string levelPath = "Assets/Levels";
    if (fs::exists(levelPath))
    {
        for (const auto &entry : fs::directory_iterator(levelPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".txt")
                m_levelOptions.push_back("Assets/Levels/" + entry.path().filename().string());
        }
    }
    if (m_levelOptions.empty())
        m_levelOptions.push_back("Assets/Levels/Default.txt");

    m_cutsceneOptions.clear();
    std::string csPath = "Assets/Cutscenes";
    if (fs::exists(csPath))
    {
        for (const auto &entry : fs::directory_iterator(csPath))
        {
            if (entry.is_regular_file())
                m_cutsceneOptions.push_back("Assets/Cutscenes/" + entry.path().filename().string());
        }
    }
    if (m_cutsceneOptions.empty())
        m_cutsceneOptions.push_back("Assets/Cutscenes/Default.txt");
}