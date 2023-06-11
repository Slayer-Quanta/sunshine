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

float Vector2Distance(const Vector2& a, const Vector2& b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx * dx + dy * dy);
}

bool CheckCollisionCircleCircle(const Vector2& center1, float radius1, const Vector2& center2, float radius2)
{
    float distance = Vector2Distance(center1, center2);
    return distance < radius1 + radius2;
}

void Seek(Rigidbody& rigidbody, const Vector2& target, float maxSpeed, float avoidRadius)
{
    Vector2 direction = Normalize(target - rigidbody.position);
    Vector2 desiredVelocity = direction * maxSpeed;

    if (CheckCollisionCircleCircle(rigidbody.position, avoidRadius, target, 20.0f + avoidRadius))
    {
        Vector2 avoidDirection = Normalize(rigidbody.position - target);
        Vector2 avoidPosition = target + avoidDirection * (20.0f + avoidRadius);
        desiredVelocity = Normalize(avoidPosition - rigidbody.position) * maxSpeed;
    }

    rigidbody.acceleration = desiredVelocity - rigidbody.velocity;
}

void Avoid(const Vector2& obstacle, Rigidbody& rigidbody, float maxSpeed, float avoidRadius)
{
    Vector2 direction = Normalize(rigidbody.position - obstacle);
    Vector2 rightDirection = { -direction.y, direction.x };
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
    float avoidRadius = 40.0f;

    Vector2 obstaclePosition{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();

        UpdateRigidbody(seeker, dt);

        Vector2 direction = Normalize(target - seeker.position);

        // Check left collision
        Vector2 leftProbe = seeker.position + Vector2{ -direction.y, direction.x } *20.0f;
        bool leftCollision = CheckCollisionCircleCircle(obstaclePosition, 20.0f, leftProbe, 1.0f);
        if (leftCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed, avoidRadius);
        }

        Vector2 rightProbe = seeker.position + Vector2{ direction.y, -direction.x } *20.0f;
        bool rightCollision = CheckCollisionCircleCircle(obstaclePosition, 20.0f, rightProbe, 1.0f);
        if (rightCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed, avoidRadius);
        }
        Vector2 LeftProbe = seeker.position + Vector2{ -direction.y, direction.x } *40.0f;
        Vector2 RightProbe = seeker.position + Vector2{ direction.y, -direction.x } *40.0f;

        bool additionalLeftCollision = CheckCollisionCircleCircle(obstaclePosition, 20.0f, LeftProbe, 1.0f);
        if (additionalLeftCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed, avoidRadius);
        }
        bool additionalRightCollision = CheckCollisionCircleCircle(obstaclePosition, 20.0f, RightProbe, 1.0f);
        if (additionalRightCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed, avoidRadius);
        }

        Vector2 topProbe = seeker.position + Vector2{ direction.x, direction.y } *40.0f;
        Vector2 bottomProbe = seeker.position + Vector2{ -direction.x, -direction.y } *40.0f;

        bool topCollision = CheckCollisionCircleCircle(obstaclePosition, 20.0f, topProbe, 1.0f);
        if (topCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed, avoidRadius);
        }

        bool bottomCollision = CheckCollisionCircleCircle(obstaclePosition, 20.0f, bottomProbe, 1.0f);
        if (bottomCollision)
        {
            Avoid(obstaclePosition, seeker, seekerSpeed, avoidRadius);
        }

        Seek(seeker, target, seekerSpeed, avoidRadius);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircleV(seeker.position, 20.0f, RED);
        DrawCircleV(target, 20.0f, BLUE);

        DrawLineV(seeker.position, seeker.position + seeker.velocity, GREEN);
        DrawLineV(seeker.position, seeker.position + seeker.acceleration, PURPLE);

        DrawText("Velocity", seeker.position.x + seeker.velocity.x, seeker.position.y + seeker.velocity.y, 10, GREEN);
        DrawText("Acceleration", seeker.position.x + seeker.acceleration.x, seeker.position.y + seeker.acceleration.y, 10, PURPLE);

        DrawCircleV(LeftProbe, 5.0f, ORANGE);
        DrawCircleV(RightProbe, 5.0f, ORANGE);
        DrawCircleV(topProbe, 5.0f, ORANGE);
        DrawCircleV(bottomProbe, 5.0f, ORANGE);

        rlImGuiBegin();
        ImGui::SliderFloat("Seeker speed", &seekerSpeed, -100.0f, 100.0f);
        ImGui::SliderFloat("Avoid radius", &avoidRadius, 0.0f, 100.0f);
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
