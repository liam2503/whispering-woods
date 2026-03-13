#include "../ScenePlay.h"
#include "Components.h"
#include <cmath>
#include <iostream>
#include "../SceneMenu.h"
#include "Components.h"

void ScenePlay::knockBack(Entity *a_pEntity)
{
    CompTransform &transform = a_pEntity->getComponent<CompTransform>();
    transform.vVelocity.x = (transform.vScale.x) * -10.0f;
    transform.vVelocity.y = -10.0f;
}

void ScenePlay::invincible(Entity *a_pEntity)
{
    const int invincibleFrames = 60;

    if (!a_pEntity->hasComponent<CompInvincible>())
    {
        a_pEntity->addComponents<CompInvincible>(invincibleFrames);
    }
    else
    {
        CompInvincible &invincibility = a_pEntity->getComponent<CompInvincible>();
        invincibility.isInvincible = true;
        invincibility.framesRemaining = invincibleFrames;
        invincibility.has = true;
    }
}

void ScenePlay::changeScale(Entity *a_pEntity)
{
    const int flipFrames = 12;
    float currScaleX = a_pEntity->getComponent<CompTransform>().vScale.x;
    float velocityX = a_pEntity->getComponent<CompTransform>().vVelocity.x;

    int sign;
    if (velocityX > 0.1f)
        sign = 1;
    else if (velocityX < -0.1f)
        sign = -1;
    else
        sign = (int)currScaleX;

    if ((currScaleX > 0 && sign < 0) || (currScaleX < 0 && sign > 0))
    {
        if (!a_pEntity->hasComponent<CompFlipScale>())
            a_pEntity->addComponents<CompFlipScale>(flipFrames);

        CompFlipScale &flip = a_pEntity->getComponent<CompFlipScale>();

        if (flip.has && flip.targetSign == sign && flip.framesRemaining > 0)
            return;

        flip.totalFrames = flipFrames;
        flip.framesRemaining = flip.totalFrames;
        flip.targetSign = sign;
        flip.has = true;
    }
}

void ScenePlay::createMeleeAttack()
{
    CompTransform &playerTransform = m_player->getComponent<CompTransform>();
    VectorPP attackPos = playerTransform.vPosition;

    float attackDistance = 500.0f;
    attackPos.x += attackDistance * playerTransform.vScale.x;

    Entity *melee = m_entityManager.addEntity("Melee");
    melee->addComponents<CompTransform>(attackPos);
    melee->addComponents<CompBoundingBox>(VectorPP(128, 115));
    melee->addComponents<CompLifeSpan>(60, m_pGame->getCurrentFrame());
    melee->addComponents<CompDamage>((int)(10 * m_damageDealtMult));
}

void ScenePlay::createElementBall(const VectorPP &familiarPos, const VectorPP &a_vMousePos, EntityStatus m_currentElement)
{
    VectorPP spawnPos = familiarPos;
    Entity *bullet = m_entityManager.addEntity("bullet");

    bullet->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Bullet"), true);
    bullet->addComponents<CompDamage>((int)(30 * m_damageDealtMult));

    auto &status = bullet->addComponents<CompStatus>();
    status.status = m_currentElement;

    float bulletSpeed = 10.0f;
    VectorPP direction = a_vMousePos - spawnPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0)
    {
        direction.x /= length;
        direction.y /= length;
    }
    bullet->addComponents<CompTransform>(spawnPos, VectorPP(bulletSpeed * direction.x, bulletSpeed * direction.y), VectorPP(1.0f, 1.0f), 0.0f);
    bullet->addComponents<CompBoundingBox>(VectorPP(8, 8));
}

