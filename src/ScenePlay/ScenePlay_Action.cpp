#include "../ScenePlay.h"
#include "Components.h"
#include "GameEngine.h"
#include "ParticleSystem.h"
#include "../SceneMenu.h"
#include <iostream>

std::string stateToString(EntityState state)
{
    switch (state)
    {
    case EntityState::STANDING:
        return "STANDING";
    case EntityState::RUNNING:
        return "RUNNING";
    case EntityState::JUMPING:
        return "JUMPING";
    case EntityState::SHOOTING:
        return "SHOOTING";
    case EntityState::BURROWING:
        return "BURROWING";
    default:
        return "UNKNOWN";
    }
}

void ScenePlay::sysDragAndDrop()
{
    if (m_pGame->isUsingController())
        return;
    VectorPP mousePos = m_vMousePos;
    bool isLeftClick = sf::Mouse::isButtonPressed(sf::Mouse::Left);

    for (Entity *entity : m_entityManager.getEntities())
    {
        if (!entity->hasComponent<CompBoundingBox>() || !entity->hasComponent<CompTransform>())
            continue;
        CompBoundingBox &bbox = entity->getComponent<CompBoundingBox>();
        CompTransform &trans = entity->getComponent<CompTransform>();
        if (isLeftClick && !bbox.dragging)
        {
            if (isInside(mousePos, entity))
                bbox.dragging = true;
        }
        else if (!isLeftClick && bbox.dragging)
        {
            bbox.dragging = false;
        }
        if (bbox.dragging)
            trans.vPosition = mousePos;
    }
}

void ScenePlay::sysTimer()
{
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (entity->hasComponent<CompLifeSpan>())
        {
            int &lifespan = entity->getComponent<CompLifeSpan>().nDuration;
            if (lifespan > 0)
                lifespan -= 1;
            else if (lifespan <= 0)
                entity->destroy();
        }
        if (entity->hasComponent<CompInvincible>())
        {
            CompInvincible &invincibility = entity->getComponent<CompInvincible>();
            if (invincibility.framesRemaining > 0)
                invincibility.framesRemaining -= 1;
            if (invincibility.framesRemaining <= 0)
            {
                invincibility.isInvincible = false;
                invincibility.has = false;
            }
        }
        if (entity->hasComponent<CompFlipScale>())
        {
            CompFlipScale &flip = entity->getComponent<CompFlipScale>();
            if (flip.has && flip.framesRemaining > 0)
            {
                float &scaleX = entity->getComponent<CompTransform>().vScale.x;
                int target = flip.targetSign;
                float scaleMagnitude = 1.0f;
                if (entity->hasComponent<CompAnimation>())
                {
                    std::string animName = entity->getComponent<CompAnimation>().animation.getName();
                    scaleMagnitude = m_pGame->getAssets().getAnimationScale(animName);
                }
                float startSign = (target > 0) ? -1.0f : 1.0f;
                float t = 1.0f - ((float)flip.framesRemaining / (float)flip.totalFrames);
                float interpolatedSign = startSign + (t * ((float)target - startSign));
                scaleX = scaleMagnitude * interpolatedSign;
                flip.framesRemaining -= 1;
                if (flip.framesRemaining <= 0)
                {
                    scaleX = scaleMagnitude * (float)target;
                    flip.has = false;
                }
            }
        }
    }

    CompStatus &statusComp = m_player->getComponent<CompStatus>();
    if (statusComp.status == FROZEN || statusComp.status == FIRE || statusComp.status == POISIONED)
    {
        if (statusComp.framesRemaining > 0)
            statusComp.framesRemaining -= 1;
        if (statusComp.framesRemaining <= 0)
        {
            statusComp.status = NORMAL;
            statusComp.framesRemaining = 60;
        }
    }
    if (statusComp.status == FIRE && statusComp.framesRemaining % 60 == 0)
        m_player->getComponent<CompHealth>().currentHealth -= (int)(10 * m_damageTakenMult);
    else if (statusComp.status == POISIONED && statusComp.framesRemaining % 60 == 0)
        m_player->getComponent<CompHealth>().currentHealth -= (int)(20 * m_damageTakenMult);

    // Falling Tile Logic
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (entity->hasComponent<CompFallingTile>())
        {
            auto &tile = entity->getComponent<CompFallingTile>();
            auto &transform = entity->getComponent<CompTransform>();
            auto &bbox = entity->getComponent<CompBoundingBox>();

            if (tile.isFading)
            {
                tile.opacity += 10.0f;
                if (tile.opacity >= 255.0f)
                {
                    tile.opacity = 255.0f;
                    tile.isFading = false;
                    bbox.isActive = true;
                }
                transform.vPosition = tile.originalPos;
                transform.vVelocity = VectorPP(0, 0);
            }
            else if (tile.triggered)
            {
                if (tile.shakeTimer > 0)
                {
                    tile.shakeTimer--;
                    transform.vPosition.x = tile.originalPos.x + ((tile.shakeTimer % 4 < 2) ? 2 : -2);
                }
                else if (tile.fallTimer > 0)
                {
                    if (entity->hasComponent<CompGravity>())
                        entity->getComponent<CompGravity>().fGravity = 0.5f;
                    tile.fallTimer--;
                    transform.vPosition.x = tile.originalPos.x;
                }
                else
                {
                    if (entity->hasComponent<CompGravity>())
                        entity->getComponent<CompGravity>().fGravity = 0.5f;
                    transform.vPosition.x = tile.originalPos.x;
                    float dropDist = transform.vPosition.y - tile.originalPos.y;
                    float maxDist = m_pGame->window().getSize().y / 4.0f;
                    if (dropDist > 0)
                    {
                        float ratio = dropDist / maxDist;
                        if (ratio > 1.0f)
                            ratio = 1.0f;
                        tile.opacity = 255.0f * (1.0f - ratio);
                    }
                    if (dropDist >= maxDist)
                    {
                        tile.triggered = false;
                        tile.shakeTimer = tile.maxShake;
                        tile.fallTimer = tile.maxFall;
                        tile.isFading = true;
                        tile.opacity = 0.0f;
                        bbox.isActive = false;
                        transform.vPosition = tile.originalPos;
                        transform.vVelocity = VectorPP(0, 0);
                        if (entity->hasComponent<CompGravity>())
                            entity->getComponent<CompGravity>().fGravity = 0.0f;
                    }
                }
            }
        }
    }
}

