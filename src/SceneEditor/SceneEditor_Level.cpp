#include "../SceneEditor.h"
#include "GameEngine.h"
#include "Components.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>


void SceneEditor::loadLevel(const std::string &a_strFilePath)
{
    m_entityManager = EntityManager();
    m_entityScripts.clear();

    std::ifstream infile(a_strFilePath);

    std::string strTemp;

    if (std::getline(infile, strTemp))
    {
        if (!strTemp.empty() && strTemp.back() == '\r')
            strTemp.pop_back();
        m_strBackgroundTexture = strTemp;

        auto it = std::find(m_bgOptions.begin(), m_bgOptions.end(), m_strBackgroundTexture);
        if (it != m_bgOptions.end())
            m_bgIndex = std::distance(m_bgOptions.begin(), it);
    }

    if (std::getline(infile, strTemp))
    {
        if (!strTemp.empty() && strTemp.back() == '\r')
            strTemp.pop_back();
        m_savedMusicPath = strTemp;

        auto it = std::find(m_musicOptions.begin(), m_musicOptions.end(), m_savedMusicPath);
        if (it != m_musicOptions.end())
            m_musicIndex = std::distance(m_musicOptions.begin(), it);

        m_pGame->requestMusic("Assets/Music/" + m_savedMusicPath, 100.0f);
    }

    if (std::getline(infile, strTemp))
    {
        if (!strTemp.empty() && strTemp.back() == '\r')
            strTemp.pop_back();
        m_savedNextLevelPath = strTemp;

        auto it = std::find(m_levelOptions.begin(), m_levelOptions.end(), m_savedNextLevelPath);
        if (it != m_levelOptions.end())
            m_levelIndex = std::distance(m_levelOptions.begin(), it);
    }

    auto &assets = m_pGame->getAssets();
    if (assets.getTexture(m_strBackgroundTexture).getSize().x > 0)
    {
        m_backgroundSprite.setTexture(assets.getTexture(m_strBackgroundTexture));
    }
    else
    {
        m_backgroundSprite.setTexture(assets.getTexture("Mysterious_Forest"));
    }

    if (m_backgroundSprite.getTexture())
    {
        sf::Vector2u texSize = m_backgroundSprite.getTexture()->getSize();
        float scaleX = (float)width() / texSize.x;
        float scaleY = (float)height() / texSize.y;
        m_backgroundSprite.setScale(scaleX, scaleY);
    }

    while (std::getline(infile, strTemp))
    {
        if (strTemp.empty() || (strTemp[0] == '/' && strTemp[1] == '/'))
            continue;
        std::stringstream lineStream(strTemp);
        std::string header;
        lineStream >> header;

        if (header == "Player")
        {
            float gx, gy, bboxW, bboxH, sx, sy, sMax, grav;
            std::string weapon;
            lineStream >> gx >> gy >> bboxW >> bboxH >> sx >> sy >> sMax >> grav >> weapon;
            std::string spawnAnim = "SpawnPoint";
            const auto &allAnims = assets.getAllAnimations();

            if (allAnims.find(spawnAnim) == allAnims.end())
            {
                if (allAnims.find("FoxStand") != allAnims.end())
                    spawnAnim = "FoxStand";
                else
                    spawnAnim = "TileGrassMid";
            }

            Entity *e = m_entityManager.addEntity("spawnpoint");
            const Animation &anim = assets.getAnimation(spawnAnim);
            e->addComponents<CompAnimation>(anim, true);

            float scale = assets.getAnimationScale(spawnAnim);
            e->addComponents<CompTransform>(gridToMidPixel(gx, gy, e, scale), VectorPP(0, 0), VectorPP(scale, scale), 0.0f);
            VectorPP size = anim.getSize();
            e->addComponents<CompBoundingBox>(VectorPP(size.x * scale, size.y * scale));
            e->getComponent<CompBoundingBox>().dragging = false;
        }
        else if (header == "Tile" || header == "Dec")
        {
            std::string type;
            float gx, gy;
            lineStream >> type >> gx >> gy;

            Entity *e = m_entityManager.addEntity(header == "Tile" ? "tile" : "decoration");
            const Animation &anim = m_pGame->getAssets().getAnimation(type);
            e->addComponents<CompAnimation>(anim, true);

            float scale = m_pGame->getAssets().getAnimationScale(type);
            e->addComponents<CompTransform>(gridToMidPixel(gx, gy, e, scale), VectorPP(0, 0), VectorPP(scale, scale), 0.0f);
            VectorPP size = anim.getSize();
            e->addComponents<CompBoundingBox>(VectorPP(size.x * scale, size.y * scale));
            e->getComponent<CompBoundingBox>().dragging = false;
            if (type == "ChatBox")
            {
                std::string scriptPath;
                if (lineStream >> scriptPath)
                    m_entityScripts[e] = scriptPath;
            }
        }
    }
}

