#include "Assets.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

Assets::Assets()
{

    std::ofstream logFile("assets_error_log.txt", std::ios::trunc);
}

void Assets::logError(const std::string &message) const
{

    std::cerr << message << std::endl;

    std::ofstream logFile("assets_error_log.txt", std::ios::app);
    if (logFile.is_open())
    {
        logFile << message << "\n";
    }
}

void Assets::loadFromFile(const std::string &a_strPath)
{

    std::ifstream stream(a_strPath);
    std::string line, word, type, asset;

    while (getline(stream, line))
    {

        if (line.empty() || line.size() == 1)
        {

            continue;
        }

        if (line[0] == '/' && line[1] == '/')
        {

            continue;
        }

        std::stringstream ss(line);
        ss >> asset;

        if (asset == "Texture")
        {
            std::string name, path;
            ss >> name >> path;
            addTexture(name, path);
        }
        else if (asset == "Animation")
        {
            std::string name, texture;
            size_t frames;
            float speed;
            int loop_int;
            float scale;

            ss >> name >> texture >> frames >> speed >> loop_int >> scale;

            if (ss.fail())
            {
                logError("ERROR: Failed to parse Animation line. Check data types (name, texture, frames, speed, loop[0/1], scale): " + line);
                continue;
            }

            addAnimation(name, texture, frames, speed, (bool)loop_int, scale);
        }
        else if (asset == "Font")
        {
            std::string name, path;
            ss >> name >> path;
            addFont(name, path);
        }
        else if (asset == "Sound")
        {
            std::string name, path;
            float volume;
            ss >> name >> path >> volume;
            addSound(name, path, volume);
        }
        else
        {
            logError("Unknown Asset Type: " + asset);
        }
    }
}

void Assets::addTexture(const std::string &textureName, const std::string &a_strPath, bool a_bSmooth)
{
    m_mapTextures[textureName] = sf::Texture();

    if (!m_mapTextures[textureName].loadFromFile(a_strPath))
    {
        logError("Failed to load texture: " + textureName + " from path: " + a_strPath);
        m_mapTextures.erase(textureName);
    }
    else
    {
        m_mapTextures[textureName].setSmooth(a_bSmooth);
        std::cout << "Loaded Texture: " << a_strPath << std::endl;
    }
}

void Assets::addAnimation(const std::string &a_strAnimationName,
                          const std::string &a_strTextureName,
                          size_t a_nFrameCount,
                          float a_nSpeed,
                          bool a_bLoop,
                          float a_fScale)
{
    m_mapAnimations[a_strAnimationName] = {
        Animation(a_strAnimationName, getTexture(a_strTextureName), a_nFrameCount, a_nSpeed, a_bLoop),
        a_fScale};
}

void Assets::addFont(const std::string &a_strFontName, const std::string &a_strPath)
{

    m_mapFonts[a_strFontName] = sf::Font();
    if (!m_mapFonts[a_strFontName].loadFromFile(a_strPath))
    {
        logError("Could not load font file: " + a_strPath);
        m_mapFonts.erase(a_strFontName);
    }
    else
    {
        std::cout << "Loaded Font:    " << a_strPath << std::endl;
    }
}

void Assets::addSound(const std::string &name, const std::string &path, float volume)
{
    m_mapSounds[name] = {sf::SoundBuffer(), volume};
    if (!m_mapSounds[name].buffer.loadFromFile(path))
    {
        logError("Could not load sound file: " + path);
        m_mapSounds.erase(name);
    }
    else
    {
        std::cout << "Loaded Sound:   " << path << std::endl;
    }
}

const SoundData &Assets::getSound(const std::string &name) const
{
    assert(m_mapSounds.find(name) != m_mapSounds.end());
    return m_mapSounds.at(name);
}

const sf::Texture &Assets::getTexture(const std::string &textureName) const
{

    assert(m_mapTextures.find(textureName) != m_mapTextures.end());
    return m_mapTextures.at(textureName);
}

const Animation &Assets::getAnimation(const std::string &a_strAnimationName) const
{

    assert(m_mapAnimations.find(a_strAnimationName) != m_mapAnimations.end());
    return m_mapAnimations.at(a_strAnimationName).animation;
}

float Assets::getAnimationScale(const std::string &a_strAnimationName) const
{
    assert(m_mapAnimations.find(a_strAnimationName) != m_mapAnimations.end());
    return m_mapAnimations.at(a_strAnimationName).scaleFactor;
}

const sf::Font &Assets::getFont(const std::string &a_strFontName) const
{

    assert(m_mapFonts.find(a_strFontName) != m_mapFonts.end());
    return m_mapFonts.at(a_strFontName);
}

const std::map<std::string, AnimationData> &Assets::getAllAnimations() const
{
    return m_mapAnimations;
}