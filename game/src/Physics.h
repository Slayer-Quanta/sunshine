#pragma once
#include "Collision.h"

struct Rigidbody
{
    Vector2 pos{};
    Vector2 vel{};
    Vector2 acc{};
    float health = 100.0f;
    Vector2 dir{ 1.0f, 0.0f };  // right
    float angularSpeed = 0.0f;  // radians
};

inline void Update(Rigidbody& rb, float dt)
{
    rb.vel = rb.vel + rb.acc * dt;
    rb.pos = rb.pos + rb.vel * dt + rb.acc * dt * dt * 0.5f;
    rb.dir = RotateTowards(rb.dir, Normalize(rb.vel), rb.angularSpeed * dt);
}

inline Vector2 Seek(Vector2 target, Vector2 seekerPosition, Vector2 seekerVelocity, float speed)
{
    return Normalize(target - seekerPosition) * speed - seekerVelocity;
}

inline Vector2 Flee(Vector2 target, Vector2 seekerPosition, Vector2 seekerVelocity, float speed)
{
    return Normalize(seekerPosition - target) * speed - seekerVelocity;
}

inline Vector2 Avoid(const Rigidbody& rb, float dt, const Circles& obstacles, const Probes& probes)
{
    // Steer away from the obstacle (right or left depending on sign of probe angle)
    auto avoid = [&rb](float probeAngle, float dt) -> Vector2
    {
        Vector2 linearDirection = Normalize(rb.vel);
        float linearSpeed = Length(rb.vel);
        float avoidSign = probeAngle >= 0.0f ? -1.0f : 1.0f;
        Vector2 vf = Rotate(linearDirection, rb.angularSpeed * avoidSign * dt) * linearSpeed;
        return (vf - rb.vel) / dt;
    };

    auto probeEnd = [&rb](const Probe& probe) -> Vector2
    {
        return rb.pos + Rotate(Normalize(rb.vel), probe.angle * DEG2RAD) * probe.length;
    };

    Points probeEnds(probes.size());
    for (size_t i = 0; i < probes.size(); i++)
        probeEnds[i] = probeEnd(probes[i]);

    // Avoid first detected obstacle (otherwise, we risk equilibrium of conflicting probes each detect obstacles)
    for (size_t i = 0; i < probes.size(); i++)
    {
        for (const Circle& obstacle : obstacles)
        {
            if (LineCircle(rb.pos, probeEnds[i], obstacle.position, obstacle.radius))
                return avoid(probes[i].angle, dt);
        }
    }
    return {};
}