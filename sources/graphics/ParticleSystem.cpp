#include "ParticleSystem.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>

void CreateParticleManager(ParticleManager &manager, Vector2 gravity, int screenWidth, int screenHeight) {
    manager.gravity = gravity;
    manager.renderTexture = LoadRenderTexture(screenWidth, screenHeight);
}

void DestroyParticleManager(ParticleManager &manager) {
    for (auto emitter : manager.emitters) {
        DestroyParticleEmitter(emitter);
    }
    manager.emitters.clear();
    UnloadRenderTexture(manager.renderTexture);
}

ParticleEmitter* CreateParticleEmitter(ParticleManager &manager, int maxParticles, Vector2 position, float duration) {
    ParticleEmitter* emitter = new ParticleEmitter();
    emitter->position = position;
    emitter->maxParticles = maxParticles;
    emitter->particles.resize(maxParticles);
    emitter->active = true;
    emitter->duration = duration;
    manager.emitters.push_back(emitter);
    return emitter;
}

void DestroyParticleEmitter(ParticleEmitter* emitter) {
    TraceLog(LOG_INFO, "Destroying particle emitter");
    delete emitter;
}

void EmitParticles(ParticleEmitter &emitter, int count, Vector2 direction, float spread, Color color,
                   float size, float lifetime, EmissionType emissionType) {
    for (int i = 0; i < count; i++) {
        for (auto &particle : emitter.particles) {
            if (!particle.active) {
                particle.active = true;
                particle.position = emitter.position;

                float speed = Vector2Length(direction);

                if (emissionType == EMIT_DIRECTIONAL) {
                    float angle = spread * (static_cast<float>(rand()) / RAND_MAX - 0.5f);
                    particle.velocity = {
                            direction.x * cosf(angle) - direction.y * sinf(angle),
                            direction.x * sinf(angle) + direction.y * cosf(angle)
                    };
                    particle.velocity = Vector2Scale(Vector2Normalize(particle.velocity), speed);
                }
                else if (emissionType == EMIT_RADIAL) {
                    float angle = 2.0f * PI * (static_cast<float>(rand()) / RAND_MAX);
                    particle.velocity = { cosf(angle) * speed, sinf(angle) * speed };
                }

                particle.color = color;
                particle.size = size;
                particle.lifeTime = particle.maxLifeTime = lifetime;
                break;
            }
        }
    }
}

void UpdateParticleManager(ParticleManager &manager, float deltaTime) {
    for (auto it = manager.emitters.begin(); it != manager.emitters.end();) {
        ParticleEmitter* emitter = *it;

        // Handle emission logic
        if (emitter->emitCallback && (emitter->duration > 0 || emitter->duration == -1)) {
            emitter->emitCallback(*emitter);
        }

        // Update particles and emitter state
        emitter->active = false;
        for (auto &particle : emitter->particles) {
            if (particle.active) {
                particle.velocity.x += manager.gravity.x * deltaTime;
                particle.velocity.y += manager.gravity.y * deltaTime;
                particle.position.x += particle.velocity.x * deltaTime;
                particle.position.y += particle.velocity.y * deltaTime;

                particle.lifeTime -= deltaTime;
                if (particle.lifeTime <= 0) particle.active = false;
                emitter->active = true;
            }
        }

        // Reduce duration if it's not infinite
        if (emitter->duration > 0) {
            emitter->duration -= deltaTime;
        }

        // Remove finished emitters
        if (!emitter->active && emitter->duration <= 0) {
            DestroyParticleEmitter(emitter);
            it = manager.emitters.erase(it);
        } else {
            ++it;
        }
    }
}


void PreRenderParticleManager(ParticleManager &manager, Camera2D& camera) {
    BeginTextureMode(manager.renderTexture);
    ClearBackground(BLANK);
    for (const auto &emitter : manager.emitters) {
        if(emitter->blendAdditive) BeginBlendMode(BLEND_ADDITIVE);
        for (const auto &particle : emitter->particles) {
            if (particle.active) {
                float lifeRatio = particle.lifeTime / particle.maxLifeTime;
                Color fadeColor = { particle.color.r, particle.color.g, particle.color.b,
                                    static_cast<unsigned char>(particle.color.a * lifeRatio) };
                Vector2 screenPos = GetWorldToScreen2D(particle.position, camera);
                DrawCircleV(screenPos, particle.size * lifeRatio, fadeColor);
            }
        }
        if(emitter->blendAdditive) EndBlendMode();
    }
    EndTextureMode();
}

