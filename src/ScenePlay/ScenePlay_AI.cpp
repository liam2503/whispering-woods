#include "../ScenePlay.h"
#include "Components.h"
#include "GameEngine.h"
#include <cmath>

void ScenePlay::sysEnemyAI()
{
    if (!m_player)
        return;

    VectorPP playerPos = m_player->getComponent<CompTransform>().vPosition;
    bool isPlayerHidden = (m_player->getComponent<CompState>().state == EntityState::BURROWING && m_player->getComponent<CompState>().burrowPhase == 1);

    for (Entity *entity : m_entityManager.getEntities())
    {
        if (!entity->isActive() || !entity->hasComponent<CompTransform>())
            continue;
        std::string tag = entity->getTag();
        CompTransform &trans = entity->getComponent<CompTransform>();

        if (tag == "monster")
        {
            float dist = std::abs(playerPos.x - trans.vPosition.x);
            float chaseRange = 600.0f;
            float speed = 0.8f;
            float absScale = std::abs(trans.vScale.x);
            if (playerPos.x > trans.vPosition.x)
                trans.vScale.x = -absScale;
            else
                trans.vScale.x = absScale;

            if (dist < chaseRange)
            {
                float dirX = (playerPos.x > trans.vPosition.x) ? 1.0f : -1.0f;
                if (isPlayerHidden)
                    dirX *= -1.0f;
                trans.vVelocity.x = dirX * speed;
                if (std::abs(trans.vVelocity.y) < 1.0f)
                {
                    bool playerAbove = (playerPos.y < trans.vPosition.y - 50.0f);
                    int jumpChance = 1;
                    if (playerAbove)
                        jumpChance += 4;
                    if (rand() % 100 < jumpChance)
                        trans.vVelocity.y = -12.0f;
                }
            }
            else
                trans.vVelocity.x = 0.0f;
        }
        else if (tag == "mushroom")
        {
            float dist = std::abs(playerPos.x - trans.vPosition.x);
            float attackRange = 500.0f;
            float moveSpeed = 0.5f;

            if (!entity->hasComponent<CompChangeDirectionTimer>())
                entity->addComponents<CompChangeDirectionTimer>(100);
            auto &timer = entity->getComponent<CompChangeDirectionTimer>();
            timer.framesRemaining--;

            float absScale = std::abs(trans.vScale.x);
            if (playerPos.x > trans.vPosition.x)
                trans.vScale.x = -absScale;
            else
                trans.vScale.x = absScale;

            CompAnimation &animComp = entity->getComponent<CompAnimation>();
            const auto &assets = m_pGame->getAssets();
            bool isAttacking = (animComp.animation.getName() == "MushroomAttack" && !animComp.animation.hasEnded());

            if (isAttacking)
                trans.vVelocity.x = 0.0f;
            else
            {
                if (dist < attackRange)
                {
                    float dirX = (playerPos.x > trans.vPosition.x) ? 1.0f : -1.0f;
                    if (isPlayerHidden)
                        dirX *= -1.0f;
                    trans.vVelocity.x = dirX * moveSpeed;
                    if (animComp.animation.getName() != "MushroomWalk")
                    {
                        animComp.animation = assets.getAnimation("MushroomWalk");
                        animComp.animation.setLoop(true);
                    }

                    if (timer.framesRemaining <= 0 && !isPlayerHidden)
                    {
                        VectorPP shootPos = trans.vPosition;
                        m_pGame->playSound("mushroom_spit");
                        if (trans.vScale.x < 0)
                            shootPos.x += 30.0f;
                        else
                            shootPos.x -= 30.0f;
                        shootPos.y -= 10.0f;
                        spawnParticles(shootPos, 25, sf::Color(148, 0, 211));

                        for (int i = 0; i < 3; i++)
                        {
                            Entity *spore = m_entityManager.addEntity("enemy_bullet");
                            spore->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Bullet"), true);
                            spore->getComponent<CompAnimation>().animation.getSprite().setColor(sf::Color(148, 0, 211));
                            spore->addComponents<CompTransform>(shootPos);
                            spore->addComponents<CompBoundingBox>(VectorPP(10, 10));
                            spore->addComponents<CompDamage>(15);
                            spore->addComponents<CompLifeSpan>(90, m_pGame->getCurrentFrame());

                            VectorPP dir = playerPos - trans.vPosition;
                            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                            if (len != 0)
                            {
                                dir.x /= len;
                                dir.y /= len;
                            }
                            float baseAngle = std::atan2(dir.y, dir.x);
                            float spreadOffset = (i - 1) * 0.3f;
                            float finalAngle = baseAngle + spreadOffset;
                            float bulletSpeed = 5.0f;
                            spore->getComponent<CompTransform>().vVelocity = VectorPP(std::cos(finalAngle) * bulletSpeed, std::sin(finalAngle) * bulletSpeed);
                        }
                        animComp.animation = assets.getAnimation("MushroomAttack");
                        animComp.animation.setLoop(false);
                        animComp.animation.update(0);
                        timer.framesRemaining = 150;
                    }
                }
                else
                {
                    trans.vVelocity.x = 0.0f;
                    if (animComp.animation.getName() != "MushroomStand")
                    {
                        animComp.animation = assets.getAnimation("MushroomStand");
                        animComp.animation.setLoop(true);
                    }
                }
            }
        }
        else if (tag == "flyingmonster")
        {
            float distX = std::abs(playerPos.x - trans.vPosition.x);
            float distY = playerPos.y - trans.vPosition.y;
            float speed = 2.5f;
            if (distX > 400.0f)
            {
                trans.vVelocity.x = 0.0f;
                trans.vVelocity.y = std::sin(m_pGame->getCurrentFrame() / 20.0f) * 0.5f;
            }
            else
            {
                if (playerPos.x > trans.vPosition.x)
                {
                    trans.vVelocity.x = speed;
                    if (trans.vScale.x > 0)
                        trans.vScale.x *= -1;
                }
                else
                {
                    trans.vVelocity.x = -speed;
                    if (trans.vScale.x < 0)
                        trans.vScale.x *= -1;
                }
                if (isPlayerHidden)
                    trans.vVelocity.x *= -1.0f;
                if (distY > 50.0f)
                    trans.vVelocity.y = 3.5f;
                else if (distY < -50.0f)
                {
                    trans.vVelocity.y = -1.5f;
                }
                else
                    trans.vVelocity.y = 0.0f;
            }
        }
        else if (tag == "boss")
        {
            trans.vVelocity.x = 0.0f;
            trans.vVelocity.y = 0.0f;
            if (playerPos.x > trans.vPosition.x)
            {
                if (trans.vScale.x > 0)
                    trans.vScale.x *= -1;
            }
            else
            {
                if (trans.vScale.x < 0)
                    trans.vScale.x *= -1;
            }

            if (rand() % 300 == 0)
            {
                int enemyType = rand() % 3;
                VectorPP summonPos = trans.vPosition;
                summonPos.y -= 100;
                summonPos.x += (rand() % 200 - 100);
                auto summon = [&](std::string name, std::string anim)
                {
                    Entity *e = m_entityManager.addEntity(name);
                    float scale = m_pGame->getAssets().getAnimationScale(anim);
                    e->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation(anim), true);
                    e->addComponents<CompTransform>(summonPos, VectorPP(0, 0), VectorPP(scale, scale), 0);
                    VectorPP size = m_pGame->getAssets().getAnimation(anim).getSize();
                    e->addComponents<CompBoundingBox>(VectorPP(size.x * scale, size.y * scale));
                    e->addComponents<CompGravity>(0.5);
                    e->addComponents<CompHealth>(30);
                    e->addComponents<CompDamage>(15);
                };
                if (enemyType == 0)
                    summon("monster", "Slime");
                else if (enemyType == 1)
                    summon("mushroom", "MushroomStand");
                else
                    summon("flyingmonster", "Bat");
                spawnParticles(summonPos, 20, sf::Color::Yellow);
            }
            if (entity->hasComponent<CompChangeDirectionTimer>())
            {
                auto &timer = entity->getComponent<CompChangeDirectionTimer>();
                timer.framesRemaining--;
                if (timer.framesRemaining <= 0)
                {
                    m_pGame->playSound("demon_attack");
                    createBossBullet(trans.vPosition, playerPos);
                    createBossBullet(trans.vPosition, VectorPP(playerPos.x, playerPos.y - 200));
                    createBossBullet(trans.vPosition, VectorPP(playerPos.x, playerPos.y + 200));
                    timer.framesRemaining = 150;
                }
            }
        }
    }
}