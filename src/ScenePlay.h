#pragma once
#include "Common.h"
#include "Physics.h"
#include "Scene.h"
#include <tgmath.h>
#include <random>
#include <vector>
#include <string>
#include <SFML/Audio.hpp>
#include "Components.h"
#include "ParticleSystem.h"
#include <set>

extern ParticleSystem particles;

// --- Helper Structs ---
struct EffectParticle
{
    VectorPP pos;
    VectorPP vel;
    int life;
    int maxLife;
    sf::Color color;
};

enum class WeaponType
{
    MELEE,
    ELEMENTBALL,
    LEAF,
    COUNT
};

class ScenePlay : public Scene
{
private:
    // --- Configuration Structs ---
    struct PlayerConfig
    {
        float X, Y, BBOX_WIDTH, BBOX_HEIGHT, SPEED_X, SPEED_Y, SPEED_MAX, GRAVITY;
        std::string WEAPON;
    };

    struct DialogueCommand
    {
        std::string targetTag;
        std::string action;
        std::string value1;
        std::string value2;
        float duration;
    };

    struct TileConfig
    {
        std::string type;
        float gridX;
        float gridY;
        std::string tag;
        std::string script;
    };

    struct DialogueLine
    {
        std::string speaker;
        std::string text;
        DialogueCommand command;
        bool isCommand = false;
    };

    typedef std::pair<int, int> ChunkCoord;

protected:
    // --- Core State ---
    Entity *m_player;
    Entity *m_player2 = nullptr;
    std::string m_strLevelPath;
    std::string m_strNextLevel;
    std::string m_currentPath;
    bool m_hasLevelFinished = false;
    bool m_bIsPaused = false;
    Physics m_physics;
    std::mt19937 m_rng;
    bool m_bIsRespawningPan = false;
    VectorPP m_vPanStart;
    VectorPP m_vPanTarget;
    float m_fPanProgress = 0.0f;

    // --- Player State ---
    PlayerConfig m_playerConfig;
    WeaponType m_currentWeapon = WeaponType::MELEE;
    EntityStatus m_currentElement = EntityStatus::FIRE;
    bool m_bGravityEnabled = true;
    bool onGround = false;
    int jumpNum = 0;
    int liveNum = 3;
    bool m_bBoomerangUnlocked = false;
    bool reloaded = false;
    bool reloading = false;

    // --- Level / Chunk System ---
    int m_chunkSize = 20;
    std::map<ChunkCoord, std::vector<TileConfig>> m_worldChunks;
    std::set<ChunkCoord> m_activeChunks;
    VectorPP m_vGridSize;

    // --- Graphics & UI ---
    sf::Text m_gridText;
    sf::Text m_debugText;
    sf::Texture m_backgroundTexture;
    sf::Sprite m_backgroundSprite;
    sf::View m_minimapView;

    // UI Overlays
    sf::Texture m_gameOverTexture;
    sf::Sprite m_gameOverSprite;
    sf::Texture m_endTexture;
    sf::Sprite m_endSprite;
    bool m_bShowGameOver = false;
    bool m_bShowEndScreen = false;
    float m_fScreenFadeAlpha = 0.0f;

    // Debug Flags
    bool m_bDrawTextures = true;
    bool m_bDrawCollision = false;
    bool m_bDrawGrid = false;
    bool m_bDrawPlayerHUD = true;

    // --- Input / Control ---
    VectorPP m_vMousePos;
    VectorPP m_lastAimDirection;
    sf::Texture m_controllerTexture;
    sf::Sprite m_controllerSprite;

    // --- Dialogue System ---
    bool m_bShowDialogue = false;
    bool m_chatBoxCutsceneTriggered = false;
    sf::Texture m_dialogueTexture;
    sf::Sprite m_dialogueSprite;
    sf::Text m_dialogueText;
    sf::Text m_speakerText;

    std::vector<DialogueLine> m_cutsceneQueue;
    std::string m_fullText;
    std::string m_currentText;
    size_t m_charIndex = 0;
    int m_textTimer = 0;

