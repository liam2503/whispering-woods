#include "../SceneEditor.h"
#include "GameEngine.h"
#include "Components.h"
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

void SceneEditor::sysRender()
{
    m_pGame->window().clear(sf::Color(40, 40, 40));

    m_worldView.setViewport(m_pGame->getViewport());
    m_pGame->window().setView(m_worldView);

    float viewW = m_worldView.getSize().x;
    float viewH = m_worldView.getSize().y;
    float left = m_worldView.getCenter().x - viewW / 2.0f;
    float top = m_worldView.getCenter().y - viewH / 2.0f;
    m_backgroundSprite.setPosition(left, top);
    m_pGame->window().draw(m_backgroundSprite);

    if (m_bDrawTextures)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CompAnimation>() && e->hasComponent<CompTransform>())
            {
                auto &anim = e->getComponent<CompAnimation>().animation;
                auto &tf = e->getComponent<CompTransform>();

                sf::Vector2f size = sf::Vector2f((float)anim.getSize().x, (float)anim.getSize().y);
                anim.getSprite().setOrigin(size.x / 2.0f, size.y / 2.0f);

                anim.getSprite().setPosition(tf.vPosition.x, tf.vPosition.y);
                anim.getSprite().setScale(tf.vScale.x, tf.vScale.y);
                anim.getSprite().setRotation(tf.fAngle);
                m_pGame->window().draw(anim.getSprite());
            }
        }
    }

    if (m_bDrawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CompBoundingBox>() && e->hasComponent<CompTransform>())
            {
                auto &box = e->getComponent<CompBoundingBox>();
                auto &tf = e->getComponent<CompTransform>();

                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.vSize.x, box.vSize.y));
                rect.setOrigin(box.vSize.x / 2, box.vSize.y / 2);
                rect.setPosition(tf.vPosition.x, tf.vPosition.y);
                rect.setFillColor(sf::Color::Transparent);
                if (m_bEraserMode)
                    rect.setOutlineColor(sf::Color::Red);
                else if (box.dragging)
                    rect.setOutlineColor(sf::Color::Green);
                else
                    rect.setOutlineColor(sf::Color::White);
                rect.setOutlineThickness(1.0f);
                m_pGame->window().draw(rect);
            }
        }
    }

    if (m_bDrawGrid)
    {
        sf::Vector2f center = m_worldView.getCenter();
        sf::Vector2f size = m_worldView.getSize();
        float left = center.x - size.x / 2;
        float right = center.x + size.x / 2;
        float top = center.y - size.y / 2;
        float bottom = center.y + size.y / 2;
        float worldH = (float)height();
        float startX = left - std::fmod(left, m_vGridSize.x);
        for (float x = startX; x < right; x += m_vGridSize.x)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, top), sf::Color(255, 255, 255, 50)),
                sf::Vertex(sf::Vector2f(x, bottom), sf::Color(255, 255, 255, 50))};
            m_pGame->window().draw(line, 2, sf::Lines);
        }

        float gridOffsetY = std::fmod(worldH, m_vGridSize.y);
        float startY = std::floor((top - gridOffsetY) / m_vGridSize.y) * m_vGridSize.y + gridOffsetY;
        for (float y = startY; y < bottom; y += m_vGridSize.y)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(left, y), sf::Color(255, 255, 255, 50)),
                sf::Vertex(sf::Vector2f(right, y), sf::Color(255, 255, 255, 50))};
            m_pGame->window().draw(line, 2, sf::Lines);
        }

        for (float x = startX; x < right; x += m_vGridSize.x)
        {
            for (float y = startY; y < bottom; y += m_vGridSize.y)
            {
                int gridX = (int)(x / m_vGridSize.x);
                int gridY = (int)((worldH - y) / m_vGridSize.y) - 1;
                if (gridX >= 0 && gridY >= 0)
                {
                    std::string coord = std::to_string(gridX) + "," + std::to_string(gridY);
                    m_gridText.setString(coord);
                    m_gridText.setPosition(x + 2, y + 2);
                    m_gridText.setFillColor(sf::Color(255, 255, 255, 100));
                    m_pGame->window().draw(m_gridText);
                }
            }
        }
    }

