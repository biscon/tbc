//
// Created by bison on 22-01-25.
//

#ifndef SANDBOX_PARTICLESYSTEM_H
#define SANDBOX_PARTICLESYSTEM_H

#include "raylib.h"
#include <vector>

#include "raylib.h"
#include <vector>
#include <functional>

enum EmissionType {
    EMIT_DIRECTIONAL, // Emit with given direction and spread
    EMIT_RADIAL       // Emit in a circular pattern
};

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float lifeTime;
    float maxLifeTime;
    bool active;
};

struct ParticleEmitter {
    Vector2 position;
    std::vector<Particle> particles;
    int maxParticles;
    float duration; // Duration of the emitter, <= 0 means infinite
    bool active;
    std::function<void(ParticleEmitter&)> emitCallback; // Custom emission logic
    bool blendAdditive;
};

struct ParticleManager {
    Vector2 gravity;
    std::vector<ParticleEmitter*> emitters;
    RenderTexture2D renderTexture;
};

void CreateParticleManager(ParticleManager &manager, Vector2 gravity, int screenWidth, int screenHeight);
void DestroyParticleManager(ParticleManager &manager);

ParticleEmitter* CreateParticleEmitter(ParticleManager &manager, int maxParticles, Vector2 position, float duration = 0);
void DestroyParticleEmitter(ParticleEmitter* emitter);

void EmitParticles(ParticleEmitter &emitter, int count, Vector2 velocity, float spread, Color color, float size, float lifeTime, EmissionType emissionType = EMIT_DIRECTIONAL);

void UpdateParticleManager(ParticleManager &manager, float deltaTime);
void PreRenderParticleManager(ParticleManager &manager);
void DrawParticleManager(ParticleManager &manager);

// Effect-specific functions
void CreateBloodSplatter(ParticleManager &manager, Vector2 position, int count, float power);
void CreateFireEffect(ParticleManager &manager, Vector2 position, Vector2 direction, float duration, float intensity);
void CreateSmokeEffect(ParticleManager &manager, Vector2 position, float duration, float intensity);
void CreateExplosionEffect(ParticleManager &manager, Vector2 position, int count, float power, float duration);

#endif //SANDBOX_PARTICLESYSTEM_H
