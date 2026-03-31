#include "../ScenePlay.h"
#include "Components.h"
#include <fstream>
#include <sstream>
#include "GameEngine.h"
#include "Assets.h"
#include <iostream>
#include "ParticleSystem.h"
#include "EntityManager.h"

VectorPP ScenePlay::gridToMidPixel(float gridX, float gridY, Entity *a_pEntity, float scale)
{
    float cellWidth = m_vGridSize.x;
    float cellHeight = m_vGridSize.y;

    float worldHeight = (float)height();

    if (!a_pEntity->hasComponent<CompAnimation>())
    {
        float pixelX = gridX * cellWidth;
        float pixelY = worldHeight - (gridY * cellHeight);
        return VectorPP(pixelX, pixelY);
    }

    Animation &anim = a_pEntity->getComponent<CompAnimation>().animation;
    VectorPP animSize = anim.getSize();

    float pixelX = (gridX * cellWidth) + (animSize.x * scale / 2.0f);
    float pixelY = worldHeight - (gridY * cellHeight) - (animSize.y * scale / 2.0f);

    return VectorPP(pixelX, pixelY);
}

void ScenePlay::loadLevel(const std::string &a_strFilePath)
{
    m_currentPath = a_strFilePath;
    m_vGridSize = VectorPP(64.0f, 64.0f);

    m_entityManager = EntityManager();

    m_activeChunks.clear();
    m_worldChunks.clear();

    std::ifstream infile(a_strFilePath);
    if (!infile.is_open())
    {
        std::cerr << "Failed to open " << a_strFilePath << "!\n";
        std::exit(10);
    }

    std::string strTemp;
    std::string textureName = "TexLevelBG1";
    std::string musicFile = "Assets/Music/menu.ogg";

    if (std::getline(infile, strTemp) && !strTemp.empty())
    {
        std::stringstream ss(strTemp);
        ss >> textureName;
    }

    if (std::getline(infile, strTemp) && !strTemp.empty())
    {
        std::stringstream ss(strTemp);
        ss >> musicFile;

        if (musicFile.find("Assets/Music/") == std::string::npos)
        {
            musicFile = "Assets/Music/" + musicFile;
        }
    }

    if (std::getline(infile, strTemp) && !strTemp.empty())
    {
        m_strNextLevel = strTemp;
    }

    m_pGame->requestMusic(musicFile, m_pGame->getVoiceVolume());

    if (m_pGame->getAssets().getTexture(textureName).getSize().x == 0)
    {
        std::cout << "Warning: Texture '" << textureName << "' not found. Using default.\n";
        textureName = "TexLevelBG1";
    }

    m_backgroundSprite.setTexture(m_pGame->getAssets().getTexture(textureName));

    if (m_backgroundSprite.getTexture())
    {
        sf::Vector2u texSize = m_backgroundSprite.getTexture()->getSize();
        float scaleX = (float)width() / texSize.x;
        float scaleY = (float)height() / texSize.y;
        m_backgroundSprite.setScale(scaleX, scaleY);
    }

    const auto &assets = m_pGame->getAssets();
    const auto &allAnims = assets.getAllAnimations();

    while (std::getline(infile, strTemp))
    {
        if (strTemp.empty())
            continue;
        if (strTemp[0] == '/' && strTemp[1] == '/')
            continue;

        std::stringstream lineStream(strTemp);
        std::string header;
        lineStream >> header;

        if (header == "Player")
        {
            lineStream >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.BBOX_WIDTH >> m_playerConfig.BBOX_HEIGHT >> m_playerConfig.SPEED_X >> m_playerConfig.SPEED_Y >> m_playerConfig.SPEED_MAX >> m_playerConfig.GRAVITY >> m_playerConfig.WEAPON;
        }
        else if (header == "Tile" || header == "Dec")
        {
            std::string tileType;
            float gridX, gridY;
            lineStream >> tileType >> gridX >> gridY;

            int chunkX = static_cast<int>(gridX) / m_chunkSize;
            int chunkY = static_cast<int>(gridY) / m_chunkSize;

            TileConfig config;
            config.type = tileType;
            config.gridX = gridX;
            config.gridY = gridY;
            config.tag = (header == "Tile") ? "tile" : "decoration";

            if (tileType == "ChatBox")
            {
                std::string script;
                if (lineStream >> script)
                    config.script = script;
            }
            m_worldChunks[{chunkX, chunkY}].push_back(config);
        }
        else if (header == "Line")
        {
            float x1, y1, x2, y2;
            lineStream >> x1 >> y1 >> x2 >> y2;
            VectorPP start = VectorPP(x1, y1);
            VectorPP end = VectorPP(x2, y2);
            drawLine(start, end);
        }
        else
        {
            m_strNextLevel = strTemp;
        }
    }
    infile.close();

    createPlayer();
    createFamiliar();
    sysChunkManagement();
}

