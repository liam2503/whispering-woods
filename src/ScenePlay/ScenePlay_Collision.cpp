#include "../ScenePlay.h"
#include "Components.h"
#include "GameEngine.h"
#include "../SceneMenu.h"
#include <iostream>
#include <fstream>

// Helper to determine enemies
bool isEnemy(Entity *e)
{
    std::string t = e->getTag();
    return (t == "monster" || t == "mushroom" || t == "flyingmonster" || t == "boss" || t == "enemy_bullet");
}

void ScenePlay::sysCollision()
{
    onGround = false;
    CompTransform &playerTransform = m_player->getComponent<CompTransform>();
    playerTransform.fAngle = 0.0f;

    auto attemptLootDrop = [&](VectorPP pos)
    {
        int roll = rand() % 100;
        if (roll < 30)
        {
            if (rand() % 100 < 20)
                createLargeBerry(pos);
            else
                createSmallBerry(pos);
        }
    };

    // --- Entity vs World (Tiles) ---
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (!entity->isActive() || !entity->hasComponent<CompTransform>() || !entity->hasComponent<CompBoundingBox>())
            continue;
        if (entity->getTag() != "player" && !isEnemy(entity) && entity->getTag() != "manaBerry")
            continue;

        CompTransform &entityTransform = entity->getComponent<CompTransform>();
        CompBoundingBox &entityBbox = entity->getComponent<CompBoundingBox>();

        // Fall off map check
        if (entityTransform.vPosition.y > height() + 100)
        {
            if (entity->hasComponent<CompFallingTile>())
                continue;
            if (entity->hasComponent<CompHealth>())
                entity->getComponent<CompHealth>().currentHealth -= 1000;
            else
                entity->destroy();
            continue;
        }

        for (Entity *tile : m_entityManager.getEntities("tile"))
        {
            if (!tile->hasComponent<CompBoundingBox>())
                continue;
            CompTransform &tileTransform = tile->getComponent<CompTransform>();
            CompBoundingBox &tileBbox = tile->getComponent<CompBoundingBox>();

            VectorPP overlap = m_physics.GetOverlap(entity, tile);
            VectorPP prevOverlap = m_physics.GetPreviousOverlap(entity, tile);

            if (overlap.x <= 0 || overlap.y <= 0 || !tileBbox.isActive)
                continue;

            // Handle special tile logic (LevelGoal, ChatBox)
            if (tile->hasComponent<CompAnimation>())
            {
                std::string tileAnimName = tile->getComponent<CompAnimation>().animation.getName();
                if (tileAnimName == "LevelGoal" && entity->getTag() == "player" && !m_hasLevelFinished)
                {
                    std::ofstream saveFile("save.csv", std::ios::app);
                    if (saveFile.is_open())
                    {
                        saveFile << m_strNextLevel << "\n";
                        saveFile.close();
                    }
                    m_hasLevelFinished = true;
                    if (m_strLevelPath.find("Level Editor.txt") != std::string::npos || m_strNextLevel.empty())
                    {
                        m_pGame->window().setView(m_pGame->window().getDefaultView());
                        m_pGame->changeScene("MENU", nullptr, true);
                    }
                    else
                    {
                        m_pGame->prepareLevelLoad(m_strNextLevel);
                        m_pGame->changeScene("PLAY", nullptr, true);
                    }
                    return;
                }
                if (tileAnimName == "ChatBox" && entity->getTag() == "player")
                {
                    if (tile->hasComponent<CompCutscene>())
                    {
                        CompCutscene &cutscene = tile->getComponent<CompCutscene>();
                        if (!cutscene.hasRun)
                        {
                            loadCutscene(cutscene.filename);
                            cutscene.hasRun = true;
                        }
                    }
                    continue;
                }
            }

            // Standard Collision Resolution
            if (overlap.x > 0 && overlap.y > 0)
            {
                if (prevOverlap.y < overlap.y && overlap.y < overlap.x)
                {
                    if (entityTransform.vPosition.y < tileTransform.vPosition.y)
                    {
                        entityTransform.vPosition.y -= overlap.y;
                        if (entity->getComponent<CompState>().state != EntityState::RUNNING)
                            entity->getComponent<CompState>().state = EntityState::STANDING;
                        if (entity->getTag() == "player")
                        {
                            onGround = true;
                            jumpNum = 0;
                            entity->getComponent<CompInput>().canJump = true;
                            if (tile->hasComponent<CompFallingTile>())
                                tile->getComponent<CompFallingTile>().triggered = true;
                        }
                    }
                    else
                    {
                        entityTransform.vPosition.y += overlap.y;
                        if (entity->getTag() == "player" && tile->hasComponent<CompAnimation>())
                        {
                            Animation &tileAnim = tile->getComponent<CompAnimation>().animation;
                            if (tileAnim.getName() == "BoxTreasure")
                            {
                                tileAnim = m_pGame->getAssets().getAnimation("BoxCoin");
                                Entity *coin = m_entityManager.addEntity("Coin");
                                coin->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("CoinSpin"), true);
                                VectorPP coinPos = tileTransform.vPosition;
                                coinPos.y -= tileBbox.vHalfSize.y + 64;
                                coin->addComponents<CompTransform>(coinPos);
                                coin->addComponents<CompLifeSpan>(30, m_pGame->getCurrentFrame());
                            }
                        }
                    }
                    if (entity->getTag() == "manaBerry")
                    {
                        if (std::abs(entityTransform.vVelocity.y) > 2.0f)
                            entityTransform.vVelocity.y *= -0.6f;
                        else
                            entityTransform.vVelocity.y = 0.0f;
                    }
                    else
                        entityTransform.vVelocity.y = 0;
                }
                else if (prevOverlap.x < overlap.x && overlap.x < overlap.y)
                {
                    if (entityTransform.vPosition.x < tileTransform.vPosition.x)
                        entityTransform.vPosition.x -= overlap.x;
                    else
                        entityTransform.vPosition.x += overlap.x;
                    if (entity->getTag() == "player")
                        entityTransform.vVelocity.x = 0;
                    if (isEnemy(entity))
                        entityTransform.vVelocity.x = 0;
                }
            }
        }
    }

    // --- Player vs Enemy ---
    for (Entity *enemy : m_entityManager.getEntities())
    {
        if (!enemy->isActive() || !isEnemy(enemy))
            continue;
        if (m_bShowEndScreen || m_bShowGameOver)
            continue;
        VectorPP overlap = m_physics.GetOverlap(enemy, m_player);
        if (overlap.x > 0 && overlap.y > 0)
        {
            if (m_player->getComponent<CompState>().state == EntityState::BURROWING && m_player->getComponent<CompState>().burrowPhase == 1)
                continue;
            if (!m_player->hasComponent<CompInvincible>() || !m_player->getComponent<CompInvincible>().isInvincible)
            {
                int dmg = enemy->hasComponent<CompDamage>() ? enemy->getComponent<CompDamage>().damage : 10;
                m_player->getComponent<CompHealth>().currentHealth -= (int)(dmg * m_damageTakenMult);
                m_pGame->playSound("fox_hurt");
                knockBack(m_player);
                invincible(m_player);
                spawnParticles(m_player->getComponent<CompTransform>().vPosition, 20, sf::Color::Red);
            }
        }
    }

    // --- Bullets vs World/Enemies ---
    for (Entity *bullet : m_entityManager.getEntities("bullet"))
    {
        if (!bullet->isActive())
            continue;
        // Bullet vs Tile
        for (Entity *tile : m_entityManager.getEntities("tile"))
        {
            if (m_physics.GetOverlap(bullet, tile).x > 0 && m_physics.GetOverlap(bullet, tile).y > 0)
            {
                if (tile->hasComponent<CompAnimation>() && tile->getComponent<CompAnimation>().animation.getName() == "Box")
                {
                    Entity *explosion = m_entityManager.addEntity("explosion");
                    explosion->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Explosion"), true);
                    explosion->addComponents<CompTransform>(tile->getComponent<CompTransform>().vPosition);
                    explosion->addComponents<CompLifeSpan>(30, m_pGame->getCurrentFrame());

                    VectorPP spawnPos = tile->getComponent<CompTransform>().vPosition;
                    if (rand() % 2 == 0)
                        createSmallBerry(spawnPos);
                    else
                        createLargeBerry(spawnPos);
                    tile->destroy();
                }
                bullet->destroy();
                break;
            }
        }
        if (!bullet->isActive())
            continue;

        // Bullet vs Enemy
        for (Entity *enemy : m_entityManager.getEntities())
        {
            if (!enemy->isActive() || !isEnemy(enemy))
                continue;
            if (m_physics.GetOverlap(enemy, bullet).x > 0 && m_physics.GetOverlap(enemy, bullet).y > 0)
            {
                auto &health = enemy->getComponent<CompHealth>();
                int dmg = bullet->hasComponent<CompDamage>() ? bullet->getComponent<CompDamage>().damage : 10;
                health.currentHealth -= dmg;

                if (enemy->getTag() == "monster")
                    m_pGame->playSound("slime_hurt");
                else if (enemy->getTag() == "mushroom")
                    m_pGame->playSound("mushroom_hurt");
                else if (enemy->getTag() == "flyingmonster")
                    m_pGame->playSound("bat_hurt");
                else if (enemy->getTag() == "boss")
                    m_pGame->playSound("demon_hurt");

                invincible(enemy);
                spawnParticles(enemy->getComponent<CompTransform>().vPosition, 15, sf::Color::Red);

                if (bullet->hasComponent<CompStatus>())
                {
                    EntityStatus bStatus = bullet->getComponent<CompStatus>().status;
                    auto &enemyStatus = enemy->getComponent<CompStatus>();
                    if (bStatus == EntityStatus::FIRE)
                    {
                        health.currentHealth -= 2;
                        enemyStatus = CompStatus(EntityStatus::FIRE, 300);
                    }
                    else if (bStatus == EntityStatus::FROZEN)
                    {
                        health.currentHealth -= 1;
                        enemyStatus = CompStatus(EntityStatus::FROZEN, 300);
                    }
                    else if (bStatus == EntityStatus::POISIONED)
                    {
                        health.currentHealth -= 15;
                        enemyStatus = CompStatus(EntityStatus::POISIONED, 300);
                    }
                }
                bullet->destroy();
                if (health.currentHealth <= 0)
                {
                    spawnParticles(enemy->getComponent<CompTransform>().vPosition, 50, sf::Color::Red);
                    attemptLootDrop(enemy->getComponent<CompTransform>().vPosition);

                    if (enemy->getTag() == "boss")
                    {
                        m_bShowEndScreen = true;
                        m_pGame->requestMusic("Assets/Music/Tutorial.ogg", 100.0f);
                    }
                    enemy->destroy();
                }
                break;
            }
        }
    }

    // --- Enemy Bullets vs Player ---
    for (Entity *bullet : m_entityManager.getEntities("enemy_bullet"))
    {
        if (!bullet->isActive())
            continue;
        if (m_bShowEndScreen || m_bShowGameOver)
            continue;
        VectorPP overlap = m_physics.GetOverlap(bullet, m_player);
        if (overlap.x > 0 && overlap.y > 0)
        {
            if (m_player->getComponent<CompState>().state == EntityState::BURROWING && m_player->getComponent<CompState>().burrowPhase == 1)
                continue;
            if (!m_player->hasComponent<CompInvincible>() || !m_player->getComponent<CompInvincible>().isInvincible)
            {
                int dmg = bullet->hasComponent<CompDamage>() ? bullet->getComponent<CompDamage>().damage : 10;
                m_player->getComponent<CompHealth>().currentHealth -= (int)(dmg * m_damageTakenMult);
                if (dmg == 15)
                    m_player->getComponent<CompStatus>() = CompStatus(EntityStatus::POISIONED, 300);
                knockBack(m_player);
                invincible(m_player);
                bullet->destroy();
                spawnParticles(m_player->getComponent<CompTransform>().vPosition, 20, sf::Color::Red);
            }
        }
    }

    // --- Melee/Leaf vs Enemies ---
    auto handleAttack = [&](Entity *attackEntity)
    {
        for (Entity *enemy : m_entityManager.getEntities())
        {
            if (!enemy->isActive() || !isEnemy(enemy))
                continue;
            if (enemy->hasComponent<CompInvincible>() && enemy->getComponent<CompInvincible>().isInvincible)
                continue;
            if (m_physics.GetOverlap(attackEntity, enemy).x > 0 && m_physics.GetOverlap(attackEntity, enemy).y > 0)
            {
                if (enemy->hasComponent<CompHealth>())
                {
                    int dmg = attackEntity->hasComponent<CompDamage>() ? attackEntity->getComponent<CompDamage>().damage : 45;
                    enemy->getComponent<CompHealth>().currentHealth -= dmg;
                    if (enemy->getTag() == "monster")
                        m_pGame->playSound("slime_hurt");
                    else if (enemy->getTag() == "mushroom")
                        m_pGame->playSound("mushroom_hurt");
                    else if (enemy->getTag() == "flyingmonster")
                        m_pGame->playSound("bat_hurt");
                    else if (enemy->getTag() == "boss")
                        m_pGame->playSound("demon_hurt");
                    invincible(enemy);
                    spawnParticles(enemy->getComponent<CompTransform>().vPosition, 15, sf::Color::Red);
                    if (enemy->getComponent<CompHealth>().currentHealth <= 0)
                    {
                        spawnParticles(enemy->getComponent<CompTransform>().vPosition, 50, sf::Color::Red);
                        attemptLootDrop(enemy->getComponent<CompTransform>().vPosition);

                        if (enemy->getTag() == "boss")
                        {
                            m_bShowEndScreen = true;
                            m_pGame->requestMusic("Assets/Music/Tutorial.ogg", 100.0f);
                        }
                        enemy->destroy();
                    }
                }
                if (attackEntity->getTag() == "Melee")
                    attackEntity->destroy();
                if (attackEntity->getTag() == "Melee")
                    break;
            }
        }
    };

    for (Entity *melee : m_entityManager.getEntities("Melee"))
        handleAttack(melee);
    for (Entity *leaf : m_entityManager.getEntities("leaf"))
        handleAttack(leaf);

    // --- Mana Flowers vs Bullets ---
    for (Entity *flower : m_entityManager.getEntities("manaFlower"))
    {
        if (flower->getComponent<CompAnimation>().animation.getName() == "ManaFlowerSpent")
            continue;
        for (Entity *bullet : m_entityManager.getEntities("bullet"))
        {
            if (bullet->isActive() && m_physics.GetOverlap(bullet, flower).x > 0 && m_physics.GetOverlap(bullet, flower).y > 0)
            {
                createManaBerry(flower->getComponent<CompTransform>().vPosition);
                flower->getComponent<CompAnimation>().animation = m_pGame->getAssets().getAnimation("ManaFlowerSpent");
                bullet->destroy();
                break;
            }
        }
    }

    // --- Pickups ---
    auto handlePickup = [&](std::string tag, int hpGain, int manaGain)
    {
        for (Entity *item : m_entityManager.getEntities(tag))
        {
            if (item->isActive() && m_physics.GetOverlap(m_player, item).x > 0 && m_physics.GetOverlap(m_player, item).y > 0)
            {
                if (hpGain > 0)
                {
                    m_pGame->playSound("health");
                    spawnParticles(item->getComponent<CompTransform>().vPosition, 15, sf::Color::Green);
                    m_player->getComponent<CompHealth>().currentHealth = std::min(m_player->getComponent<CompHealth>().currentHealth + hpGain, 100);
                }
                if (manaGain > 0)
                {
                    m_pGame->playSound("mana");
                    spawnParticles(item->getComponent<CompTransform>().vPosition, 15, sf::Color(100, 200, 255));
                    m_player->getComponent<CompMana>().currentMana = std::min(m_player->getComponent<CompMana>().currentMana + manaGain, 100);
                }
                if (tag == "smallBerry" || tag == "largeBerry")
                {
                    auto &animComp = m_player->getComponent<CompAnimation>();
                    animComp.animation = m_pGame->getAssets().getAnimation("FoxBite");
                    animComp.animation.setLoop(false);
                }
                item->destroy();
            }
        }
    };

    for (Entity *item : m_entityManager.getEntities("boomerangPickup"))
    {
        if (item->isActive() && m_physics.GetOverlap(m_player, item).x > 0 && m_physics.GetOverlap(m_player, item).y > 0)
        {
            if (!m_bBoomerangUnlocked)
            {
                std::ofstream saveFile("save.csv", std::ios::app);
                if (saveFile.is_open())
                {
                    saveFile << "BoomerangUnlocked,1\n";
                    saveFile.close();
                }
                m_bBoomerangUnlocked = true;
            }

            item->destroy();
            m_pGame->playSound("mana");
        }
    }

    handlePickup("manaBerry", 0, 50);
    handlePickup("smallBerry", 20, 0);
    handlePickup("largeBerry", 50, 0);

    if (!onGround && jumpNum < 2 && m_player->getComponent<CompState>().state != EntityState::JUMPING && m_player->getComponent<CompState>().state != EntityState::BURROWING)
    {
        m_player->getComponent<CompState>().state = EntityState::JUMPING;
    }
}