#include "../ScenePlay.h"
#include "Components.h"
#include "GameEngine.h"
#include <cmath>

void ScenePlay::spawnParticles(VectorPP pos, int count, sf::Color color)
{
    for (int i = 0; i < count; i++)
    {
        EffectParticle p;
        p.pos = pos;
        float angle = (rand() % 360) * 3.14159f / 180.0f;
        float speed = (rand() % 60 + 20) / 10.0f;
        p.vel = VectorPP(std::cos(angle) * speed, std::sin(angle) * speed);
        p.vel.y -= (rand() % 50) / 10.0f;
        p.life = 45 + (rand() % 30);
        p.maxLife = p.life;
        p.color = color;
        m_effectParticles.push_back(p);
    }
}

void ScenePlay::sysAnimation()
{
    // Update Particles
    for (int i = 0; i < m_effectParticles.size(); i++)
    {
        EffectParticle &p = m_effectParticles[i];
        p.pos.x += p.vel.x;
        p.pos.y += p.vel.y;
        if (p.color == sf::Color::Green || p.color == sf::Color(100, 200, 255))
            p.vel.y -= 0.2f;
        else
            p.vel.y += 0.5f;
        p.life--;
        if (p.life <= 0)
        {
            m_effectParticles.erase(m_effectParticles.begin() + i);
            i--;
        }
    }

    const auto &assets = m_pGame->getAssets();
    for (Entity *entity : m_entityManager.getEntities())
    {
        if (!entity->hasComponent<CompAnimation>())
            continue;
        CompAnimation &animComp = entity->getComponent<CompAnimation>();
        Animation &anim = animComp.animation;
        std::string currentAnim = anim.getName();
        std::string tag = entity->getTag();

        if (entity == m_player)
        {
            if (currentAnim == "FoxBite" && !animComp.animation.hasEnded())
            {
                animComp.animation.update(m_pGame->getCurrentFrame());
                continue;
            }

            CompState &stateComp = m_player->getComponent<CompState>();
            VectorPP &velocity = m_player->getComponent<CompTransform>().vVelocity;

            if (stateComp.state == EntityState::BURROWING)
            {
                if (stateComp.burrowPhase == 0)
                {
                    if (animComp.animation.hasEnded())
                    {
                        stateComp.burrowPhase = 1;
                        stateComp.burrowTimer = 180;
                    }
                    else
                        animComp.animation.update(m_pGame->getCurrentFrame());
                }
                else if (stateComp.burrowPhase == 1)
                {
                    stateComp.burrowTimer--;
                    if (stateComp.burrowTimer <= 0)
                    {
                        stateComp.burrowPhase = 2;
                        animComp.animation = assets.getAnimation("FoxJump");
                        animComp.animation.setLoop(false);
                    }
                }
                else if (stateComp.burrowPhase == 2)
                {
                    if (animComp.animation.hasEnded())
                        stateComp.state = EntityState::STANDING;
                    else
                        animComp.animation.update(m_pGame->getCurrentFrame());
                }
                continue;
            }

            if (stateComp.state == EntityState::JUMPING)
            {
                if (currentAnim != "FoxJump" && assets.getAllAnimations().count("FoxJump"))
                {
                    animComp.animation = assets.getAnimation("FoxJump");
                    animComp.animation.getSprite().setTextureRect(animComp.animation.getFrame(0));
                }
                if (velocity.y < 0)
                {
                    if (animComp.animation.getCurrentFrame() < 3)
                        animComp.animation.update(m_pGame->getCurrentFrame());
                }
                else
                {
                    if (animComp.animation.getCurrentFrame() < 4)
                    {
                        animComp.animation.update(m_pGame->getCurrentFrame());
                        animComp.animation.update(m_pGame->getCurrentFrame());
                    }
                }
            }
            else if (stateComp.state == EntityState::STANDING)
            {
                if (currentAnim == "FoxJump")
                {
                    if (!animComp.animation.hasEnded())
                        animComp.animation.update(m_pGame->getCurrentFrame());
                    else
                        animComp.animation = assets.getAnimation("FoxStand");
                }
                else if (currentAnim != "FoxStand")
                    animComp.animation = assets.getAnimation("FoxStand");
                else
                    animComp.animation.update(m_pGame->getCurrentFrame());
            }
            else
            {
                std::string targetAnim = "";
                if (stateComp.state == EntityState::RUNNING)
                    targetAnim = "FoxWalking";
                else if (stateComp.state == EntityState::SHOOTING)
                    targetAnim = assets.getAllAnimations().count("FoxShoot") ? "FoxShoot" : "FoxBite";
                if (!targetAnim.empty() && currentAnim != targetAnim)
                    animComp.animation = assets.getAnimation(targetAnim);
                animComp.animation.update(m_pGame->getCurrentFrame());
            }
        }
        else if (entity->hasComponent<CompTransform>() && (tag == "monster" || tag == "mushroom" || tag == "flyingmonster"))
        {
            CompTransform &trans = entity->getComponent<CompTransform>();
            bool isMoving = std::abs(trans.vVelocity.x) > 0.1f;
            std::string targetAnim = currentAnim;
            if (tag == "monster")
            {
                targetAnim = isMoving ? "SlimeWalk" : "Slime";
                float baseScale = assets.getAnimationScale(currentAnim);
                if (isMoving)
                {
                    float wobbleSpeed = 0.25f;
                    float wobbleAmount = 0.1f;
                    float wobble = std::sin(m_pGame->getCurrentFrame() * wobbleSpeed) * wobbleAmount;
                    trans.vScale.y = baseScale + wobble;
                    float facingDir = (trans.vScale.x > 0) ? 1.0f : -1.0f;
                    trans.vScale.x = (baseScale - wobble) * facingDir;
                }
                else
                {
                    trans.vScale.y = baseScale;
                    float facingDir = (trans.vScale.x > 0) ? 1.0f : -1.0f;
                    trans.vScale.x = baseScale * facingDir;
                }
            }
            if (tag == "flyingmonster")
            {
                VectorPP overlap = m_physics.GetOverlap(entity, m_player);
                if (overlap.x > 0 && overlap.y > 0)
                    targetAnim = "BatAttack";
                else
                    targetAnim = "BatFly";
            }
            if (tag != "mushroom")
            {
                if (currentAnim != targetAnim && assets.getAllAnimations().count(targetAnim))
                {
                    animComp.animation = assets.getAnimation(targetAnim);
                    if (targetAnim == "BatAttack")
                        animComp.animation.setLoop(false);
                    else
                        animComp.animation.setLoop(true);
                }
            }
            anim.update(m_pGame->getCurrentFrame());
        }
        else
        {
            anim.update(m_pGame->getCurrentFrame());
            if (anim.hasEnded())
            {
                if (tag == "explosion" || tag == "bullet" || tag == "coin" || tag == "enemy_bullet")
                    entity->destroy();
            }
        }
    }
}