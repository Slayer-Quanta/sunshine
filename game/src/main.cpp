#include "rlImGui.h"
#include "Game.h"
#include <iostream>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
using namespace std;

void OnGui(Circles& obstacles);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    // Create a world such that there will always be at least 1 cover point and 1 point of visibility!
    World world;
    Load(world.obstacles);
    world.waypoints = {
        { SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.25f },
        { SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.25f },
        { SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.75f },
        { SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.75f },
    };

    Player player;
    Enemy enemy(world);
    enemy.pos = { SCREEN_WIDTH * 0.1f, SCREEN_HEIGHT * 0.1f };
    player.pos = { SCREEN_WIDTH * 0.9f, SCREEN_HEIGHT * 0.9f };
    enemy.angularSpeed = 100.0f * DEG2RAD;
    player.angularSpeed = 250.0f * DEG2RAD;

    // *Build decision tree for lab 8 here*

    bool showGui = false;
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_GRAVE))
            showGui = !showGui;

        const float dt = GetFrameTime();
        player.Update(dt, world);
        enemy.Update(dt);

        for (Bullet& bullet : world.bullets)
        {
            Update(bullet, dt);
        }

        world.bullets.erase(
            remove_if(world.bullets.begin(), world.bullets.end(),
                [&player, &enemy, &world](const Bullet& bullet)
                {
                    // A4 TODO -- test enemy bullets against player (damage and remove if colliding)
                    // A4 TODO -- test player bullets against enemy (damage and remove if colliding)
                    // A4 TODO -- test bullet against all obstacles (remove if colliding)
                    // A4 TODO -- test bullet against screen rectangle (remove if not colliding)
                    return false;   // <-- removes nothing (returning true removes the current bullet)
                }),
            world.bullets.end());

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawCircleV(player.pos, player.Radius(), BLUE);
        DrawLineV(player.pos, player.pos + player.dir * 100.0f, BLACK);
        DrawCircleV(enemy.pos, enemy.Radius(), RED);
        DrawLineV(enemy.pos, enemy.pos + enemy.dir * 100.0f, BLACK);

        for (const Circle& obstacle : world.obstacles)
            DrawCircleV(obstacle.position, obstacle.radius, GRAY);

        for (const Vector2& point : world.waypoints)
            DrawCircleV(point, 20.0f, DARKBLUE);

        for (const Bullet& bullet : world.bullets)
            DrawCircleV(bullet.pos, bullet.Radius(), RED);

        rlImGuiBegin();
        if (showGui)
            OnGui(world.obstacles);
        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}

void OnGui(Circles& obstacles)
{
    if (ImGui::Button("Save Obstacles"))
    {
        Save(obstacles);
    }

    if (ImGui::Button("Load Obstacles"))
    {
        obstacles.clear();
        Load(obstacles);
    }

    if (ImGui::Button("Add Obstacle"))
    {
        Circle obstacle;
        obstacle.position = { 0.0f, 0.0f };
        obstacle.radius = 100.0f;
        obstacles.push_back(obstacle);
    }

    if (ImGui::Button("Remove Obstacle"))
    {
        obstacles.pop_back();
    }

    char obstacleLabel[64];
    for (size_t i = 0; i < obstacles.size(); i++)
    {
        sprintf(obstacleLabel, "Obstacle %zu", i);
        ImGui::SliderFloat3(obstacleLabel, (float*)&obstacles[i], 0.0f, SCREEN_WIDTH);
    }
}