void SceneEditor::saveLevel()
{
    m_pGame->setLoadingScreen(true, "Saving Level...");
    
    std::string savePath = m_strLevelPath;
    std::ofstream outfile(savePath);
    if (!outfile.is_open())
    {
        setStatus("Error: Cannot save!", sf::Color::Red);
        return;
    }

    outfile << m_strBackgroundTexture << "\n";
    outfile << (m_savedMusicPath.empty() ? "Title_Theme.ogg" : m_savedMusicPath) << "\n";
    outfile << (m_savedNextLevelPath.empty() ? "Assets/Levels/Default.txt" : m_savedNextLevelPath) << "\n\n";

    m_pGame->setLoadingProgress(0.1f);
    m_pGame->window().clear(sf::Color::Black);
    m_pGame->renderFade();
    m_pGame->window().display();

    const auto& entities = m_entityManager.getEntities();
    size_t totalEntities = entities.size();
    size_t processedCount = 0;
    float lastReportedProgress = 0.0f;

    float pGridX = 3;
    float pGridY = 3;
    float worldH = (float)height();

    for (auto e : entities)
    {
        if (e->isActive() && e->hasComponent<CompAnimation>() && 
            e->getComponent<CompAnimation>().animation.getName() == "SpawnPoint")
        {
            VectorPP pos = e->getComponent<CompTransform>().vPosition;
            VectorPP animSize = e->getComponent<CompAnimation>().animation.getSize();
            float scale = e->getComponent<CompTransform>().vScale.x;
            pGridX = std::round((pos.x - (animSize.x * scale / 2.0f)) / m_vGridSize.x);
            pGridY = std::round((worldH - pos.y - (animSize.y * scale / 2.0f)) / m_vGridSize.y);
            break;
        }
    }
    outfile << "Player " << pGridX << " " << pGridY << " 64 64 10 15 20 0.5 MELEE\n\n";

    for (auto e : entities)
    {
        if (e->isActive() && e->hasComponent<CompAnimation>() && e->hasComponent<CompTransform>())
        {
            std::string name = e->getComponent<CompAnimation>().animation.getName();
            if (name != "SpawnPoint")
            {
                std::string header = (e->getTag() == "decoration") ? "Dec" : "Tile";
                VectorPP pos = e->getComponent<CompTransform>().vPosition;
                VectorPP animSize = e->getComponent<CompAnimation>().animation.getSize();
                float scale = e->getComponent<CompTransform>().vScale.x;
                float gridX = (pos.x - (animSize.x * scale / 2.0f)) / m_vGridSize.x;
                float gridY = (worldH - pos.y - (animSize.y * scale / 2.0f)) / m_vGridSize.y;

                outfile << header << " " << name << " " << std::round(gridX) << " " << std::round(gridY);
                if (name == "ChatBox" && m_entityScripts.count(e))
                {
                    outfile << " " << m_entityScripts[e];
                }
                outfile << "\n";
            }
        }

        processedCount++;
        float currentProgress = 0.1f + (static_cast<float>(processedCount) / totalEntities) * 0.9f;

        if (currentProgress - lastReportedProgress >= 0.01f)
        {
            m_pGame->setLoadingProgress(currentProgress);
            m_pGame->window().clear(sf::Color::Black);
            m_pGame->renderFade(); 
            m_pGame->window().display();
            lastReportedProgress = currentProgress;
        }
    }

    outfile.close();
}