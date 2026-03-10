#ifndef PARTICLE_SYSTEM
#define PARTICLE_SYSTEM

#include <SFML/Graphics.hpp>
#include <vector>
#include "VectorPP.h"

class ParticleSystem
{
private:
    struct Particle
    {
        sf::Vector2f velocity;
        int lifetime = 0;
        float speed = 0;
    };

    std::vector<Particle> m_vecParticles;
    sf::VertexArray m_vertices;
    sf::Vector2u m_vWindowSize;
    sf::Vector2f m_vEmitter;
    float m_fSize = 8.f;
    size_t m_nCount = 0;

    void resetAllParticles();
    void resetParticle(size_t a_nIndex, bool a_bFirstReset = false);

public:
    ParticleSystem();

    void init(size_t particleCount, float particleSize, VectorPP position);
    void update();
    void draw(sf::RenderWindow &a_rWindow) const;
    void reset() { resetAllParticles(); }

    void setEmitter(VectorPP position);
    VectorPP getEmitter() const { return VectorPP(m_vEmitter.x, m_vEmitter.y); }
    
    void moveAllParticles(const sf::Vector2f &delta);
};

#endif