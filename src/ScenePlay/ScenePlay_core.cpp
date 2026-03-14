#include <fstream>
#include "../ScenePlay.h"
#include "Action.h"
#include "Components.h"
#include <iostream>
#include "ParticleSystem.h"
#include "../SceneMenu.h"

ParticleSystem particles;

ScenePlay::ScenePlay(GameEngine *gameEngine, const std::string &a_strLevelPath, bool reloaded)
    : Scene(gameEngine), m_strLevelPath(a_strLevelPath)
{
    this->reloaded = reloaded;
    init(m_strLevelPath);
}

void ScenePlay::init(const std::string &a_strLevelPath)
{
    std::ifstream saveFile("save.csv");
    if (saveFile.is_open())
    {
        std::string line;
        while (std::getline(saveFile, line))
        {
            if (line.find("BoomerangUnlocked,") == 0)
            {
                m_bBoomerangUnlocked = true;
            }
            else if (line.find("Difficulty,") == 0)
            {
                std::string diff = line.substr(11);
                if (diff == "Easy")
                {
                    m_damageDealtMult = 2.0f;
                    m_damageTakenMult = 0.5f;
                    m_manaCostMult = 0.5f;
                }
                else if (diff == "Hard")
                {
                    m_damageDealtMult = 0.5f;
                    m_damageTakenMult = 1.5f;
                    m_manaCostMult = 1.5f;
                }
            }
        }
    }

    if (m_gameOverTexture.loadFromFile("Assets/Images/Menu/GameOver.png"))
    {
        m_gameOverSprite.setTexture(m_gameOverTexture);
        sf::Vector2u size = m_gameOverTexture.getSize();
        m_gameOverSprite.setScale((float)width() / size.x, (float)height() / size.y);
    }

    if (m_endTexture.loadFromFile("Assets/Images/Menu/EndScreen.png"))
    {
        m_endSprite.setTexture(m_endTexture);
        sf::Vector2u size = m_endTexture.getSize();
        m_endSprite.setScale((float)width() / size.x, (float)height() / size.y);
    }

    if (m_controllerTexture.loadFromFile("Assets/Images/controller1.png"))
    {
        m_controllerSprite.setTexture(m_controllerTexture);

        m_controllerSprite.setScale(0.15f, 0.15f);

        sf::Vector2u texSize = m_controllerTexture.getSize();
        m_controllerSprite.setPosition(40.0f, 660.0f);
    }

    registerAction(m_pGame->getKey("MOVE_LEFT"), "MOVE_LEFT");
    registerAction(m_pGame->getKey("MOVE_RIGHT"), "MOVE_RIGHT");
    registerAction(m_pGame->getKey("JUMP"), "JUMP");
    registerAction(m_pGame->getKey("BURROW"), "BURROW");
    registerAction(m_pGame->getKey("MENU"), "QUIT");

    registerAction(m_pGame->getKey("MELEE"), "INTERACT");

    registerAction(m_pGame->getKey("ELEMENT_SWITCH"), "ELEMSWITCH");
    registerAction(m_pGame->getKey("WEAPON_UP"), "WEAPON_UP");
    registerAction(m_pGame->getKey("WEAPON_DOWN"), "WEAPON_DOWN");

    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("JUMP")), "JUMP");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("BURROW")), "BURROW");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("ATTACK")), "ATTACK");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("MENU")), "QUIT");
    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("MELEE")), "INTERACT");

    registerAction("BTN_" + std::to_string(m_pGame->getControllerButton("BOOMERANG")), "BOOMERANG");
    registerAction(m_pGame->getKey("BOOMERANG"), "BOOMERANG");

    registerAction("ELEMENT_SWITCH", "ELEMSWITCH");
    registerAction("WEAPON_SWITCH_L", "SWITCH_WEAPON_L");
    registerAction("WEAPON_SWITCH_R", "SWITCH_WEAPON_R");
    registerAction("SCROLL_UP", "SWITCH_WEAPON_L");
    registerAction("SCROLL_DOWN", "SWITCH_WEAPON_R");

    registerAction("LS_LEFT", "MOVE_LEFT");
    registerAction("LS_RIGHT", "MOVE_RIGHT");
    registerAction("LS_UP", "JUMP");
    registerAction("LS_DOWN", "BURROW");

    registerAction("DPAD_LEFT", "MOVE_LEFT");
    registerAction("DPAD_RIGHT", "MOVE_RIGHT");
    registerAction("DPAD_UP", "JUMP");
    registerAction("DPAD_DOWN", "BURROW");

    registerAction("TRIGGER", "SHOOT");

    m_vMousePos = VectorPP((float)width() / 2.0f, (float)height() / 2.0f);
    m_lastAimDirection = VectorPP(1.0f, 0.0f);

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_pGame->getAssets().getFont("Harnold"));

    m_debugText.setFont(m_pGame->getAssets().getFont("Harnold"));
    m_debugText.setCharacterSize(20);
    m_debugText.setFillColor(sf::Color::White);
    m_debugText.setPosition(10, 10);

    if (m_dialogueTexture.loadFromFile("Assets/Images/TextBox.png"))
    {
        m_dialogueSprite.setTexture(m_dialogueTexture);
        float targetWidth = (float)width() * 0.6f; 
        float targetHeight = 150.0f;
        sf::Vector2u texSize = m_dialogueTexture.getSize();

        m_dialogueSprite.setScale(targetWidth / texSize.x, targetHeight / texSize.y);
        m_dialogueSprite.setOrigin(texSize.x / 2.0f, (float)texSize.y);
    }

    m_dialogueText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_dialogueText.setCharacterSize(24);
    m_dialogueText.setFillColor(sf::Color::White);

    m_speakerText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
    m_speakerText.setCharacterSize(22);
    m_speakerText.setFillColor(sf::Color(0xFF, 0xD7, 0x00));

    loadLevel(a_strLevelPath);
    initializeSpeechSounds();

    float voiceVolume = m_pGame->getVoiceVolume();
    m_chatterSound.setVolume(voiceVolume);

    if (!m_damageShader.loadFromFile("Assets/Shaders/damage.frag", sf::Shader::Fragment))
    {
        std::cout << "Shader cannot be loaded\n";
    }

    if (!m_frozenShader.loadFromFile("Assets/Shaders/burning.frag", sf::Shader::Fragment))
    {
        std::cout << "Shader cannot be loaded\n";
    }

    if (!m_burningShader.loadFromFile("Assets/Shaders/frozen.frag", sf::Shader::Fragment))
    {
        std::cout << "Shader cannot be loaded\n";
    }

    if (!m_poisonedShader.loadFromFile("Assets/Shaders/poison.frag", sf::Shader::Fragment))
    {
        std::cout << "Shader cannot be loaded\n";
    }
}