void ScenePlay::createFamiliar()
{
    particles.init(256, 2.0f, m_player->getComponent<CompTransform>().vPosition);
}

void ScenePlay::createPlayer()
{
    const std::string PLAYER_ANIM_NAME = "FoxStand";
    float CUSTOM_SCALE_FACTOR = m_pGame->getAssets().getAnimationScale(PLAYER_ANIM_NAME);
    if (CUSTOM_SCALE_FACTOR <= 0.0f)
    {
        std::cout << "WARNING: Player Scale is 0! Defaulting to 1.0f\n";
        CUSTOM_SCALE_FACTOR = 1.0f;
    }

    m_player = m_entityManager.addEntity("player");
    m_player->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation(PLAYER_ANIM_NAME), true);

    m_player->addComponents<CompTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player, CUSTOM_SCALE_FACTOR));

    m_player->getComponent<CompTransform>().vScale.x = CUSTOM_SCALE_FACTOR;
    m_player->getComponent<CompTransform>().vScale.y = CUSTOM_SCALE_FACTOR;

    VectorPP animSize = m_pGame->getAssets().getAnimation(PLAYER_ANIM_NAME).getSize();
    int sizeX = static_cast<int>(animSize.x * CUSTOM_SCALE_FACTOR);
    int sizeY = static_cast<int>(animSize.y * CUSTOM_SCALE_FACTOR);

    m_player->addComponents<CompBoundingBox>(VectorPP(sizeX, sizeY));

    m_player->addComponents<CompGravity>(m_playerConfig.GRAVITY);
    m_player->addComponents<CompInput>();
    m_player->getComponent<CompInput>().canJump = false;
    jumpNum = 0;

    m_player->addComponents<CompHealth>(100);
    m_player->addComponents<CompMana>(100);
    m_player->addComponents<CompStatus>();
}

void ScenePlay::reloadLayer()
{
    reloading = true;
    loadLevel(m_currentPath);
    reloading = false;
}

