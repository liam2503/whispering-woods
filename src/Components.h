#pragma once

#include "Animation.h"
#include "Assets.h"
#include "VectorPP.h"
#include <cmath>
#include <SFML/Graphics.hpp>

// --- Base Component ---
class Components
{
public:
    bool has = false;
};

// --- Transform & Physics ---
class CompTransform : public Components
{
public:
    VectorPP vPosition = {0.0, 0.0};
    VectorPP vPrevPos = {0.0, 0.0};
    VectorPP vScale = {1.0, 1.0};
    VectorPP vVelocity = {0.0, 0.0};
    float fAngle = 0;

    CompTransform() {}
    CompTransform(const VectorPP &a_vPosition)
        : vPosition(a_vPosition) {}
    CompTransform(const VectorPP &a_vPosition, const VectorPP &a_vVelocity, const VectorPP &a_vScale, float a_fAngle)
        : vPosition(a_vPosition), vPrevPos(a_vPosition), vVelocity(a_vVelocity), vScale(a_vScale), fAngle(a_fAngle) {}
};

class CompBoundingBox : public Components
{
public:
    VectorPP vSize = {0.0, 0.0};
    VectorPP vHalfSize;
    bool isActive = true;
    bool dragging = false;

    CompBoundingBox() {}
    CompBoundingBox(const VectorPP &a_vSize)
        : vSize(a_vSize), vHalfSize(a_vSize.x / 2, a_vSize.y / 2) {}

    void enable() { isActive = true; }
    void disable() { isActive = false; }
    void toggle() { isActive = !isActive; }
};

class CompGravity : public Components
{
public:
    float fGravity = 0;
    CompGravity() {}
    CompGravity(float a_fGravity) : fGravity(a_fGravity) {}
};

// --- Graphics & Animation ---
class CompAnimation : public Components
{
public:
    Animation animation;
    bool bRepeat;
    CompAnimation() {}
    CompAnimation(const Animation &a_animation, bool a_bRepeat)
        : animation(a_animation), bRepeat(a_bRepeat) {}
};

class CompLayer : public Components
{
public:
    int layer = 3;
    CompLayer() {}
    CompLayer(int l) : layer(l) {}
};

class CompFlipScale : public Components
{
public:
    int totalFrames = 20;
    int framesRemaining = totalFrames;
    int targetSign = 0;

    CompFlipScale() {}
    CompFlipScale(int a_frames) : totalFrames(a_frames), framesRemaining(a_frames), targetSign(0) {}
};

// --- Logic & State ---
enum EntityState
{
    STANDING = 0,
    RUNNING,
    JUMPING,
    SHOOTING,
    BURROWING
};

class CompState : public Components
{
public:
    EntityState state = EntityState::JUMPING;
    bool hasDoubleJumped = false;
    int burrowTimer = 0;
    int burrowPhase = 0;

    CompState() {}
    CompState(EntityState a_state) : state(a_state) {}
};

class CompLifeSpan : public Components
{
public:
    int nDuration = 0;
    int nFrameCreated = 0;

    CompLifeSpan() {}
    CompLifeSpan(int a_nDuration, int a_nFrameCreated) : nDuration(a_nDuration), nFrameCreated(a_nFrameCreated) {}
};

class CompInput : public Components
{
public:
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool canShoot = true;
    bool canJump = true;
    CompInput() {}
};

class CompChunk : public Components
{
public:
    int x = 0;
    int y = 0;
    CompChunk() {}
    CompChunk(int gridX, int gridY) : x(gridX), y(gridY) {}
};

// --- Combat & Stats ---
class CompHealth : public Components
{
public:
    int health = 100;
    int currentHealth = health;

    CompHealth() {}
    CompHealth(int a_health) : health(a_health) {}
};

class CompMana : public Components
{
public:
    int mana = 100;
    int currentMana = mana;

    CompMana() {}
    CompMana(int a_mana) : mana(a_mana) {}
};

class CompDamage : public Components
{
public:
    int damage = 10;
    CompDamage() {}
    CompDamage(int a_damage) : damage(a_damage) {}
};

class CompInvincible : public Components
{
public:
    bool isInvincible = false;
    int framesRemaining = 60;

    CompInvincible() {}
    CompInvincible(int a_frames) : isInvincible(true), framesRemaining(a_frames) {}
};

enum EntityStatus
{
    NORMAL,
    FROZEN,
    FIRE,
    POISIONED
};

class CompStatus : public Components
{
public:
    EntityStatus status = EntityStatus::NORMAL;
    int framesRemaining = 60;

    CompStatus() {}
    CompStatus(EntityStatus a_status) : status(a_status) {}
    CompStatus(EntityStatus a_status, int a_frames) : status(a_status), framesRemaining(a_frames) {}
};

class CompChangeDirectionTimer : public Components
{
public:
    int totalFrames = 0;
    int framesRemaining = 0;

    CompChangeDirectionTimer() {}
    CompChangeDirectionTimer(int a_frames) : totalFrames(a_frames) {}
};

// --- Special Mechanics ---
class CompBoomerang : public Components
{
public:
    float totalDistance = 0.0f;
    bool returning = false;
    float angle = 0.0f;
    VectorPP initialVel;
};

class CompLine : public Components
{
public:
    sf::Vertex verts[2];
    float slope;
    float angleDeg = 0.0f;

    CompLine() {}
    CompLine(const VectorPP &a, const VectorPP &b)
    {
        verts[0] = sf::Vertex(sf::Vector2f(a.x, a.y), sf::Color::Green);
        verts[1] = sf::Vertex(sf::Vector2f(b.x, b.y), sf::Color::Green);
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        slope = dy / (dx == 0 ? 0.0001f : dx);
        float angleRad = std::atan2(dy, dx);
        angleDeg = angleRad * 180.0f / 3.14159265f;
    }
};

struct CompCutscene
{
    std::string filename;
    bool hasRun = false;
    bool has = true;

    CompCutscene() = default;
    CompCutscene(const std::string &file) : filename(file), hasRun(false) {}
};

struct CompFallingTile : public Components
{
    VectorPP originalPos;
    bool triggered = false;
    bool isFading = false;
    float opacity = 255.0f;
    int shakeTimer = 0;
    int fallTimer = 0;
    int maxShake = 60;
    int maxFall = 120;

    CompFallingTile() {}
    CompFallingTile(VectorPP origin, int shake, int fall)
        : originalPos(origin), shakeTimer(shake), maxShake(shake), fallTimer(fall), maxFall(fall) {}
    CompFallingTile(VectorPP origin) : originalPos(origin)
    {
        shakeTimer = maxShake;
        fallTimer = maxFall;
    }
};