sf::View ScenePlay::buildPlayerView(Entity *player)
{
    sf::View view(sf::FloatRect(0.f, 0.f, (float)width(), (float)height()));
    
    sf::FloatRect lb = m_pGame->getViewport();
    sf::Vector2f viewSize = view.getSize();

    if (m_player2 == nullptr)
    {
        view.setViewport(lb);
        view.setSize(viewSize.x, viewSize.y);
    }
    else
    {
        view.setSize(viewSize.x / 2.f, viewSize.y);

        if (player->getTag() == "player")
        {
            view.setViewport(sf::FloatRect(lb.left, lb.top, lb.width * 0.5f, lb.height));
        }
        else
        {
            view.setViewport(sf::FloatRect(lb.left + (lb.width * 0.5f), lb.top, lb.width * 0.5f, lb.height));
        }
    }

    VectorPP &pPos = player->getComponent<CompTransform>().vPosition;
    
    float halfW = view.getSize().x / 2.0f;
    float halfH = view.getSize().y / 2.0f;

    float viewX = std::max(halfW, pPos.x);
    float viewY = std::min(pPos.y - 100.0f, (float)height() - halfH);

    view.setCenter(viewX, viewY);
    return view;
}

VectorPP ScenePlay::windowToWorld(const VectorPP &a_vWorldPos)
{
    sf::View p1View = buildPlayerView(m_player);
    sf::Vector2i pixel((int)a_vWorldPos.x, (int)a_vWorldPos.y);
    sf::Vector2f world = m_pGame->window().mapPixelToCoords(pixel, p1View);

    return VectorPP(world.x, world.y);
}

void ScenePlay::update()
{
    m_entityManager.update();
    sysChunkManagement();

    if (m_bIsRespawningPan)
    {
        m_fPanProgress += 0.015f;
        if (m_fPanProgress >= 1.0f)
        {
            m_bIsRespawningPan = false;
            reloadLayer();
        }
        else
        {
            float t = m_fPanProgress;
            float smoothT = t * t * (3.0f - 2.0f * t);
            m_player->getComponent<CompTransform>().vPosition = m_vPanStart + (m_vPanTarget - m_vPanStart) * smoothT;
        }

        sysRender();
        return;
    }

    if (m_pGame->isUsingController())
    {
        int joyID = m_pGame->getJoystickID();
        float aimX = sf::Joystick::getAxisPosition(joyID, sf::Joystick::U);
        float aimY = sf::Joystick::getAxisPosition(joyID, sf::Joystick::V);

        float deadzone = 20.0f;
        float aimRadius = 250.0f;

        float magnitude = std::sqrt(aimX * aimX + aimY * aimY);
        bool isAiming = magnitude > deadzone;

        static bool wasAiming = false;

        if (isAiming)
        {
            m_lastAimDirection.x = aimX / magnitude;
            m_lastAimDirection.y = aimY / magnitude;

            if (!wasAiming)
            {
                sysDoAction(Action("SHOOT", "START"));
            }
        }
        else
        {
            if (m_player->getComponent<CompTransform>().vScale.x < 0)
                m_lastAimDirection = VectorPP(-1.0f, 0.0f);
            else
                m_lastAimDirection = VectorPP(1.0f, 0.0f);

            if (wasAiming)
            {
                sysDoAction(Action("SHOOT", "END"));
            }
        }

        wasAiming = isAiming;

        sf::Vector2u winSize = m_pGame->window().getSize();
        VectorPP centerScreen(winSize.x / 2.0f, winSize.y / 2.0f);
        m_vMousePos = VectorPP(centerScreen.x + m_lastAimDirection.x * aimRadius,
                               centerScreen.y + m_lastAimDirection.y * aimRadius);
    }

    if (m_bShowGameOver || m_bShowEndScreen)
    {
        if (m_fScreenFadeAlpha < 255.0f)
        {
            m_fScreenFadeAlpha += 2.0f;
            
            if (m_fScreenFadeAlpha > 255.0f)
            {
                m_fScreenFadeAlpha = 255.0f;
            }
        }
    }

    if (!m_bIsPaused)
    {
        if (!m_bShowDialogue)
        {
            sysMovement();
        }
        sysTimer();
    }

    sysEnemyAI();
    updateDialogue();
    sysCollision();
    sysAnimation();
    sysHealthDetection();
    sysRender();
}

