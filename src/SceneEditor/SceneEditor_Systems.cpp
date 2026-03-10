#include "../SceneEditor.h"
#include "GameEngine.h"
#include "../SceneMenu.h"
#include "Components.h"
#include <cmath>
#include <iostream>

void SceneEditor::update()
{
    m_entityManager.update();

    int joyID = m_pGame->getJoystickID();
    if (sf::Joystick::isConnected(joyID))
    {
        float axisX = sf::Joystick::getAxisPosition(joyID, sf::Joystick::X);
        float axisY = sf::Joystick::getAxisPosition(joyID, sf::Joystick::Y);

        if (std::abs(axisX) > 15.0f || std::abs(axisY) > 15.0f)
        {
            float speed = 10.0f;
            m_vMousePos.x += (axisX * 0.01f) * speed;
            m_vMousePos.y += (axisY * 0.01f) * speed;
            if (m_vMousePos.x < 0)
                m_vMousePos.x = 0;
            if (m_vMousePos.y < 0)
                m_vMousePos.y = 0;
            if (m_vMousePos.x > width())
                m_vMousePos.x = (float)width();
            if (m_vMousePos.y > height())
                m_vMousePos.y = (float)height();
        }
    }

    m_cursorSprite.setPosition(m_vMousePos.x, m_vMousePos.y);
    if (m_bCamLeft)
        m_worldView.move(-m_cameraSpeed, 0);
    if (m_bCamRight)
        m_worldView.move(m_cameraSpeed, 0);
    if (m_bCamUp)
        m_worldView.move(0, -m_cameraSpeed);
    if (m_bCamDown)
        m_worldView.move(0, m_cameraSpeed);
    if (m_statusFrames > 0)
    {
        m_statusFrames--;
        if (m_statusFrames < 60)
        {
            float alpha = 255.0f * ((float)m_statusFrames / 60.0f);
            sf::Color c = m_statusText.getFillColor();
            m_statusText.setFillColor(sf::Color(c.r, c.g, c.b, static_cast<sf::Uint8>(alpha)));
        }
    }

    sDragAndDrop();
    sysRender();
}

void SceneEditor::sDragAndDrop()
{
    if (m_bEraserMode)
        return;
    if (m_bChatBoxPopupOpen)
        return;
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CompBoundingBox>() && e->getComponent<CompBoundingBox>().dragging)
        {
            VectorPP mouseWorld = windowToWorld(m_vMousePos);
            VectorPP animSize = e->getComponent<CompAnimation>().animation.getSize();
            float worldH = (float)m_pGame->window().getSize().y;
            float scale = e->getComponent<CompTransform>().vScale.x;
            float gx = std::round((mouseWorld.x - (animSize.x * scale / 2)) / m_vGridSize.x);
            float gy = std::round((worldH - mouseWorld.y - (animSize.y * scale / 2)) / m_vGridSize.y);

            e->getComponent<CompTransform>().vPosition = gridToMidPixel(gx, gy, e, scale);
        }
    }
}

