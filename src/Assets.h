#pragma once

#include "Animation.h"
#include "Common.h"
#include <map>
#include <string>

// --- Helper Structs ---
struct AnimationData
{
    Animation animation;
    float scaleFactor = 1.0f;
};

struct SoundData
{
    sf::SoundBuffer buffer;
    float volume = 100.0f;
};

// --- Assets Manager ---
class Assets
{
private:
    // Resource Storage
    std::map<std::string, sf::Texture> m_mapTextures;
    std::map<std::string, AnimationData> m_mapAnimations;
    std::map<std::string, sf::Font> m_mapFonts;
    std::map<std::string, SoundData> m_mapSounds;

    // Internal Load Helpers
    void addTexture(const std::string &a_strTextureTypes, const std::string &a_strPath, bool a_bSmooth = false);
    void addAnimation(const std::string &a_strAnimationName, const std::string &a_strTextureName, size_t a_nFrameCount, float a_nSpeed, bool a_bLoop, float a_fScale);
    void addFont(const std::string &a_strFontName, const std::string &a_strPath);
    void addSound(const std::string &name, const std::string &path, float volume);
    
    void logError(const std::string &message) const;

public:
    Assets();

    // Main Loading Interface
    void loadFromFile(const std::string &a_strPath);

    // --- Getters ---
    const sf::Texture &getTexture(const std::string &a_strTextureTypes) const;
    const Animation &getAnimation(const std::string &a_strAnimationTypes) const;
    float getAnimationScale(const std::string &a_strAnimationName) const;
    const std::map<std::string, AnimationData> &getAllAnimations() const;
    
    const sf::Font &getFont(const std::string &a_strFontName) const;
    const SoundData &getSound(const std::string &name) const;
};