void ScenePlay::sysDoAction(const Action &a_action)
{
    if (m_bShowGameOver || m_bShowEndScreen)
    {
        if (a_action.getName() == "LEFT_CLICK" || 
            a_action.getName() == "RIGHT_CLICK" || 
            a_action.getName() == "MOUSE_MOVE")
        {
            return;
        }

        if (a_action.getState() == "START")
        {
            m_pGame->changeScene("MENU", nullptr, true);
        }
        return; 
    }
    if (a_action.getState() == "START")
    {
        if (m_bShowGameOver || m_bShowEndScreen)
        {
            if (a_action.getName() == "QUIT" || a_action.getName() == "INTERACT")
                onEnd();
            return;
        }
        if (a_action.getName() == "PAUSE")
        {
            setPaused(!m_bIsPaused);
            return;
        }
        else if (a_action.getName() == "QUIT")
        {
            onEnd();
            return;
        }
        else if (a_action.getName() == "BOOMERANG")
        {
            if (m_bBoomerangUnlocked && m_player->getComponent<CompState>().state != EntityState::BURROWING &&
                m_entityManager.getEntities("leaf").empty())
            {
                spawnBoomerang(m_player);
                m_pGame->playSound("familiar_shoot");
            }
        }
        else if (a_action.getName() == "BURROW")
        {
            if (onGround && m_player->getComponent<CompState>().state == EntityState::STANDING)
            {
                m_pGame->playSound("fox_burrow");
                auto &state = m_player->getComponent<CompState>();
                state.state = EntityState::BURROWING;
                state.burrowPhase = 0;
                auto &anim = m_player->getComponent<CompAnimation>();
                anim.animation = m_pGame->getAssets().getAnimation("FoxBurrow");
                anim.animation.setLoop(false);
            }
        }
        else if (a_action.getName() == "INTERACT")
        {
            if (m_bShowDialogue)
            {
                if (m_charIndex < m_fullText.size())
                {
                    m_currentText = m_fullText;
                    m_charIndex = m_fullText.size();
                    m_dialogueText.setString(m_currentText);
                }
                else
                    nextDialogueLine();
            }
            else
            {
                std::string cutsceneFile = getCutsceneFileFromChatBox();
                if (!cutsceneFile.empty())
                    loadCutscene(cutsceneFile);
                else
                    createMeleeAttack();
            }
            return;
        }

        if (m_bShowDialogue)
            return;
        else if (a_action.getName() == "TOGGLE_TEXTURE")
            m_bDrawTextures = !m_bDrawTextures;
        else if (a_action.getName() == "TOGGLE_COLLISION")
            m_bDrawCollision = !m_bDrawCollision;
        else if (a_action.getName() == "TOGGLE_GRID")
            m_bDrawGrid = !m_bDrawGrid;
        else if (a_action.getName() == "TOGGLE_GRAVITY")
            m_bGravityEnabled = !m_bGravityEnabled;
        else if (a_action.getName() == "WEAPON_UP")
            weaponCycleUp();
        else if (a_action.getName() == "WEAPON_DOWN")
            weaponCycleDown();
        else if (a_action.getName() == "ELEMSWITCH" && m_currentWeapon == WeaponType::ELEMENTBALL)
            cycleElement();
        else if (a_action.getName() == "LEFT_CLICK" || a_action.getName() == "SHOOT")
        {
            if (!m_bIsPaused && m_player->getComponent<CompInput>().canShoot)
            {
                if (m_player->getComponent<CompMana>().currentMana >= std::max(1, (int)std::ceil(1 * m_manaCostMult)))
                {
                    VectorPP finalTarget;
                    if (m_pGame->isUsingController())
                        finalTarget = familiarPosition + (m_lastAimDirection * 1000.0f);
                    else
                    {
                        VectorPP mouseScreen = (a_action.getName() == "SHOOT") ? m_vMousePos : a_action.getPos();
                        sf::View pView = buildPlayerView(m_player);
                        sf::Vector2i pixel((int)mouseScreen.x, (int)mouseScreen.y);
                        sf::Vector2f world = m_pGame->window().mapPixelToCoords(pixel, pView);
                        finalTarget = VectorPP(world.x, world.y);
                    }
                    createBullet(VectorPP(particles.getEmitter().x, particles.getEmitter().y), finalTarget);
                    m_pGame->playSound("familiar_shoot");
                    m_player->getComponent<CompMana>().currentMana -= std::max(1, (int)std::ceil(1 * m_manaCostMult));
                    m_player->getComponent<CompInput>().canShoot = false;
                }
            }
        }
        else if (a_action.getName() == "ATTACK")
        {
            if (!m_bIsPaused && m_player->getComponent<CompState>().state != EntityState::BURROWING)
            {
                if (m_currentWeapon == WeaponType::MELEE)
                {
                    createMeleeAttack();
                    m_pGame->playSound("fox_bite");
                }
                else
                {
                    VectorPP finalTarget;
                    if (m_pGame->isUsingController())
                        finalTarget = familiarPosition + (m_lastAimDirection * 1000.0f);
                    else
                    {
                        sf::View pView = buildPlayerView(m_player);
                        sf::Vector2i pixel((int)m_vMousePos.x, (int)m_vMousePos.y);
                        sf::Vector2f world = m_pGame->window().mapPixelToCoords(pixel, pView);
                        finalTarget = VectorPP(world.x, world.y);
                    }
                    if (m_currentWeapon == WeaponType::ELEMENTBALL)
                    {
                        if (m_player->getComponent<CompMana>().currentMana >= std::max(1, (int)std::ceil(15 * m_manaCostMult)))
                        {
                            createElementBall(particles.getEmitter(), finalTarget, m_currentElement);
                            m_pGame->playSound("familiar_shoot");
                            m_player->getComponent<CompMana>().currentMana -= std::max(1, (int)std::ceil(15 * m_manaCostMult));
                        }
                    }
                    else if (m_currentWeapon == WeaponType::LEAF)
                    {
                        if (m_player->getComponent<CompMana>().currentMana >= std::max(1, (int)std::ceil(5 * m_manaCostMult)))
                        {
                            createLeaf(particles.getEmitter());
                            m_pGame->playSound("familiar_shoot");
                            m_player->getComponent<CompMana>().currentMana -= std::max(1, (int)std::ceil(5 * m_manaCostMult));
                        }
                    }
                }
            }
        }
        else if (a_action.getName() == "MOUSE_MOVE")
            m_vMousePos = a_action.getPos();
        else if (a_action.getName() == "MOVE_LEFT")
            m_player->getComponent<CompInput>().left = true;
        else if (a_action.getName() == "MOVE_RIGHT")
            m_player->getComponent<CompInput>().right = true;
        else if (a_action.getName() == "JUMP")
        {
            auto &state = m_player->getComponent<CompState>();
            auto &transform = m_player->getComponent<CompTransform>();
            auto &mana = m_player->getComponent<CompMana>();
            if (state.state != EntityState::JUMPING)
            {
                transform.vVelocity.y = -15.0f;
                m_pGame->playSound("fox_jump");
                state.state = EntityState::JUMPING;
                state.hasDoubleJumped = false;
            }
            else if (!state.hasDoubleJumped && mana.currentMana >= std::max(1, (int)std::ceil(5 * m_manaCostMult)))
            {
                transform.vVelocity.y = -12.0f;
                m_pGame->playSound("familiar_jump");
                mana.currentMana -= std::max(1, (int)std::ceil(5 * m_manaCostMult));
                state.hasDoubleJumped = true;
                VectorPP feetPos = transform.vPosition;
                feetPos.y += 20;
                spawnParticles(feetPos, 15, sf::Color(100, 200, 255));
            }
        }
    }
    else if (a_action.getState() == "END")
    {
        if (a_action.getName() == "INTERACT")
            return;
        if (a_action.getName() == "MOVE_LEFT")
            m_player->getComponent<CompInput>().left = false;
        else if (a_action.getName() == "MOVE_RIGHT")
            m_player->getComponent<CompInput>().right = false;
        else if (a_action.getName() == "JUMP")
        {
            m_player->getComponent<CompInput>().up = false;
            m_player->getComponent<CompInput>().canJump = true;
        }
        else if (a_action.getName() == "LEFT_CLICK" || a_action.getName() == "SHOOT")
            m_player->getComponent<CompInput>().canShoot = true;
    }
}