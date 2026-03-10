#include "../ScenePlay.h"
#include "Components.h"
#include "GameEngine.h"
#include "ParticleSystem.h"

void ScenePlay::sysRender()
{
    if (!m_bIsPaused)
        m_pGame->window().clear(sf::Color(100, 100, 255));
    else
        m_pGame->window().clear(sf::Color(50, 50, 150));

    VectorPP &pPos = m_player->getComponent<CompTransform>().vPosition;
    sf::View view = m_pGame->window().getView();
    float halfW = view.getSize().x / 2.0f;
    float viewX = std::max(halfW, pPos.x);
    float halfH = view.getSize().y / 2.0f;
    float targetY = pPos.y - 100.0f;
    float viewY = std::min(targetY, height() - halfH);
    if (height() < view.getSize().y)
        viewY = halfH;

    view.setCenter(viewX, viewY);
    m_pGame->window().setView(view);

    // Draw Background
    float camTopLeftX = view.getCenter().x - (view.getSize().x / 2.0f);
    float camTopLeftY = view.getCenter().y - halfH;
    m_backgroundSprite.setPosition(camTopLeftX, camTopLeftY);
    m_pGame->window().draw(m_backgroundSprite);

    // Draw Entities
    if (m_bDrawTextures)
    {
        for (Entity *entity : m_entityManager.getEntities())
        {
            if (entity->getTag() == "player")
            {
                auto &st = m_player->getComponent<CompState>();
                if (st.state == EntityState::BURROWING && st.burrowPhase == 1)
                    continue;
            }

            if (entity->getTag() == "tile" && entity->hasComponent<CompAnimation>())
            {
                if (entity->getComponent<CompAnimation>().animation.getName() == "LevelGoal")
                    continue;
            }

            CompTransform &transform = entity->getComponent<CompTransform>();
            if (entity->hasComponent<CompAnimation>())
            {
                Animation &animation = entity->getComponent<CompAnimation>().animation;
                sf::Sprite &sprite = animation.getSprite();
                sprite.setRotation(transform.fAngle);
                sprite.setPosition(transform.vPosition.x, transform.vPosition.y);
                sprite.setScale(transform.vScale.x, transform.vScale.y);

                if (entity->hasComponent<CompFallingTile>())
                {
                    float op = entity->getComponent<CompFallingTile>().opacity;
                    if (op < 0)
                        op = 0;
                    if (op > 255)
                        op = 255;
                    sprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(op)));
                }

                sf::Shader *shaderToUse = nullptr;
                if (entity->hasComponent<CompInvincible>())
                {
                    CompInvincible &inv = entity->getComponent<CompInvincible>();
                    if (inv.isInvincible && inv.framesRemaining > 0)
                        shaderToUse = &m_damageShader;
                }
                if (!shaderToUse && entity->hasComponent<CompStatus>())
                {
                    CompStatus &statusComp = entity->getComponent<CompStatus>();
                    if (statusComp.status == EntityStatus::FROZEN)
                        shaderToUse = &m_frozenShader;
                    else if (statusComp.status == EntityStatus::FIRE)
                        shaderToUse = &m_burningShader;
                    else if (statusComp.status == EntityStatus::POISIONED)
                        shaderToUse = &m_poisonedShader;
                }
                if (shaderToUse)
                {
                    shaderToUse->setUniform("texture", sf::Shader::CurrentTexture);
                    m_pGame->window().draw(sprite, shaderToUse);
                }
                else
                    m_pGame->window().draw(sprite);
            }
        }
    }

    // Minimap
    sf::View minimapView;
    sf::Vector2u windowSize = m_pGame->window().getSize();
    float zoomFactor = 2.0f;
    float heightRatio = 0.7f;
    minimapView.setSize(windowSize.x * zoomFactor, windowSize.y * zoomFactor * heightRatio);

    if (m_player)
    {
        auto &pPos = m_player->getComponent<CompTransform>().vPosition;
        float mmHalfWidth = minimapView.getSize().x / 2.0f;
        float mmHalfHeight = minimapView.getSize().y / 2.0f;
        float centerX = std::max(mmHalfWidth, pPos.x);
        float centerY = std::max(mmHalfHeight, std::min(pPos.y, (float)height() - mmHalfHeight));
        minimapView.setCenter(centerX, centerY);
    }

    minimapView.setViewport(sf::FloatRect(0.75f, 0.0f, 0.25f, 0.25f * heightRatio));
    m_pGame->window().setView(minimapView);

    sf::RectangleShape mmBg;

    mmBg.setSize(sf::Vector2f(windowSize.x * zoomFactor, windowSize.y * zoomFactor * heightRatio));

    mmBg.setOrigin(windowSize.x * zoomFactor / 2.0f, (windowSize.y * zoomFactor * heightRatio) / 2.0f);

    mmBg.setPosition(minimapView.getCenter());
    mmBg.setFillColor(sf::Color(0, 0, 0, 150));
    m_pGame->window().draw(mmBg);

    sf::RectangleShape tileShape;
    sf::CircleShape unitShape;
    for (auto &entity : m_entityManager.getEntities())
    {
        if (!entity->hasComponent<CompTransform>())
            continue;
        auto &pos = entity->getComponent<CompTransform>().vPosition;
        std::string tag = entity->getTag();

        if (tag == "tile" && entity->hasComponent<CompAnimation>())
        {
            if (entity->getComponent<CompAnimation>().animation.getName() == "LevelGoal")
            {
                continue;
            }
            if (entity->getComponent<CompAnimation>().animation.getName() == "ChatBox")
            {
                continue;
            }
        }

        if (tag == "tile" || entity->hasComponent<CompFallingTile>())
        {
            tileShape.setPosition(pos.x, pos.y);
            if (entity->hasComponent<CompBoundingBox>())
            {
                VectorPP size = entity->getComponent<CompBoundingBox>().vSize;
                tileShape.setSize(sf::Vector2f(size.x, size.y));
            }
            else
                tileShape.setSize(sf::Vector2f(64.0f, 64.0f));
            tileShape.setFillColor(sf::Color::White);
            m_pGame->window().draw(tileShape);
        }
        else if (tag == "player")
        {
            float radius = 32.0f;
            unitShape.setRadius(radius);
            unitShape.setPosition(pos.x, pos.y);
            unitShape.setFillColor(sf::Color::Yellow);
            m_pGame->window().draw(unitShape);
        }
    }

    m_pGame->window().setView(m_pGame->window().getDefaultView());
    m_pGame->window().setView(view);

    // Familiar and Particles
    int familiarOffsetX = 60;
    int familiarOffsetY = -90;
    if (m_player->getComponent<CompTransform>().vScale.x < 0)
        familiarOffsetX = -familiarOffsetX;
    familiarPosition = VectorPP(
        m_player->getComponent<CompTransform>().vPosition.x + familiarOffsetX,
        m_player->getComponent<CompTransform>().vPosition.y + familiarOffsetY);
    particles.setEmitter(familiarPosition);
    particles.update();
    particles.draw(m_pGame->window());

    sf::VertexArray vertArray(sf::Quads);
    for (const auto &p : m_effectParticles)
    {
        float size = 4.0f;
        sf::Color c = p.color;
        c.a = static_cast<sf::Uint8>(255 * ((float)p.life / (float)p.maxLife));
        vertArray.append(sf::Vertex(sf::Vector2f(p.pos.x - size, p.pos.y - size), c));
        vertArray.append(sf::Vertex(sf::Vector2f(p.pos.x + size, p.pos.y - size), c));
        vertArray.append(sf::Vertex(sf::Vector2f(p.pos.x + size, p.pos.y + size), c));
        vertArray.append(sf::Vertex(sf::Vector2f(p.pos.x - size, p.pos.y + size), c));
    }
    m_pGame->window().draw(vertArray, sf::BlendAdd);

    // HUD
    if (m_bDrawPlayerHUD)
    {
        sf::View hudView = m_pGame->window().getDefaultView();
        m_pGame->window().setView(hudView);
        if (m_bShowDialogue)
        {
            float viewWidth = hudView.getSize().x;
            float viewHeight = hudView.getSize().y;

            m_dialogueSprite.setPosition(viewWidth / 2.0f, viewHeight - 20.0f);
            m_pGame->window().draw(m_dialogueSprite);

            sf::FloatRect bounds = m_dialogueSprite.getGlobalBounds();

            m_speakerText.setPosition(
                m_dialogueSprite.getPosition().x - bounds.width / 2.0f + 90.0f,
                m_dialogueSprite.getPosition().y - bounds.height + 5.0f);
            m_pGame->window().draw(m_speakerText);

            m_dialogueText.setPosition(
                m_dialogueSprite.getPosition().x - bounds.width / 2.0f + 45.0f,
                m_dialogueSprite.getPosition().y - bounds.height + 50.0f);
            m_pGame->window().draw(m_dialogueText);
        }
        else
        {
            float margin = 20.0f;
            sf::Sprite hudFrame(m_pGame->getAssets().getTexture("TexHUD"));
            float hudScale = 0.17f;
            hudFrame.setScale(hudScale, hudScale);
            hudFrame.setPosition(margin, margin);

            float barX = margin + (115.0f);
            float hpY = margin + (67.0f);
            float manaY = margin + (85.0f);
            float barWidth = 190.0f;
            float barHeight = 15.0f;
            CompHealth &healthComp = m_player->getComponent<CompHealth>();
            CompMana &manaComp = m_player->getComponent<CompMana>();

            float hpRatio = healthComp.health > 0 ? (float)healthComp.currentHealth / healthComp.health : 0.0f;
            float manaRatio = manaComp.mana > 0 ? (float)manaComp.currentMana / manaComp.mana : 0.0f;

            sf::RectangleShape fillHp(sf::Vector2f(barWidth * hpRatio, barHeight));
            fillHp.setPosition(barX, hpY);
            fillHp.setFillColor(sf::Color(200, 0, 0));
            m_pGame->window().draw(fillHp);

            sf::RectangleShape fillMana(sf::Vector2f((barWidth - 10) * manaRatio, barHeight));
            fillMana.setPosition(barX - 15, manaY);
            fillMana.setFillColor(sf::Color(30, 144, 255));
            m_pGame->window().draw(fillMana);

            m_pGame->window().draw(hudFrame);

            sf::Text livesText;
            livesText.setFont(m_pGame->getAssets().getFont("Pixeloid"));
            livesText.setCharacterSize(26);
            livesText.setFillColor(sf::Color::White);
            livesText.setOutlineColor(sf::Color::Black);
            livesText.setOutlineThickness(1.0f);
            livesText.setString("x" + std::to_string(liveNum));
            float livesX = margin + (62.0f);
            float livesY = margin + (65.0f);
            livesText.setPosition(livesX, livesY);
            m_pGame->window().draw(livesText);

            sf::Text statusInfo;
            statusInfo.setFont(m_pGame->getAssets().getFont("Pixeloid"));
            statusInfo.setCharacterSize(14);
            statusInfo.setFillColor(sf::Color::White);
            statusInfo.setString(std::to_string(healthComp.currentHealth) + "/" + std::to_string(healthComp.health));
            statusInfo.setPosition(barX + 10, hpY - 1);
            m_pGame->window().draw(statusInfo);
            statusInfo.setString(std::to_string(manaComp.currentMana) + "/" + std::to_string(manaComp.mana));
            statusInfo.setPosition(barX + 10, manaY - 1);
            m_pGame->window().draw(statusInfo);
            if (m_bBoomerangUnlocked)
            {
                const Animation &boomAnim = m_pGame->getAssets().getAnimation("Boomerang");
                sf::Sprite boomSprite = boomAnim.getSprite();

                float iconX = barX + barWidth + 65.0f;
                float iconY = hpY + 15.0f;

                boomSprite.setPosition(iconX, iconY);
                boomSprite.setScale(1.0f, 1.0f);

                m_pGame->window().draw(boomSprite);
            }
        }
    }

    m_pGame->window().setView(view);

    // Enemy Health Bars
    if (m_bDrawPlayerHUD)
    {
        for (Entity *entity : m_entityManager.getEntities())
        {
            if (entity->getTag() != "player" && entity->hasComponent<CompHealth>())
            {
                CompHealth &healthComp = entity->getComponent<CompHealth>();
                float hpRatio = 0.0f;
                CompTransform &transform = entity->getComponent<CompTransform>();
                float monsterX = transform.vPosition.x - (100.0f / 2.0f);
                float monsterY = transform.vPosition.y - 50.0f;
                float barWidth = 100.0f;
                float barHeight = 5.0f;
                if (healthComp.health > 0)
                    hpRatio = healthComp.currentHealth / (float)healthComp.health;

                sf::RectangleShape bgHp(sf::Vector2f(barWidth, barHeight));
                bgHp.setPosition(monsterX, monsterY);
                bgHp.setFillColor(sf::Color::White);
                m_pGame->window().draw(bgHp);

                sf::RectangleShape fillHp(sf::Vector2f(barWidth * hpRatio, barHeight));
                fillHp.setPosition(monsterX, monsterY);
                fillHp.setFillColor(sf::Color::Red);
                m_pGame->window().draw(fillHp);

                sf::RectangleShape borderHp(sf::Vector2f(barWidth, barHeight));
                borderHp.setPosition(monsterX, monsterY);
                borderHp.setFillColor(sf::Color::Transparent);
                borderHp.setOutlineThickness(2.0f);
                borderHp.setOutlineColor(sf::Color::Black);
                m_pGame->window().draw(borderHp);
            }
        }
    }

    // Game Over / Fade Overlays
    m_pGame->window().setView(m_pGame->window().getDefaultView());
    if (m_bShowGameOver || m_bShowEndScreen)
    {
        sf::RectangleShape darkenOverlay(sf::Vector2f((float)width(), (float)height()));
        float darkenProgress = (m_fScreenFadeAlpha / 255.0f);
        sf::Uint8 darkenAlpha = static_cast<sf::Uint8>(180 * darkenProgress);
        darkenOverlay.setFillColor(sf::Color(0, 0, 0, darkenAlpha));
        m_pGame->window().draw(darkenOverlay);

        sf::Uint8 screenAlpha = static_cast<sf::Uint8>(m_fScreenFadeAlpha);
        if (m_bShowGameOver)
        {
            m_gameOverSprite.setColor(sf::Color(255, 255, 255, screenAlpha));
            m_pGame->window().draw(m_gameOverSprite);
        }
        else if (m_bShowEndScreen)
        {
            m_endSprite.setColor(sf::Color(255, 255, 255, screenAlpha));
            m_pGame->window().draw(m_endSprite);
        }
    }

    if (m_controllerTexture.getSize().x > 0)
    {
        m_pGame->window().draw(m_controllerSprite);
    }

    m_pGame->renderFade();
    if (m_strLevelPath != "Assets/Levels/Level Editor.txt")
    {
        m_pGame->window().display();
    }
}