Entity *ScenePlay::createMonster(VectorPP a_vPos)
{
    Entity *monster = m_entityManager.addEntity("monster");

    const float SCALE = m_pGame->getAssets().getAnimationScale("Slime");

    monster->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Slime"), true);
    monster->addComponents<CompTransform>(gridToMidPixel(a_vPos.x, a_vPos.y, monster, SCALE), VectorPP(1.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("Slime").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    monster->addComponents<CompBoundingBox>(scaledSize);
    monster->addComponents<CompGravity>(0.5);
    monster->addComponents<CompHealth>(50);
    monster->addComponents<CompDamage>(10);

    return monster;
}

Entity *ScenePlay::createMushroom(VectorPP a_vPos)
{
    Entity *mushroom = m_entityManager.addEntity("mushroom");

    const float SCALE = m_pGame->getAssets().getAnimationScale("MushroomStand");

    mushroom->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("MushroomStand"), true);
    mushroom->addComponents<CompTransform>(gridToMidPixel(a_vPos.x, a_vPos.y, mushroom, SCALE), VectorPP(1.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("MushroomStand").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    mushroom->addComponents<CompBoundingBox>(scaledSize);
    mushroom->addComponents<CompGravity>(0.5);
    mushroom->addComponents<CompHealth>(30);
    mushroom->addComponents<CompDamage>(20);

    return mushroom;
}

Entity *ScenePlay::createFlyingMonster(VectorPP a_vPos)
{
    Entity *FlyingMonster = m_entityManager.addEntity("flyingmonster");

    const float SCALE = m_pGame->getAssets().getAnimationScale("Bat");

    FlyingMonster->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Bat"), true);
    FlyingMonster->addComponents<CompTransform>(gridToMidPixel(a_vPos.x, a_vPos.y, FlyingMonster, SCALE), VectorPP(1.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("Bat").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    FlyingMonster->addComponents<CompBoundingBox>(scaledSize);
    FlyingMonster->addComponents<CompHealth>(30);
    FlyingMonster->addComponents<CompDamage>(15);

    return FlyingMonster;
}

Entity *ScenePlay::createBoss(VectorPP a_vPos)
{
    Entity *boss = m_entityManager.addEntity("boss");

    const float SCALE = m_pGame->getAssets().getAnimationScale("Demon");
    boss->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Demon"), true);
    boss->addComponents<CompTransform>(gridToMidPixel(a_vPos.x, a_vPos.y, boss, SCALE), VectorPP(1.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("Demon").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);
    boss->addComponents<CompBoundingBox>(scaledSize);

    boss->addComponents<CompGravity>(0.0f);
    boss->addComponents<CompHealth>(300);
    boss->addComponents<CompDamage>(50);

    boss->addComponents<CompChangeDirectionTimer>(0);

    return boss;
}

Entity *ScenePlay::createManaFlower(VectorPP a_vPos)
{
    Entity *item = m_entityManager.addEntity("manaFlower");
    const float SCALE = m_pGame->getAssets().getAnimationScale("ManaFlower");

    item->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("ManaFlower"), true);
    item->addComponents<CompTransform>(gridToMidPixel(a_vPos.x, a_vPos.y, item, SCALE), VectorPP(0.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("ManaFlower").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    item->addComponents<CompBoundingBox>(scaledSize);

    return item;
}

Entity *ScenePlay::createManaBerry(VectorPP a_vPos)
{
    Entity *item = m_entityManager.addEntity("manaBerry");
    const float SCALE = m_pGame->getAssets().getAnimationScale("Mana");

    item->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Mana"), true);

    item->addComponents<CompTransform>(a_vPos, VectorPP(0.0f, -5.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("Mana").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    item->addComponents<CompBoundingBox>(scaledSize);

    item->addComponents<CompGravity>(0.5f);

    return item;
}

Entity *ScenePlay::createSmallBerry(VectorPP a_vPos)
{
    Entity *item = m_entityManager.addEntity("smallBerry");
    const float SCALE = m_pGame->getAssets().getAnimationScale("BerriesSmall");

    item->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("BerriesSmall"), true);
    
    item->addComponents<CompTransform>(a_vPos, VectorPP(0.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("BerriesSmall").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    item->addComponents<CompBoundingBox>(scaledSize);

    return item;
}

Entity *ScenePlay::createLargeBerry(VectorPP a_vPos)
{
    Entity *item = m_entityManager.addEntity("largeBerry");
    const float SCALE = m_pGame->getAssets().getAnimationScale("BerriesLarge");

    item->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("BerriesLarge"), true);
    
    item->addComponents<CompTransform>(a_vPos, VectorPP(0.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("BerriesLarge").getSize();
    VectorPP scaledSize = VectorPP(animSize.x * SCALE, animSize.y * SCALE);

    item->addComponents<CompBoundingBox>(scaledSize);

    return item;
}

Entity *ScenePlay::createBoomerangPickup(VectorPP a_vPos)
{
    Entity *item = m_entityManager.addEntity("boomerangPickup");
    const float SCALE = m_pGame->getAssets().getAnimationScale("Boomerang");
    item->addComponents<CompAnimation>(m_pGame->getAssets().getAnimation("Boomerang"), true);

    item->addComponents<CompTransform>(gridToMidPixel(a_vPos.x, a_vPos.y, item, SCALE), VectorPP(0.0f, 0.0f), VectorPP(SCALE, SCALE), 0.0f);

    VectorPP animSize = m_pGame->getAssets().getAnimation("Boomerang").getSize();
    item->addComponents<CompBoundingBox>(VectorPP(animSize.x * SCALE, animSize.y * SCALE));

    return item;
}

void ScenePlay::drawWalkableLine(const VectorPP &a_vStart, const VectorPP &a_vEnd)
{
    Entity *lineEntity = m_entityManager.addEntity("line");
    lineEntity->addComponents<CompLine>(a_vStart, a_vEnd);

    float centerX = (a_vStart.x + a_vEnd.x) / 2.0f;
    float centerY = (a_vStart.y + a_vEnd.y) / 2.0f;

    float dx = std::abs(a_vEnd.x - a_vStart.x);
    float dy = std::abs(a_vEnd.y - a_vStart.y);

    VectorPP bboxSize;
    if (dx >= dy)
        bboxSize = VectorPP(dx, 4.0f);
    else
        bboxSize = VectorPP(4.0f, dy);

    lineEntity->addComponents<CompTransform>(VectorPP(centerX, centerY));
}

void ScenePlay::sysChunkManagement()
{
    if (!m_player)
        return;

    VectorPP playerPos = m_player->getComponent<CompTransform>().vPosition;
    
    float gridX = playerPos.x / 64.0f;
    float gridY = (height() - playerPos.y) / 64.0f;

    int pChunkX = static_cast<int>(std::floor(gridX)) / m_chunkSize;
    int pChunkY = static_cast<int>(std::floor(gridY)) / m_chunkSize;

    // Load chunk
    int radius = 1;
    std::vector<ChunkCoord> neededChunks;
    for (int x = pChunkX - radius; x <= pChunkX + radius; x++)
    {
        for (int y = pChunkY - radius; y <= pChunkY + radius; y++)
        {
            neededChunks.push_back({x, y});
        }
    }

    // Spawn needed chunks
    for (auto &coord : neededChunks)
    {
        if (m_worldChunks.count(coord) && m_activeChunks.find(coord) == m_activeChunks.end())
        {
            spawnChunk(coord);
        }
    }

    // Unload active chunks
    auto it = m_activeChunks.begin();
    while (it != m_activeChunks.end())
    {
        ChunkCoord coord = *it;
        bool stillNeeded = false;

        for (auto &needed : neededChunks)
        {
            if (needed == coord)
            {
                stillNeeded = true;
                break;
            }
        }

        if (!stillNeeded)
        {
            unloadChunk(coord);
            it = m_activeChunks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ScenePlay::spawnChunk(ChunkCoord coord)
{
    if (m_worldChunks.find(coord) == m_worldChunks.end())
        return;

    const auto &configs = m_worldChunks[coord];

    for (const auto &conf : configs)
    {
        std::string tileType = conf.type;
        float gridX = conf.gridX;
        float gridY = conf.gridY;
        VectorPP pos(gridX, gridY);

        Entity *e = nullptr;
        if (tileType == "Boomerang")
            e = createBoomerangPickup(pos);
        else if (tileType == "Slime")
            e = createMonster(pos);
        else if (tileType == "Mushroom")
            e = createMushroom(pos);
        else if (tileType == "Bat")
            e = createFlyingMonster(pos);
        else if (tileType == "Demon")
            e = createBoss(pos);
        else if (tileType == "ManaFlower")
            e = createManaFlower(pos);
        else if (tileType == "BerriesSmall")
        {
            e = createSmallBerry(VectorPP(0, 0));
            float scale = m_pGame->getAssets().getAnimationScale("BerriesSmall");
            e->getComponent<CompTransform>().vPosition = gridToMidPixel(gridX, gridY, e, scale);
        }
        else if (tileType == "BerriesLarge")
        {
            e = createLargeBerry(VectorPP(0, 0));
            float scale = m_pGame->getAssets().getAnimationScale("BerriesLarge");
            e->getComponent<CompTransform>().vPosition = gridToMidPixel(gridX, gridY, e, scale);
        }

        if (e == nullptr)
        {
            const auto &assets = m_pGame->getAssets();
            const auto &allAnims = assets.getAllAnimations();

            if (allAnims.find(tileType) == allAnims.end())
                continue;

            e = m_entityManager.addEntity(conf.tag);

            if (tileType == "ChatBox" && !conf.script.empty())
            {
                e->addComponents<CompCutscene>(conf.script);
            }

            float scale = assets.getAnimationScale(tileType);
            if (scale <= 0.0f)
                scale = 1.0f;

            e->addComponents<CompAnimation>(assets.getAnimation(tileType), true);
            VectorPP pixelPos = gridToMidPixel(gridX, gridY, e, scale);
            e->addComponents<CompTransform>(pixelPos);
            e->getComponent<CompTransform>().vScale = VectorPP(scale, scale);

            if (tileType == "TileGrassMid" || tileType == "TileGrassLeft" || tileType == "TileGrassRight")
            {
                e->addComponents<CompFallingTile>(pixelPos);
                e->addComponents<CompGravity>(0.0f);
            }

            VectorPP animSize = assets.getAnimation(tileType).getSize();
            e->addComponents<CompBoundingBox>(VectorPP(animSize.x * scale, animSize.y * scale));
            e->getComponent<CompBoundingBox>().dragging = false;

            if (conf.tag == "tile")
            {
                e->getComponent<CompTransform>().vPrevPos = pixelPos;
            }
        }

        if (e != nullptr)
        {
            e->addComponents<CompChunk>(coord.first, coord.second);
        }
    }

    m_activeChunks.insert(coord);
}

void ScenePlay::unloadChunk(ChunkCoord coord)
{
    if (m_activeChunks.find(coord) == m_activeChunks.end())
        return;

    for (auto e : m_entityManager.getEntities())
    {
        if (!e->isActive() || !e->hasComponent<CompChunk>())
            continue;

        const auto &chunkC = e->getComponent<CompChunk>();
        if (chunkC.x == coord.first && chunkC.y == coord.second)
        {
            e->destroy();
        }
    }
}