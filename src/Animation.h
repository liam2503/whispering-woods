#pragma once

#include "VectorPP.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <string>

typedef size_t Frame;

class Animation
{
private:
    // --- Properties ---
    std::string m_name = "none";
    sf::Sprite m_sprite;
    VectorPP m_vSize = {1, 1};
    bool m_bLoop = true;

    // --- Frame Data ---
    Frame m_frameCount = 1;
    Frame m_frameCurrent = 0;
    float m_nSpeed = 0.0f;
    float m_timeBuffer = 0.0f;

public:
    // --- Constructors ---
    Animation();
    Animation(const std::string &a_strName, const sf::Texture &a_texture);
    Animation(const std::string &name, const sf::Texture &t, size_t frameCount, float speed, bool a_bLoop = true);

    // --- Core Logic ---
    void update(size_t gameFrame);
    bool hasEnded() const;
    
    // --- Setters/Getters ---
    void setLoop(bool bLoop);
    const std::string &getName() const;
    const VectorPP &getSize() const;
    sf::Sprite &getSprite() const;
    size_t getCurrentFrame() const;
    sf::IntRect getFrame(size_t nFrame) const;
};