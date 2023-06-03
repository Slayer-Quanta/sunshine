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

void Avoid(const Vector2& obstacle, Rigidbody& rigidbody, float maxSpeed)
{
    Vector2 direction = Normalize(rigidbody.position - obstacle);
    Vector2 rightDirection = { -direction.y, direction.x };  // Perpendicular to the obstacle direction
    rigidbody.acceleration = rightDirection * maxSpeed - rigidbody.velocity;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SUNSHINE");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    Rigidbody seeker;
    seeker.position = { SCREEN_WIDTH * 0.1f, SCREEN_HEIGHT * 0.9f };
    seeker.velocity = {};
    seeker.acceleration = {};

    Vector2 target{ SCREEN_WIDTH * 0.9f, SCREEN_HEIGHT * 0.1f };
    float seekerSpeed = 500.0f;

    Vector2 obstaclePosition{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();

        UpdateRigidbody(seeker, dt);

        Vector2 direction = Normalize(target - seeker.position);

        // Check left collision
        Vector2 leftProbe = seeker.position + Vector2{ -direction.y, direction.x } *20.0f;
        bool leftCollision = CheckCollisionCircleRec(obstaclePosition, 20.0f, { leftProbe.x, leftProbe.y, 1.0f, 1.0f });
        if (leftCollision)
        {
            Vector2 rightDirection = { -direction.y, direction.x };  // Perpendicular to the left direction
            seeker.acceleration = rightDirection * seekerSpeed - seeker.velocity;
        }

        // Check right collision
        Vector2 rightProbe = seeker.position + Vector2{ direction.y, -direction.x } *20.0f;
        bool rightCollision = CheckCollisionCircleRec(obstaclePosition, 20.0f, { rightProbe.x, rightProbe.y, 1.0f, 1.0f });
        if (rightCollision)
        {
            Vector2 leftDirection = { direction.y, -direction.x };  // Perpendicular to the right direction
            seeker.acceleration = leftDirection * seekerSpeed - seeker.velocity;
        }

        // Additional left and right probes
        Vector2 additionalLeftProbe = seeker.position + Vector2{ -direction.y, direction.x } *40.0f;
        Vector2 additionalRightProbe = seeker.position + Vector2{ direction.y, -direction.x } *40.0f;

        // Check additional left collision
        bool additionalLeftCollision = CheckCollisionCircleRec(obstaclePosition, 20.0f, { additionalLeftProbe.x, additionalLeftProbe.y, 1.0f, 1.0f });
        if (additionalLeftCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed);
        }

        // Check additional right collision
        bool additionalRightCollision = CheckCollisionCircleRec(obstaclePosition, 20.0f, { additionalRightProbe.x, additionalRightProbe.y, 1.0f, 1.0f });
        if (additionalRightCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed);
        }

        Seek(seeker, target, seekerSpeed);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleV(seeker.position, 20.0f, RED);
        DrawCircleV(target, 20.0f, BLUE);

        DrawLineV(seeker.position, seeker.position + seeker.velocity, GREEN);
        DrawLineV(seeker.position, seeker.position + seeker.acceleration, PURPLE);

        DrawText("Velocity", seeker.position.x + seeker.velocity.x, seeker.position.y + seeker.velocity.y, 10, GREEN);
        DrawText("Acceleration", seeker.position.x + seeker.acceleration.x, seeker.position.y + seeker.acceleration.y, 10, PURPLE);

        // Draw additional left and right probes
        DrawCircleV(additionalLeftProbe, 5.0f, ORANGE);
        DrawCircleV(additionalRightProbe, 5.0f, ORANGE);

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
