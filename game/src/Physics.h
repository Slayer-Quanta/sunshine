#pragma once
#include "Math.h"

struct Rigidbody
{
    Vector2 velocity{ 0.0f, 0.0f };
    Vector2 acceleration{ 0.0f, 0.0f };
};

// v2 = v1 + a(t)
// p2 = p1 + v2(t) + 0.5a(t^2)
Vector2 Integrate(const Vector2& pos, Rigidbody& rb, float dt)
{
    rb.velocity = rb.velocity + rb.acceleration * dt;
    return pos + rb.velocity * dt + rb.acceleration * dt * dt * 0.5f;
}

// vf^2 = vi^2 + 2a(d)
// 0^2 = vi^2 + 2a(d)
// -vi^2 / 2d = a
Vector2 Decelerate(
    const Vector2& targetPosition,
    const Vector2& seekerPosition,
    const Vector2& seekerVelocity)
{
    float d = Length(targetPosition - seekerPosition);
    float a = Dot(seekerVelocity, seekerVelocity) / (d * 2.0f);

    return Negate(Normalize(seekerVelocity)) * a;
}

// Accelerate towards target
Vector2 Seek(
    const Vector2& targetPosition,
    const Vector2& seekerPosition,
    const Vector2& seekerVelocity, float maxSpeed)
{
    Vector2 desiredVelocity = Normalize(targetPosition - seekerPosition) * maxSpeed;
    return desiredVelocity - seekerVelocity;
}