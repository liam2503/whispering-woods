#include "ParticleSystem.h"
#include "VectorPP.h"
#include <math.h>
#include <iostream>

void ParticleSystem::resetAllParticles()
{
	for (size_t i = 0; i < m_nCount; i++)
	{
		resetParticle(i, true);
	}
}

void ParticleSystem::moveAllParticles(const sf::Vector2f &delta)
{
	for (size_t i = 0; i < m_vecParticles.size(); i++)
	{
		m_vertices[4 * i + 0].position += delta;
		m_vertices[4 * i + 1].position += delta;
		m_vertices[4 * i + 2].position += delta;
		m_vertices[4 * i + 3].position += delta;
	}
}

void ParticleSystem::resetParticle(size_t a_nIndex, bool a_bFirstReset)
{

	float x = m_vEmitter.x;
	float y = m_vEmitter.y;

	m_vertices[4 * a_nIndex + 0].position = sf::Vector2f(x, y);
	m_vertices[4 * a_nIndex + 1].position = sf::Vector2f(x + m_fSize, y);
	m_vertices[4 * a_nIndex + 2].position = sf::Vector2f(x + m_fSize, y + m_fSize);
	m_vertices[4 * a_nIndex + 3].position = sf::Vector2f(x, y + m_fSize);

	sf::Uint8 r = 235 + rand() % 20;
	sf::Uint8 g = 230 + rand() % 25;
	sf::Uint8 b = 200 + rand() % 40;
	sf::Uint8 a = rand() % 255;

	sf::Color color(r, g, b, a);

	if (a_bFirstReset)
	{
		color.a = 0;
	}

	m_vertices[4 * a_nIndex + 0].color = color;
	m_vertices[4 * a_nIndex + 1].color = color;
	m_vertices[4 * a_nIndex + 2].color = color;
	m_vertices[4 * a_nIndex + 3].color = color;

	float angle = ((float)rand() / RAND_MAX) * 2 * 3.1415926f;

	float speed = ((float)rand() / RAND_MAX) * 4 - 2;

	float randX = cos(angle) * speed;
	float randY = sin(angle) * speed;

	m_vecParticles[a_nIndex].velocity = sf::Vector2f(randX, randY);

	m_vecParticles[a_nIndex].lifetime = 2.0f + (rand() % 5);
}

ParticleSystem::ParticleSystem() {}

void ParticleSystem::init(size_t particleCount, float particleSize, VectorPP position)
{
	m_nCount = particleCount;
	m_fSize = particleSize;

	m_vEmitter.x = position.x;
	m_vEmitter.y = position.y;

	m_vecParticles = std::vector<Particle>(m_nCount);
	m_vertices = sf::VertexArray(sf::Quads, m_nCount * 4);

	resetAllParticles();
}

void ParticleSystem::update()
{

	for (size_t i = 0; i < m_vecParticles.size(); i++)
	{

		if (m_vecParticles[i].lifetime <= 0)
		{
			resetParticle(i);
		}

		m_vertices[4 * i + 0].position += m_vecParticles[i].velocity;
		m_vertices[4 * i + 1].position += m_vecParticles[i].velocity;
		m_vertices[4 * i + 2].position += m_vecParticles[i].velocity;
		m_vertices[4 * i + 3].position += m_vecParticles[i].velocity;

		m_vecParticles[i].lifetime--;
	}
}

void ParticleSystem::setEmitter(VectorPP targetPos)
{

	sf::Vector2f target(targetPos.x, targetPos.y);

	sf::Vector2f current(m_vEmitter.x, m_vEmitter.y);

	float followSpeed = 0.07f;

	sf::Vector2f newPos = current + (target - current) * followSpeed;

	sf::Vector2f delta = newPos - current;

	m_vEmitter.x = newPos.x;
	m_vEmitter.y = newPos.y;

	moveAllParticles(delta);
}

void ParticleSystem::draw(sf::RenderWindow &a_rWindow) const
{
	a_rWindow.draw(m_vertices);
}