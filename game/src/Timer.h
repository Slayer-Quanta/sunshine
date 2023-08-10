#pragma once
struct Timer
{
    float duration = 0.0f;
    float elapsed = 0.0f;

    bool Expired() { return elapsed >= duration; }
    void Reset() { elapsed = 0.0f; }
    void Tick(float dt) { elapsed += dt; }

    float Percent()
    {
        float t = elapsed / duration;
        return t <= 1.0f ? t : 1.0f;
    }
};
