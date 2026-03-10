#pragma once
#include "Scene.h"
#include "GameEngine.h"
#include "EntityManager.h"

class SceneEditor : public Scene
{
public:
    SceneEditor(GameEngine *a_pGame, const std::string &a_strLevelPath);

    void init();
    void update() override;
    void onEnd() override;
    void sysDoAction(const Action &a_action) override;

    std::map<VectorPP, std::string> m_tilePaths;
    std::string m_strGlobalNextLevel = "";

private:
    // --- Level Data ---
    std::string m_strLevelPath;
    std::string m_strBackgroundTexture = "Mysterious_Forest";
    std::string m_savedMusicPath = "Title_Theme.ogg";
    std::string m_savedNextLevelPath = "Assets/Levels/Default.txt";
    std::map<Entity *, std::string> m_entityScripts;

    // --- Editor View & State ---
    sf::View m_worldView;
    sf::View m_hudView;
    sf::Sprite m_backgroundSprite;
    
    float m_cameraSpeed;
    bool m_bCamLeft = false;
    bool m_bCamRight = false;
    bool m_bCamUp = false;
    bool m_bCamDown = false;

    // --- Tools & Options ---
    bool m_bDrawGrid = true;
    bool m_bDrawTextures = true;
    bool m_bDrawCollision = true;
    bool m_bEraserMode = false;
    std::string m_selectedTile;
    VectorPP m_vGridSize = {64, 64};

    // --- UI Elements ---
    sf::Text m_statusText;
    int m_statusFrames = 0;
    sf::Text m_gridText;
    sf::Text m_selectorText;
    
    // Cursor
    VectorPP m_vMousePos;
    sf::Sprite m_cursorSprite;
    sf::Texture m_cursorTexture;
    bool m_cursorLoaded = false;
    
    sf::Texture m_editorControllerTexture;
    sf::Sprite m_editorControllerSprite;

    // --- Selector Lists ---
    std::vector<std::string> m_bgOptions;
    size_t m_bgIndex = 0;
    std::vector<std::string> m_musicOptions;
    size_t m_musicIndex = 0;
    std::vector<std::string> m_levelOptions;
    size_t m_levelIndex = 0;

    // --- Popups (Cutscenes) ---
    bool m_bChatBoxPopupOpen = false;
    Entity *m_pCurrentChatEntity = nullptr;
    std::vector<std::string> m_cutsceneOptions;
    size_t m_cutsceneIndex = 0;

    const std::vector<std::string> m_editorPalette = {
        "SpawnPoint", "ChatBox", "LevelGoal",
        "TileMud", "TileGrass", "TileGrassCenter", "TileGrassLeft", "TileGrassMid", "TileGrassRight",
        "Box", "Slime", "Mushroom", "Bat", "Demon",
        "ManaFlower", "Boomerang", "Whisper"
    };

    // --- Helpers ---
    void loadLevel(const std::string &a_strFilePath);
    void saveLevel();
    void setStatus(const std::string &text, const sf::Color &color = sf::Color(0xFFffae00));

    void sysRender() override;
    void sDragAndDrop();
    
    void populateSelectors();
    void drawSelectors();
    void drawChatPopup();

    VectorPP windowToWorld(const VectorPP &a_vWindowPos);
    VectorPP gridToMidPixel(float gridX, float gridY, Entity *entity, float scale);
    bool isInside(const VectorPP &pos, Entity *entity);
};