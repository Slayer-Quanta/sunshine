#include "rlImGui.h"
#include "Game.h"
#include <iostream>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
using namespace std;

void OnGui(Circles& obstacles);
float playerHealth = 100.0f;
float enemyHealth = 100.0f;
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

    SeekAction seekPlayer(enemy, player, world, 500.0f);
    ShootAction shootPlayer(enemy, player, world, 0.25f);

    CoverAction takeCover(enemy, player, world, 500.0f); 
  
    // Build decision trees
    ProximityNode defensiveProximity(enemy, player, world, 200.0f);
    SeekAction offensiveAction(enemy, player, world, 500.0f); 

    // Set trees
    enemy.SetTrees(&offensiveAction, nullptr); 

    // Example tree:
    Color proximityColor = SKYBLUE;
    proximityColor.a = 128;

    // Create nodes
    ProximityNode isNear(enemy, player, world, 200.0f);
    SeekAction seek(enemy, player, world, 500.0f);
    ShootAction shoot(enemy, player, world, 0.25f);
    shoot.next = &seek;

    // Build tree
    isNear.no = &seek;
    isNear.yes = &shoot;
    enemy.SetTree(&isNear);

    bool showGui = false;
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_L))
            showGui = !showGui;

        const float dt = GetFrameTime();
        player.Update(dt, world);
        enemy.Update(dt);

        for (Bullet& bullet : world.bullets)
        {
            Update(bullet, dt);

            // Test enemy bullets against player
            if (bullet.GetType() == Bullet::ENEMY && CheckCollisionCircleRec(bullet.pos, bullet.Radius(), { player.pos.x - player.Radius(), player.pos.y - player.Radius(), player.Radius() * 2, player.Radius() * 2 }))
            {
                playerHealth -= 10.0f; // Modify this line based on your desired damage
                bullet.isActive = false;
            }

            // Test player bullets against enemy
    
            if (bullet.GetType() == Bullet::PLAYER && CheckCollisionCircleRec(bullet.pos, bullet.Radius(), { enemy.pos.x - enemy.Radius(), enemy.pos.y - enemy.Radius(), enemy.Radius() * 2, enemy.Radius() * 2 }))
            {
                enemyHealth -= 10.0f; // Modify this line based on your desired damage
                bullet.isActive = false;
            }
            // Test bullet against all obstacles
            for (const Circle& obstacle : world.obstacles)
            {
                if (CheckCollisionCircles(bullet.pos, bullet.Radius(), obstacle.position, obstacle.radius))
                {
                    bullet.isActive = false;
                    break; // No need to continue checking obstacles for this bullet
                }
            }

            // Test bullet against screen rectangle
            if (bullet.pos.x < 0 || bullet.pos.x > SCREEN_WIDTH || bullet.pos.y < 0 || bullet.pos.y > SCREEN_HEIGHT)
            {
                bullet.isActive = false;
            }
        }

        // Remove inactive bullets
        world.bullets.erase(remove_if(world.bullets.begin(), world.bullets.end(), [](const Bullet& bullet) {
            return !bullet.isActive;
            }), world.bullets.end());
        // Wrapping logic for player
        if (player.pos.x < 0)
            player.pos.x = SCREEN_WIDTH;
        else if (player.pos.x > SCREEN_WIDTH)
            player.pos.x = 0;

        if (player.pos.y < 0)
            player.pos.y = SCREEN_HEIGHT;
        else if (player.pos.y > SCREEN_HEIGHT)
            player.pos.y = 0;

        // Wrapping logic for enemy
        if (enemy.pos.x < 0)
            enemy.pos.x = SCREEN_WIDTH;
        else if (enemy.pos.x > SCREEN_WIDTH)
            enemy.pos.x = 0;

        if (enemy.pos.y < 0)
            enemy.pos.y = SCREEN_HEIGHT;
        else if (enemy.pos.y > SCREEN_HEIGHT)
            enemy.pos.y = 0;

        if (playerHealth <= 0.0f)
        {
            player.pos = { -100.0f, -100.0f }; // Move player off-screen
        }

        if (enemyHealth <= 0.0f)
        {
            enemy.pos = { -200.0f, -200.0f }; // Move enemy off-screen
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawRectangle(50, 20, playerHealth * 2, 10, GREEN);
        DrawRectangle(SCREEN_WIDTH - 250, 20, enemyHealth * 2, 10, RED);
        DrawCircleV(enemy.pos, isNear.Proximity(), proximityColor);
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

    ImGui::SameLine();
    if (ImGui::Button("Remove Obstacle"))
    {
        if (!obstacles.empty())
        {
            obstacles.pop_back();
        }
    }

    for (size_t i = 0; i < obstacles.size(); i++)
    {
        char obstacleLabel[64];
        sprintf(obstacleLabel, "Obstacle %zu", i);

        if (ImGui::TreeNode(obstacleLabel))
        {
            ImGui::SliderFloat2("Position", (float*)&obstacles[i].position, 0.0f, SCREEN_WIDTH);
            ImGui::SliderFloat("Radius", &obstacles[i].radius, 0.0f, 250.0f);
            if (ImGui::Button("Remove"))
            {
                obstacles.erase(obstacles.begin() + i);
                i--;
            }
            ImGui::TreePop();
        }
    }
}