void ScenePlay::cycleElement()
{
    switch (m_currentElement)
    {
    case EntityStatus::NORMAL:
        break;
        
    case EntityStatus::FIRE:
        m_currentElement = EntityStatus::FROZEN;
        break;

    case EntityStatus::FROZEN:
        m_currentElement = EntityStatus::POISIONED;
        break;

    case EntityStatus::POISIONED:
        m_currentElement = EntityStatus::FIRE;
        break;
    }
}

void ScenePlay::weaponCycleUp()
{
    int w = static_cast<int>(m_currentWeapon);
    w = (w + 1) % static_cast<int>(WeaponType::COUNT);
    if (static_cast<WeaponType>(w) == WeaponType::LEAF && !m_bBoomerangUnlocked)
    {
        w = (w + 1) % static_cast<int>(WeaponType::COUNT);
    }
    m_currentWeapon = static_cast<WeaponType>(w);
}

void ScenePlay::weaponCycleDown()
{
    int w = static_cast<int>(m_currentWeapon);
    w = (w - 1 + static_cast<int>(WeaponType::COUNT)) %
        static_cast<int>(WeaponType::COUNT);
    if (static_cast<WeaponType>(w) == WeaponType::LEAF && !m_bBoomerangUnlocked)
    {
        w = (w - 1 + static_cast<int>(WeaponType::COUNT)) % static_cast<int>(WeaponType::COUNT);
    }
    m_currentWeapon = static_cast<WeaponType>(w);
}

void ScenePlay::drawLine(const VectorPP &start, const VectorPP &end)
{
    sf::Vertex line[] =
        {
            sf::Vertex(sf::Vector2f(start.x, start.y)),
            sf::Vertex(sf::Vector2f(end.x, end.y))};
    m_pGame->window().draw(line, 2, sf::Lines);
}

void ScenePlay::setPaused(bool paused)
{
    m_bIsPaused = paused;
}

void ScenePlay::onEnd()
{
    m_pGame->playSound("return");
    m_pGame->window().setView(m_pGame->window().getDefaultView());
    m_pGame->changeScene("MENU", nullptr, true);
}

bool ScenePlay::isInside(const VectorPP &a_vPos, Entity *a_pEntity)
{
    VectorPP vEntityPos = a_pEntity->getComponent<CompTransform>().vPosition;
    VectorPP vSize = a_pEntity->getComponent<CompAnimation>().animation.getSize();

    float dx = fabs(a_vPos.x - vEntityPos.x);
    float dy = fabs(a_vPos.y - vEntityPos.y);

    return (dx <= vSize.x / 2) && (dy <= vSize.y / 2);
}

void ScenePlay::saveLineToTempVector(const VectorPP &a_vStart, const VectorPP &a_vEnd)
{
    unsavedLines.push_back({a_vStart, a_vEnd});
}

bool ScenePlay::isPlayerTouchingChatBox() const
{
    return !getCutsceneFileFromChatBox().empty();
}

std::string ScenePlay::getCutsceneFileFromChatBox() const
{
    const EntityVec &tiles = m_entityManager.getEntities("tile");
    for (Entity *tile : tiles)
    {
        if (!tile->hasComponent<CompAnimation>() || !tile->hasComponent<CompBoundingBox>() || !tile->hasComponent<CompCutscene>())
            continue;

        if (tile->getComponent<CompAnimation>().animation.getName() == "ChatBox")
        {
            VectorPP overlap = m_physics.GetOverlap(m_player, tile);

            if (overlap.x > 0 && overlap.y > 0)
            {

                return tile->getComponent<CompCutscene>().filename;
            }
        }
    }

    return "";
}