m_hudView.setViewport(m_pGame->getViewport());
    m_pGame->window().setView(m_hudView);
    float currentHudHeight = 180.0f;
    sf::RectangleShape hudBg(sf::Vector2f((float)width(), currentHudHeight));
    hudBg.setFillColor(sf::Color(0, 0, 0, 200));
    m_pGame->window().draw(hudBg);

    float iconSize = 60.0f;
    float padding = 12.0f;
    float totalPaletteWidth = m_editorPalette.size() * (iconSize + padding) - padding;
    float startX = (width() - totalPaletteWidth) / 2.0f;
    if (startX < 10.0f)
        startX = 10.0f;

    float currentX = startX;
    float currentY = 15.0f;

    for (const auto &animName : m_editorPalette)
    {
        const Animation &anim = m_pGame->getAssets().getAnimation(animName);
        sf::Sprite sprite = anim.getSprite();

        float scaleX = iconSize / anim.getSize().x;
        float scaleY = iconSize / anim.getSize().y;
        float scale = std::min(scaleX, scaleY);

        sprite.setScale(scale, scale);
        sprite.setOrigin(anim.getSize().x / 2, anim.getSize().y / 2);
        float drawX = currentX + (iconSize / 2.0f);
        float drawY = currentY + (iconSize / 2.0f);
        sprite.setPosition(drawX, drawY);

        m_pGame->window().draw(sprite);
        if (m_selectedTile == animName)
        {
            sf::RectangleShape highlight(sf::Vector2f(iconSize, iconSize));
            highlight.setOrigin(iconSize / 2, iconSize / 2);
            highlight.setPosition(drawX, drawY);
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color::Red);
            highlight.setOutlineThickness(2.0f);
            m_pGame->window().draw(highlight);
        }

        currentX += iconSize + padding;
        if (currentX + iconSize > width() - 10.0f)
        {
            currentX = startX;
            currentY += iconSize + padding;
            if (currentY > 90.0f)
                break;
        }
    }

    drawSelectors();

    if (m_bChatBoxPopupOpen)
    {
        drawChatPopup();
    }

    m_pGame->renderFade();

    if (m_statusFrames > 0)
    {
        sf::FloatRect bounds = m_statusText.getLocalBounds();
        float x = width() - bounds.width - 20.0f;
        float y = height() - bounds.height - 20.0f;
        m_statusText.setPosition(x, y);
        m_pGame->window().draw(m_statusText);
    }

    if (m_cursorLoaded)
    {
        sf::Vector2i pixelPos((int)m_vMousePos.x, (int)m_vMousePos.y);
        sf::Vector2f hudCoords = m_pGame->window().mapPixelToCoords(pixelPos, m_hudView);
        
        m_cursorSprite.setPosition(hudCoords.x, hudCoords.y);
        m_pGame->window().draw(m_cursorSprite);
    }

    if (m_editorControllerTexture.getSize().x > 0)
    {
        m_pGame->window().draw(m_editorControllerSprite);
    }

    m_pGame->window().display();
}

void SceneEditor::drawSelectors()
{
    float startY = 90.0f;
    float rowHeight = 28.0f;
    float centerX = width() / 2.0f;

    std::string bgStr = "Select Background: < " + m_bgOptions[m_bgIndex] + " >";
    m_selectorText.setString(bgStr);
    sf::FloatRect bounds = m_selectorText.getLocalBounds();
    m_selectorText.setPosition(centerX - bounds.width / 2.0f, startY);
    m_pGame->window().draw(m_selectorText);
    std::string musStr = "Select Music: < " + m_musicOptions[m_musicIndex] + " >";
    m_selectorText.setString(musStr);
    bounds = m_selectorText.getLocalBounds();
    m_selectorText.setPosition(centerX - bounds.width / 2.0f, startY + rowHeight);
    m_pGame->window().draw(m_selectorText);

    std::string levelName = fs::path(m_levelOptions[m_levelIndex]).filename().string();
    std::string lvlStr = "Select Next Level: < " + levelName + " >";
    m_selectorText.setString(lvlStr);
    bounds = m_selectorText.getLocalBounds();
    m_selectorText.setPosition(centerX - bounds.width / 2.0f, startY + rowHeight * 2);
    m_pGame->window().draw(m_selectorText);
}

void SceneEditor::drawChatPopup()
{
    if (!m_pCurrentChatEntity || !m_pCurrentChatEntity->hasComponent<CompTransform>())
        return;

    VectorPP pos = m_pCurrentChatEntity->getComponent<CompTransform>().vPosition;
    sf::Vector2f entityPos((float)pos.x, (float)pos.y);

    sf::Vector2i entityPixelPos = m_pGame->window().mapCoordsToPixel(entityPos, m_worldView);
    sf::Vector2f hudPos = m_pGame->window().mapPixelToCoords(entityPixelPos, m_hudView);

    float w = 300.0f;
    float h = 100.0f;
    float x = hudPos.x - w / 2.0f;
    float y = hudPos.y - h - 50.0f;

    sf::RectangleShape bg(sf::Vector2f(w, h));
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(0, 0, 0, 200));
    bg.setOutlineColor(sf::Color::White);
    bg.setOutlineThickness(2.0f);
    m_pGame->window().draw(bg);

    m_selectorText.setString("Select Cutscene:");
    m_selectorText.setPosition(x + 10, y + 10);
    m_pGame->window().draw(m_selectorText);
    std::string name = fs::path(m_cutsceneOptions[m_cutsceneIndex]).filename().string();
    m_selectorText.setString("< " + name + " >");
    sf::FloatRect bounds = m_selectorText.getLocalBounds();
    m_selectorText.setPosition(x + w / 2 - bounds.width / 2, y + 40);
    m_pGame->window().draw(m_selectorText);

    m_selectorText.setString("[ OK ]");
    bounds = m_selectorText.getLocalBounds();
    m_selectorText.setPosition(x + w / 2 - bounds.width / 2, y + 70);
    m_pGame->window().draw(m_selectorText);
}