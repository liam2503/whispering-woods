#include "Animation.h"
#include "ScenePlay.h"
#include <cmath>

Animation::Animation() {}

Animation::Animation(const std::string &a_strName, const sf::Texture &a_texture) : Animation(a_strName, a_texture, 1, 0) {}

Animation::Animation(const std::string &name, const sf::Texture &t,
                     size_t frameCount, float speed, bool a_bLoop)
    : m_name(name), m_sprite(t), m_frameCount(frameCount), m_frameCurrent(0), m_nSpeed(speed), m_bLoop(a_bLoop), m_timeBuffer(0.0f)
{
    m_vSize = VectorPP((float)t.getSize().x / frameCount, (float)t.getSize().y);
    m_sprite.setOrigin(m_vSize.x / 2.0f, m_vSize.y / 2.0f);
    m_sprite.setTextureRect(sf::IntRect(std::floor(m_frameCurrent) * m_vSize.x, 0,
                                        m_vSize.x, m_vSize.y));
}

void Animation::update(size_t gameFrame)
{
    m_timeBuffer += 1.0f;

    while (m_nSpeed > 0 && m_timeBuffer >= m_nSpeed)
    {
        m_timeBuffer -= m_nSpeed;

        // If we are at the last frame and NOT looping, don't advance
        if (!m_bLoop && m_frameCurrent >= m_frameCount - 1)
        {
            break;
        }

        m_frameCurrent += 1;
        if (m_frameCurrent >= m_frameCount)
        {
            if (m_bLoop)
            {
                m_frameCurrent = 0;
            }
            else
            {
                m_frameCurrent = m_frameCount - 1;
                break;
            }
        }
    }

    m_sprite.setTextureRect(sf::IntRect(
        static_cast<int>(std::floor(m_frameCurrent)) * static_cast<int>(m_vSize.x),
        0,
        static_cast<int>(m_vSize.x),
        static_cast<int>(m_vSize.y)));
}

void Animation::setLoop(bool bLoop)
{
    m_bLoop = bLoop;
}

const VectorPP &Animation::getSize() const
{
    return m_vSize;
}

const std::string &Animation::getName() const
{
    return m_name;
}

sf::Sprite &Animation::getSprite() const
{
    return const_cast<sf::Sprite &>(m_sprite);
}

bool Animation::hasEnded() const
{
    if (!m_bLoop && std::floor(m_frameCurrent) >= m_frameCount - 1)
        return true;

    return false;
}

size_t Animation::getCurrentFrame() const
{
    return m_frameCurrent;
}

sf::IntRect Animation::getFrame(size_t nFrame) const
{
    if (nFrame >= m_frameCount)
        nFrame = m_frameCount - 1;

    return sf::IntRect(
        static_cast<int>(nFrame) * static_cast<int>(m_vSize.x),
        0,
        static_cast<int>(m_vSize.x),
        static_cast<int>(m_vSize.y));
}