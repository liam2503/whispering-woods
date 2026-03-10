#include "../ScenePlay.h"
#include "Components.h"
#include "GameEngine.h"

void ScenePlay::sysMovement()
{
    // Player Input Movement
    if (m_player->getComponent<CompState>().state != EntityState::BURROWING)
    {
        if (m_player->getComponent<CompInput>().left)
        {
            if (m_player->getComponent<CompStatus>().status != FROZEN)
            {
                m_player->getComponent<CompTransform>().vVelocity.x -= 0.5f;
                changeScale(m_player);
            }
            if (m_player->getComponent<CompTransform>().vVelocity.x < -m_playerConfig.SPEED_X)
                m_player->getComponent<CompTransform>().vVelocity.x = -m_playerConfig.SPEED_X;
        }
        else if (m_player->getComponent<CompInput>().right)
        {
            if (m_player->getComponent<CompStatus>().status != FROZEN)
            {
                m_player->getComponent<CompTransform>().vVelocity.x += 0.5f;
                changeScale(m_player);
            }
            if (m_player->getComponent<CompTransform>().vVelocity.x > m_playerConfig.SPEED_X)
                m_player->getComponent<CompTransform>().vVelocity.x = m_playerConfig.SPEED_X;
        }
        else
        {
            if (m_player->getComponent<CompTransform>().vVelocity.x > 0)
                m_player->getComponent<CompTransform>().vVelocity.x -= 0.5f;
            else if (m_player->getComponent<CompTransform>().vVelocity.x < 0)
                m_player->getComponent<CompTransform>().vVelocity.x += 0.5f;
        }

        if (m_player->getComponent<CompState>().state == EntityState::JUMPING &&
            m_player->getComponent<CompInput>().up)
        {
            float maxUpwardSpeed = -m_playerConfig.SPEED_Y * 1.5f;
            if (m_player->getComponent<CompTransform>().vVelocity.y > maxUpwardSpeed)
            {
                m_player->getComponent<CompTransform>().vVelocity.y -= 0.3f;
            }
        }
    }

    // Entity Gravity & Position Updates
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (entity->getTag() == "player" && entity->getComponent<CompState>().state == EntityState::BURROWING)
            continue;
        if (entity->isActive() || entity->hasComponent<CompGravity>())
        {
            CompTransform &entityTransform = entity->getComponent<CompTransform>();
            entityTransform.vPrevPos = entityTransform.vPosition;
            entityTransform.vVelocity.y += entity->getComponent<CompGravity>().fGravity;
            entityTransform.vPosition += entityTransform.vVelocity;
        }
    }

    // Player Velocity Caps
    if (m_player->getComponent<CompTransform>().vVelocity.y > m_playerConfig.SPEED_MAX)
        m_player->getComponent<CompTransform>().vVelocity.y = m_playerConfig.SPEED_MAX;
    if (m_player->getComponent<CompTransform>().vVelocity.y < -m_playerConfig.SPEED_MAX)
        m_player->getComponent<CompTransform>().vVelocity.y = -m_playerConfig.SPEED_MAX;

    // Ground State Logic
    if (onGround)
    {
        if (m_player->getComponent<CompState>().state != EntityState::BURROWING)
        {
            if ((m_player->getComponent<CompInput>().left || m_player->getComponent<CompInput>().right))
                m_player->getComponent<CompState>().state = EntityState::RUNNING;
            else
                m_player->getComponent<CompState>().state = EntityState::STANDING;
        }
    }

    // Bullet Updates
    for (Entity *bullet : m_entityManager.getEntities("bullet"))
    {
        if (!bullet->isActive() || !bullet->hasComponent<CompTransform>())
            continue;
        CompTransform &BulletTransform = bullet->getComponent<CompTransform>();
        BulletTransform.vPrevPos = BulletTransform.vPosition;
        BulletTransform.vPosition += BulletTransform.vVelocity;
    }

    // Enemy Updates
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (!entity->isActive() || !entity->hasComponent<CompTransform>() || (entity->getTag() != "monster" && entity->getTag() != "mushroom" && entity->getTag() != "boss"))
            continue;
        CompTransform &entityTransform = entity->getComponent<CompTransform>();
        entityTransform.vPosition += entityTransform.vVelocity;
    }

    // Leaf/Boomerang Updates
    for (Entity *leaf : m_entityManager.getEntities("leaf"))
    {
        if (!leaf->isActive() || !leaf->hasComponent<CompTransform>() || !leaf->hasComponent<CompBoomerang>())
            continue;
        auto &transform = leaf->getComponent<CompTransform>();
        auto &boomer = leaf->getComponent<CompBoomerang>();
        VectorPP &vel = transform.vVelocity;
        float frameDist = std::sqrt(vel.x * vel.x + vel.y * vel.y);
        boomer.totalDistance += frameDist;

        float maxDist = transform.fAngle;
        if (!boomer.returning)
        {
            const float gravity = 0.35f;
            transform.vPosition.x += vel.x;
            transform.vPosition.y += vel.y + 0.5f * gravity;
            vel.y += gravity;
            if (boomer.totalDistance > maxDist * 0.5f && boomer.totalDistance < maxDist)
            {
                float sideX = -vel.y;
                float sideY = vel.x;
                float len = std::sqrt(sideX * sideX + sideY * sideY);
                if (len > 0.001f)
                {
                    sideX /= len;
                    sideY /= len;
                }
                float curveForce = 0.15f;
                vel.x += sideX * curveForce;
                vel.y += sideY * curveForce;
            }

            if (boomer.totalDistance >= maxDist)
            {
                boomer.returning = true;
                vel.y = -std::abs(vel.y) * 0.3f;
                VectorPP playerPos = m_player->getComponent<CompTransform>().vPosition;
                boomer.angle = std::atan2(transform.vPosition.y - playerPos.y, transform.vPosition.x - playerPos.x);
            }
        }
        else
        {
            VectorPP playerPos = m_player->getComponent<CompTransform>().vPosition;
            boomer.angle += 0.15f;
            float radius = 50.0f;

            VectorPP targetPos;
            targetPos.x = playerPos.x + radius * std::cos(boomer.angle);
            targetPos.y = playerPos.y + radius * std::sin(boomer.angle);

            VectorPP dir = targetPos - transform.vPosition;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.001f)
            {
                dir.x /= len;
                dir.y /= len;
            }

            float followSpeed = 8.0f;
            vel.x = dir.x * followSpeed;
            vel.y = dir.y * followSpeed;
            if (len < 20.0f)
                leaf->destroy();
        }
    }
}