    // Audio
    std::vector<sf::SoundBuffer> m_speechSoundBuffers;
    std::vector<sf::SoundBuffer> m_DemonSpeechSoundBuffers;
    std::vector<sf::SoundBuffer> m_FamiliarSpeechSoundBuffers;
    std::string m_currentSpeaker;
    sf::Sound m_speechSound;
    sf::Sound m_chatterSound;
    int m_chatterTimer = 0;
    const int CHATTER_RATE = 15;

    // --- Combat / Mechanics ---
    std::vector<EffectParticle> m_effectParticles;
    std::vector<std::pair<VectorPP, VectorPP>> unsavedLines;
    VectorPP familiarPosition;

    // Shaders
    sf::Shader m_damageShader;
    sf::Shader m_frozenShader;
    sf::Shader m_burningShader;
    sf::Shader m_poisonedShader;

    // Stats
    float m_damageDealtMult = 1.0f;
    float m_damageTakenMult = 1.0f;
    float m_manaCostMult = 1.0f;

    // --- Internal Systems ---
    void init(const std::string &a_strLevelPath);
    void loadLevel(const std::string &a_strFilePath);
    void reloadLayer();

    // Spawning
    void createPlayer();
    void createFamiliar();
    Entity *createMonster(VectorPP a_vPos);
    Entity *createMushroom(VectorPP a_vPos);
    Entity *createFlyingMonster(VectorPP a_vPos);
    Entity *createBoss(VectorPP a_vPos);
    Entity *createManaFlower(VectorPP a_vPos);
    Entity *createManaBerry(VectorPP a_vPos);
    Entity *createSmallBerry(VectorPP a_vPos);
    Entity *createLargeBerry(VectorPP a_vPos);
    Entity *createBoomerangPickup(VectorPP a_vPos);

    // Combat Creation
    void createBullet(const VectorPP &familiarPos, const VectorPP &a_vMousePos);
    void createMeleeAttack();
    void createElementBall(const VectorPP &familiarPos, const VectorPP &a_vMousePos, EntityStatus m_currentElement);
    void createLeaf(const VectorPP &familiarPos);
    void createBossBullet(VectorPP origin, VectorPP target);
    void spawnBoomerang(Entity *entity);

    // Helpers
    sf::View buildPlayerView(Entity *player);
    VectorPP windowToWorld(const VectorPP &a_vWorldPos);
    VectorPP gridToMidPixel(float a_fGridX, float a_fGridY, Entity *a_pEntity, float scale);
    void drawLine(const VectorPP &start, const VectorPP &end);
    void spawnParticles(VectorPP pos, int count, sf::Color color);

    // Mechanics
    void knockBack(Entity *a_pEntity);
    void invincible(Entity *a_pEntity);
    void changeScale(Entity *a_pEntity);
    void cycleElement();
    void weaponCycleUp();
    void weaponCycleDown();

    // Chunking
    void sysChunkManagement();
    void spawnChunk(ChunkCoord coord);
    void unloadChunk(ChunkCoord coord);

    // Dialogue Logic
    void loadCutscene(const std::string &a_strFilePath);
    void nextDialogueLine();
    void updateDialogue();
    void processCommand(const DialogueCommand &cmd);
    void initializeSpeechSounds();
    void playRandomSpeechSound();
    std::string getCutsceneFileFromChatBox() const;
    bool isPlayerTouchingChatBox() const;

    // Debug
    void saveLineToTempVector(const VectorPP &a_vStart, const VectorPP &a_vEnd);
    void drawWalkableLine(const VectorPP &a_vP1, const VectorPP &a_vP2);
    bool isInside(const VectorPP &a_vPos, Entity *a_pEntity);

public:
    ScenePlay(GameEngine *a_pGame, const std::string &a_strLevelPath, bool reloaded = false);

    void update() override;
    void onEnd() override;
    void sysDoAction(const Action &a_action) override;
    void sysRender() override;

    void setPaused(bool paused);

    // Logic Systems
    void sysMovement();
    void sysTimer();
    void sysCollision();
    void sysAnimation();
    void sysDragAndDrop();
    void sysHealthDetection();
    void sysEnemyAI();
};