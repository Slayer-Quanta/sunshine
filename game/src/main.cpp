#include "rlImGui.h"
#include "Physics.h"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
using namespace std;

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    Vector2 targetPosition{ SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.75f };
    float targetRadius = 20.0f;
    float targetViewDistance = 1000.0f;

    vector<Circle> obstacles(5);
    for (Circle& obstacle : obstacles)
    {
        float maxRadius = 50.0f;
        obstacle.position = { Random(maxRadius, SCREEN_WIDTH - maxRadius), Random(maxRadius, SCREEN_HEIGHT - maxRadius) };
        obstacle.radius = Random(5.0f, maxRadius);
    }

    Probes probes(4);
    probes[0].angle =  30.0f;
    probes[1].angle =  15.0f;
    probes[2].angle = -15.0f;
    probes[3].angle = -30.0f;
    probes[0].length = probes[3].length = 100.0f;
    probes[1].length = probes[2].length = 250.0f;

    Rigidbody rb;
    rb.pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    rb.angularSpeed = 100.0f * DEG2RAD;
    float seekerRadius = 20.0f;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        rb.acc = Seek(GetMousePosition(), rb.pos, rb.vel, 500.0f);
        rb.acc = rb.acc + Avoid(rb, dt, obstacles, probes);
        Update(rb, dt);
        bool seekerVisible = IsVisible(rb.pos, targetViewDistance, targetPosition, targetRadius, obstacles);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawCircleV(targetPosition, seekerRadius, seekerVisible ? GREEN : RED);
        DrawLineV(targetPosition, targetPosition + Normalize(rb.pos - targetPosition) * targetViewDistance, seekerVisible ? GREEN : RED);

        DrawCircleV(rb.pos, seekerRadius, BLUE);
        DrawLineV(rb.pos, rb.pos + rb.dir * 250.0f, DARKBLUE);

        for (const Probe& probe : probes)
            DrawLineV(rb.pos, rb.pos + Rotate(Normalize(rb.vel), probe.angle * DEG2RAD) * probe.length, PURPLE);

        for (const Circle& obstacle : obstacles)
            DrawCircleV(obstacle.position, obstacle.radius, GRAY);

        rlImGuiBegin();
        ImGui::SliderFloat("View Distance", &targetViewDistance, 10.0f, 1250.0f);
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
