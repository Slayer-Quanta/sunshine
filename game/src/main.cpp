#include "raylib.h"
#include "rlImGui.h"
#include "Math.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

struct Rigidbody
{
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
};

void UpdateRigidbody(Rigidbody& rigidbody, float dt)
{
    rigidbody.position = rigidbody.position + rigidbody.velocity * dt + rigidbody.acceleration * 0.5f * dt * dt;
    rigidbody.velocity = rigidbody.velocity + rigidbody.acceleration * dt;

    // Apply screen wrapping
    if (rigidbody.position.y > SCREEN_HEIGHT) rigidbody.position.y = 0.0f;
    if (rigidbody.position.y < 0.0f) rigidbody.position.y = SCREEN_HEIGHT;
    if (rigidbody.position.x > SCREEN_WIDTH) rigidbody.position.x = 0.0f;
    if (rigidbody.position.x < 0.0f) rigidbody.position.x = SCREEN_WIDTH;
}

void Seek(Rigidbody& rigidbody, const Vector2& target, float maxSpeed)
{
    Vector2 direction = Normalize(target - rigidbody.position);
    rigidbody.acceleration = direction * maxSpeed - rigidbody.velocity;
}

void Flee(Rigidbody& rigidbody, const Vector2& target, float maxSpeed)
{
    Vector2 direction = Normalize(rigidbody.position - target);
    rigidbody.acceleration = direction * maxSpeed - rigidbody.velocity;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    Rigidbody seeker;
    seeker.position = { SCREEN_WIDTH * 0.1f, SCREEN_HEIGHT * 0.9f };
    seeker.velocity = {};
    seeker.acceleration = {};

    Vector2 target{ SCREEN_WIDTH * 0.9f, SCREEN_HEIGHT * 0.1f };
    float seekerSpeed = 500.0f;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();

        UpdateRigidbody(seeker, dt);

        Seek(seeker, target, seekerSpeed);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleV(seeker.position, 20.0f, RED);
        DrawCircleV(target, 20.0f, BLUE);

        // Draw line segments representing velocity and acceleration
        DrawLineV(seeker.position, seeker.position + seeker.velocity, GREEN);
        DrawLineV(seeker.position, seeker.position + seeker.acceleration, PURPLE);

        // Draw text labels for velocity and acceleration
        DrawText("Velocity", seeker.position.x + seeker.velocity.x, seeker.position.y + seeker.velocity.y, 10, GREEN);
        DrawText("Acceleration", seeker.position.x + seeker.acceleration.x, seeker.position.y + seeker.acceleration.y, 10, PURPLE);

        rlImGuiBegin();
        ImGui::SliderFloat("Seeker speed", &seekerSpeed, -100.0f, 100.0f);
        ImGui::SliderFloat2("Target", &target.x, 0.0f, SCREEN_WIDTH);
        ImGui::SliderFloat2("Position", &seeker.position.x, 0.0f, SCREEN_WIDTH);
        ImGui::SliderFloat2("Velocity", &seeker.velocity.x, -100.0f, 100.0f);
        ImGui::SliderFloat2("Acceleration", &seeker.acceleration.x, -100.0f, 100.0f);
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