void SceneEditor::sysDoAction(const Action &a_action)
{
    if (a_action.getState() == "START")
    {
        if (a_action.getName() == "QUIT")
        {
            saveLevel();
            onEnd();
        }

        if (a_action.getName() == "SAVE_LEVEL")
            saveLevel();
        if (a_action.getName() == "TOGGLE_GRID")
            m_bDrawGrid = !m_bDrawGrid;
        if (a_action.getName() == "TOGGLE_TEXTURE")
            m_bDrawTextures = !m_bDrawTextures;
        if (a_action.getName() == "TOGGLE_COLLISION")
            m_bDrawCollision = !m_bDrawCollision;
        if (a_action.getName() == "TOGGLE_ERASER")
        {
            m_bEraserMode = !m_bEraserMode;
            if (m_bEraserMode)
                setStatus("Eraser Mode ON", sf::Color::Red);
            else
                setStatus("Eraser Mode OFF", sf::Color(0xFFffae00));
        }

        if (a_action.getName() == "LEFT_CLICK")
        {
            VectorPP clickPos = m_vMousePos;
            if (m_bChatBoxPopupOpen && m_pCurrentChatEntity)
            {
                VectorPP pos = m_pCurrentChatEntity->getComponent<CompTransform>().vPosition;
                sf::Vector2f entityPos((float)pos.x, (float)pos.y);

                sf::Vector2i pixelPos = m_pGame->window().mapCoordsToPixel(entityPos, m_worldView);
                float w = 300.0f;
                float h = 100.0f;
                float x = pixelPos.x - w / 2.0f;
                float y = pixelPos.y - h - 50.0f;
                if (clickPos.x >= x && clickPos.x <= x + w && clickPos.y >= y && clickPos.y <= y + h)
                {
                    if (clickPos.y > y + 65)
                    {
                        m_entityScripts[m_pCurrentChatEntity] = m_cutsceneOptions[m_cutsceneIndex];
                        m_bChatBoxPopupOpen = false;
                        m_pCurrentChatEntity = nullptr;
                        setStatus("Cutscene Set");
                        m_pGame->playSound("apply");
                    }
                    else if (clickPos.y > y + 35)
                    {
                        if (clickPos.x < x + w / 2)
                        {
                            if (!m_cutsceneOptions.empty())
                            {
                                m_cutsceneIndex = (m_cutsceneIndex == 0) ? m_cutsceneOptions.size() - 1 : m_cutsceneIndex - 1;
                                m_pGame->playSound("ui");
                            }
                        }
                        else
                        {
                            if (!m_cutsceneOptions.empty())
                            {
                                m_cutsceneIndex = (m_cutsceneIndex + 1) % m_cutsceneOptions.size();
                                m_pGame->playSound("ui");
                            }
                        }
                    }
                }
                return;
            }

            float currentHudHeight = 180.0f;
            if (clickPos.y < currentHudHeight)
            {
                if (clickPos.y < 85.0f)
                {
                    if (m_bEraserMode)
                    {
                        m_bEraserMode = false;
                        setStatus("Eraser Mode OFF", sf::Color(0xFFffae00));
                    }

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
                        if (clickPos.x >= currentX && clickPos.x <= currentX + iconSize &&
                            clickPos.y >= currentY && clickPos.y <= currentY + iconSize)
                        {
                            m_selectedTile = animName;
                            setStatus("Selected: " + m_selectedTile, sf::Color(0xFFffae00));
                            m_pGame->playSound("ui");
                            break;
                        }

                        currentX += iconSize + padding;
                        if (currentX + iconSize > width() - 10.0f)
                        {
                            currentX = startX;
                            currentY += iconSize + padding;
                            if (currentY > 85.0f)
                                break;
                        }
                    }
                }
                else
                {
                    float startY = 90.0f;
                    float rowHeight = 28.0f;
                    float centerX = width() / 2.0f;
                    auto handleSelector = [&](size_t &index, size_t max, float yPos)
                    {
                        if (std::abs(clickPos.y - (yPos + 10.0f)) < 14.0f)
                        {
                            if (clickPos.x < centerX)
                            {
                                if (max > 0)
                                {
                                    index = (index == 0) ? max - 1 : index - 1;
                                    m_pGame->playSound("ui");
                                }
                            }
                            else
                            {
                                if (max > 0)
                                {
                                    index = (index + 1) % max;
                                    m_pGame->playSound("ui");
                                }
                            }
                            return true;
                        }
                        return false;
                    };

                    if (handleSelector(m_bgIndex, m_bgOptions.size(), startY))
                    {
                        m_strBackgroundTexture = m_bgOptions[m_bgIndex];
                        auto &assets = m_pGame->getAssets();
                        if (assets.getTexture(m_strBackgroundTexture).getSize().x > 0)
                            m_backgroundSprite.setTexture(assets.getTexture(m_strBackgroundTexture));
                    }
                    else if (handleSelector(m_musicIndex, m_musicOptions.size(), startY + rowHeight))
                    {
                        m_savedMusicPath = m_musicOptions[m_musicIndex];
                        m_pGame->requestMusic("Assets/Music/" + m_savedMusicPath, 100.0f);
                    }
                    else if (handleSelector(m_levelIndex, m_levelOptions.size(), startY + rowHeight * 2))
                    {
                        m_savedNextLevelPath = m_levelOptions[m_levelIndex];
                    }
                }
            }
            else
            {
                VectorPP worldPos = windowToWorld(clickPos);
                if (m_bEraserMode)
                {
                    bool deleted = false;
                    for (auto e : m_entityManager.getEntities())
                    {
                        if (isInside(worldPos, e))
                        {
                            e->destroy();
                            deleted = true;
                        }
                    }
                    if (deleted)
                    {
                        setStatus("Tile Deleted", sf::Color::Red);
                        m_pGame->playSound("return");
                    }
                }
                else
                {
                    bool interacted = false;
                    for (auto e : m_entityManager.getEntities())
                    {
                        if (isInside(worldPos, e))
                        {
                            if (e->hasComponent<CompBoundingBox>())
                            {
                                e->getComponent<CompBoundingBox>().dragging = !e->getComponent<CompBoundingBox>().dragging;
                                interacted = true;
                            }
                        }
                    }

                    if (!interacted && !m_selectedTile.empty())
                    {
                        if (m_selectedTile == "SpawnPoint")
                        {
                            std::vector<Entity *> toDelete;
                            for (auto e : m_entityManager.getEntities())
                            {
                                if (e->getTag() == "spawnpoint")
                                    toDelete.push_back(e);
                            }
                            for (auto e : toDelete)
                                e->destroy();
                        }

                        float worldH = (float)m_pGame->window().getSize().y;
                        float gx = std::floor(worldPos.x / m_vGridSize.x);
                        float gy = std::floor((worldH - worldPos.y) / m_vGridSize.y);
                        std::string tag = (m_selectedTile == "SpawnPoint") ? "spawnpoint" : ((m_selectedTile.substr(0, 3) == "Dec") ? "decoration" : "tile");
                        Entity *e = m_entityManager.addEntity(tag);
                        const Animation &anim = m_pGame->getAssets().getAnimation(m_selectedTile);
                        e->addComponents<CompAnimation>(anim, true);

                        float scale = m_pGame->getAssets().getAnimationScale(m_selectedTile);
                        e->addComponents<CompTransform>(gridToMidPixel(gx, gy, e, scale), VectorPP(0, 0), VectorPP(scale, scale), 0.0f);

                        VectorPP size = anim.getSize();
                        e->addComponents<CompBoundingBox>(VectorPP(size.x * scale, size.y * scale));
                        e->getComponent<CompBoundingBox>().dragging = false;

                        if (m_selectedTile == "ChatBox")
                        {
                            m_pCurrentChatEntity = e;
                            m_bChatBoxPopupOpen = true;
                            m_cutsceneIndex = 0;
                            if (m_entityScripts.count(e))
                            {
                                auto it = std::find(m_cutsceneOptions.begin(), m_cutsceneOptions.end(), m_entityScripts[e]);
                                if (it != m_cutsceneOptions.end())
                                    m_cutsceneIndex = std::distance(m_cutsceneOptions.begin(), it);
                            }
                        }
                    }
                }
            }
        }

        if (a_action.getName() == "RIGHT_CLICK")
        {
            if (!m_selectedTile.empty())
            {
                m_selectedTile = "";
                setStatus("Selection Cleared", sf::Color::White);
            }
        }
    }

    if (a_action.getName() == "CAM_LEFT")
        m_bCamLeft = (a_action.getState() == "START");
    if (a_action.getName() == "CAM_RIGHT")
        m_bCamRight = (a_action.getState() == "START");
    if (a_action.getName() == "CAM_UP")
        m_bCamUp = (a_action.getState() == "START");
    if (a_action.getName() == "CAM_DOWN")
        m_bCamDown = (a_action.getState() == "START");
    if (a_action.getName() == "MOUSE_MOVE")
    {
        m_vMousePos = a_action.getPos();
        m_cursorSprite.setPosition(m_vMousePos.x, m_vMousePos.y);
    }
}