void ScenePlay::createLeaf(const VectorPP &familiarPos, const VectorPP &targetPos)
{
    Entity *leaf = m_entityManager.addEntity("leaf");

    VectorPP dir = targetPos - familiarPos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len != 0.0f)
    {
        dir.x /= len;
        dir.y /= len;
    }
    else
    {
        
        float direction = (m_player->getComponent<CompTransform>().vScale.x > 0) ? 1.0f : -1.0f;
        dir = VectorPP(direction, 0.0f);
    }

    leaf->addComponents<CompAnimation>(
        m_pGame->getAssets().getAnimation("Boomerang"), true);

    const float speed = 10.0f;
    const float maxDist = 400.0f;

    leaf->addComponents<CompTransform>(
        familiarPos,
        VectorPP(dir.x * speed, dir.y * speed),
        VectorPP(1.0f, 1.0f),
        maxDist);

    leaf->addComponents<CompDamage>((int)(15 * m_damageDealtMult));
    leaf->addComponents<CompBoundingBox>(VectorPP(8, 8));

    auto &boomer = leaf->addComponents<CompBoomerang>();
    boomer.totalDistance = 0.0f;
    boomer.returning = false;
}

void ScenePlay::spawnBoomerang(Entity *entity, const VectorPP &targetPos)
{
    if (m_bBoomerangUnlocked)
    {
        VectorPP origin = entity->getComponent<CompTransform>().vPosition;
        createLeaf(origin, targetPos);
    }
}

void ScenePlay::createBullet(const VectorPP &familiarPos, const VectorPP &a_vMousePos)
{
    VectorPP spawnPos = familiarPos;
    Entity *bullet = m_entityManager.addEntity("bullet");
    bullet->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Bullet"), true);

    float bulletSpeed = 10.0f;
    VectorPP direction = a_vMousePos - spawnPos;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0)
    {
        direction.x /= length;
        direction.y /= length;
    }
    bullet->addComponents<CompTransform>(spawnPos, VectorPP(bulletSpeed * direction.x, bulletSpeed * direction.y), VectorPP(1.0f, 1.0f), 0.0f);
    bullet->addComponents<CompBoundingBox>(VectorPP(8, 8));
    bullet->addComponents<CompDamage>((int)(5 * m_damageDealtMult));
}

void ScenePlay::sysHealthDetection()
{
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (entity->hasComponent<CompHealth>())
        {
            CompHealth &health = entity->getComponent<CompHealth>();
            if (health.currentHealth <= 0)
            {
                if (entity->getTag() == "player" || entity->getTag() == "player2")
                {
                    if (liveNum <= 0)
                    {
                        m_player->getComponent<CompBoundingBox>().isActive = false;
                        m_bShowGameOver = true;
                        m_fScreenFadeAlpha = 0.0f;
                        setPaused(true); 
                    }
                    else
                    {
                        liveNum -= 1;
                        m_bIsRespawningPan = true;
                        m_fPanProgress = 0.0f;
                        m_vPanStart = m_player->getComponent<CompTransform>().vPosition;
                        float scale = m_pGame->getAssets().getAnimationScale("FoxStand");
                        m_vPanTarget = gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player, scale);
                        
                        health.currentHealth = health.health;
                        m_player->getComponent<CompTransform>().vVelocity = VectorPP(0, 0);
                        if (m_player->hasComponent<CompGravity>()) 
                            m_player->getComponent<CompGravity>().fGravity = 0.0f;
                        if (m_player->hasComponent<CompBoundingBox>()) 
                            m_player->getComponent<CompBoundingBox>().isActive = false;
                    }
                }
                else
                {
                    entity->destroy();
                }
            }
        }
    }
}

void ScenePlay::createBossBullet(VectorPP origin, VectorPP target)
{
    Entity *bullet = m_entityManager.addEntity("enemy_bullet");

    bullet->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Bullet"), true);
    bullet->getComponent<CompAnimation>().animation.getSprite().setColor(sf::Color::Red);

    float scale = 2.0f;
    bullet->addComponents<CompTransform>(origin, VectorPP(0, 0), VectorPP(scale, scale), 0);
    bullet->addComponents<CompBoundingBox>(VectorPP(16 * scale, 16 * scale));
    bullet->addComponents<CompDamage>(20);
    bullet->addComponents<CompLifeSpan>(120, m_pGame->getCurrentFrame());

    VectorPP diff = target - origin;
    float length = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    VectorPP velocity = VectorPP(0, 0);

    if (length != 0)
    {
        velocity = VectorPP(diff.x / length, diff.y / length);
    }

    float speed = 7.0f;
    bullet->getComponent<CompTransform>().vVelocity = VectorPP(velocity.x * speed, velocity.y * speed);
}