void DrawParticleManager(ParticleManager &manager) {
    DrawTexturePro(
            manager.renderTexture.texture,
            { 0, 0, (float)manager.renderTexture.texture.width, -(float)manager.renderTexture.texture.height },
            { 0, 0, 480, 270 },
            { 0, 0 },
            0,
            WHITE
    );
}

void CreateBloodSplatter(ParticleManager &manager, Vector2 position, int count, float power) {
    ParticleEmitter* emitter = CreateParticleEmitter(manager, count * 2, position);
    emitter->emitCallback = [power](ParticleEmitter &emitter) {
        const Color DARK_BLOOD_RED = Color{139, 0, 0, 255};   // Dark blood red (#8B0000)
        const Color MEDIUM_BLOOD_RED = Color{178, 34, 34, 255};  // Medium blood red (#B22222)
        const Color LIGHT_BLOOD_RED = Color{204, 51, 51, 255};  // Light blood red (#FF6347)
        EmitParticles(emitter, emitter.maxParticles/2, { power, 0 }, 0.0f, DARK_BLOOD_RED, 3.0f, 0.75f, EMIT_RADIAL);
        EmitParticles(emitter, emitter.maxParticles/4, { power, 0 }, 0.0f, MEDIUM_BLOOD_RED, 2.0f, 0.75f, EMIT_RADIAL);
        EmitParticles(emitter, emitter.maxParticles/4, { power, 0 }, 0.0f, LIGHT_BLOOD_RED, 1.0f, 0.75f, EMIT_RADIAL);
    };
    emitter->duration = 0.1f; // Quick burst
    emitter->blendAdditive = true;
}

void CreateFireEffect(ParticleManager &manager, Vector2 position, Vector2 direction, float duration, float intensity) {
    ParticleEmitter* emitter = CreateParticleEmitter(manager, 100, position, duration);
    emitter->emitCallback = [intensity, direction](ParticleEmitter &emitter) {
        const Color FIRE_RED = Color{255, 69, 0, 255};   // Fire red (#FF4500)
        EmitParticles(emitter, intensity, direction, 0.3f, FIRE_RED, 2.0f, 1.0f);
    };
    emitter->blendAdditive = true;
}

void CreateSmokeEffect(ParticleManager &manager, Vector2 position, float duration, float intensity) {
    ParticleEmitter* emitter = CreateParticleEmitter(manager, 100, position, duration);
    emitter->emitCallback = [intensity](ParticleEmitter &emitter) {
        EmitParticles(emitter, intensity, { 0, -20 }, 0.5f, GRAY, 4.0f, 3.0f);
        EmitParticles(emitter, intensity, { 0, -20 }, 0.3f, GRAY, 6.0f, 2.0f);
    };
    emitter->blendAdditive = false;
}

void CreateExplosionEffect(ParticleManager &manager, Vector2 position, int count, float power, float duration) {
    ParticleEmitter* emitter = CreateParticleEmitter(manager, count * 3, position);
    emitter->emitCallback = [power](ParticleEmitter &emitter) {
        const Color FIRE_RED = Color{255, 69, 0, 255};   // Fire red (#FF4500)
        EmitParticles(emitter, emitter.maxParticles/2, { power, 0 }, 0.0f, FIRE_RED, 3.0f, 0.75f, EMIT_RADIAL);
        EmitParticles(emitter, emitter.maxParticles/4, { power, 0 }, 0.0f, FIRE_RED, 2.0f, 0.75f, EMIT_RADIAL);
        EmitParticles(emitter, emitter.maxParticles/4, { power, 0 }, 0.0f, FIRE_RED, 1.0f, 0.75f, EMIT_RADIAL);
    };
    emitter->duration = duration; // Quick burst
    emitter->blendAdditive = true;
}