void SceneEditor::onEnd()
{
    m_pGame->playSound("return");
    m_pGame->window().setMouseCursorVisible(true);
    m_pGame->window().setView(m_pGame->window().getDefaultView());
    m_pGame->changeScene("MENU", nullptr, true, false);
}

VectorPP SceneEditor::windowToWorld(const VectorPP &a_vWindowPos)
{
    sf::Vector2f viewCenter = m_worldView.getCenter();
    sf::Vector2f viewSize = m_worldView.getSize();
    float currentX = viewCenter.x - (viewSize.x / 2.0f);
    float currentY = viewCenter.y - (viewSize.y / 2.0f);
    return VectorPP(currentX + a_vWindowPos.x, currentY + a_vWindowPos.y);
}

VectorPP SceneEditor::gridToMidPixel(float gridX, float gridY, Entity *a_pEntity, float scale)
{
    Animation &anim = a_pEntity->getComponent<CompAnimation>().animation;
    VectorPP animSize = anim.getSize();
    float worldHeight = (float)m_pGame->window().getSize().y;

    float pixelX = (gridX * m_vGridSize.x) + (animSize.x * scale / 2.0f);
    float pixelY = worldHeight - (gridY * m_vGridSize.y) - (animSize.y * scale / 2.0f);
    return VectorPP(pixelX, pixelY);
}

bool SceneEditor::isInside(const VectorPP &pos, Entity *entity)
{
    if (!entity->hasComponent<CompTransform>() || !entity->hasComponent<CompBoundingBox>())
        return false;
    auto &tf = entity->getComponent<CompTransform>();
    auto &box = entity->getComponent<CompBoundingBox>();
    float halfW = box.vSize.x / 2.0f;
    float halfH = box.vSize.y / 2.0f;
    bool xOverlap = (pos.x >= tf.vPosition.x - halfW) && (pos.x <= tf.vPosition.x + halfW);
    bool yOverlap = (pos.y >= tf.vPosition.y - halfH) && (pos.y <= tf.vPosition.y + halfH);
    return xOverlap && yOverlap;
}

void SceneEditor::setStatus(const std::string &text, const sf::Color &color)
{
    m_statusText.setString(text);
    m_statusText.setFillColor(color);
    m_statusFrames = 120;
}