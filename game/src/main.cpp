#include "raylib.h"
#include "rlImGui.h"
#include "Math.h"
#include <array>
#include <vector>
#include <cmath> // Include this header for additional math functions
#include <fstream>
#include <iostream>

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TILE_COUNT = 20;
constexpr int TILE_WIDTH = SCREEN_WIDTH / TILE_COUNT;
constexpr int TILE_HEIGHT = SCREEN_HEIGHT / TILE_COUNT;
using namespace std;

// The circle's position is a good enough approximation of the point-of-intersection (poi) to use in our distance-checks
bool LineCircle(Vector2 lineStart, Vector2 lineEnd, Vector2 circlePosition, float circleRadius)
{
    Vector2 nearest = ProjectPointLine(lineStart, lineEnd, circlePosition);
    return DistanceSqr(nearest, circlePosition) <= circleRadius * circleRadius;
}

// Function to check if two circles are colliding
bool CircleCollision(Vector2 pos1, float radius1, Vector2 pos2, float radius2)
{
    return Distance(pos1, pos2) < (radius1 + radius2);
}

struct Circle
{
    Vector2 position;
    float radius;
};

// Struct to represent bullets
struct Bullet
{
    Rectangle rect;
    Vector2 direction;
    float speed;
};
// Function to load waypoints from a file

// Function to wrap a position around the screen boundaries
void WrapPosition(Vector2& position)
{
    if (position.x < 0)
        position.x = SCREEN_WIDTH;
    else if (position.x > SCREEN_WIDTH)
        position.x = 0;

    if (position.y < 0)
        position.y = SCREEN_HEIGHT;
    else if (position.y > SCREEN_HEIGHT)
        position.y = 0;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    Vector2 playerPosition{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    Vector2 playerDirection = Direction(0.0f);
    float playerSpeed = 250.0f;
    float playerRotationSpeed = 250.0f * DEG2RAD;
    float playerLength = 250.0f;
    float playerRadius = 20.0f;
    float viewDistance = 1500.0f;

    Vector2 targetPosition{ SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.75f };
    float targetRadius = 20.0f;

    // Consider hard-coding obstacle positions to test line-of-sight math since random-generator is seeded by default.
    array<Circle, 5> obstacles{};
    for (Circle& obstacle : obstacles)
    {
        float maxRadius = 50.0f;
        obstacle.position = { Random(maxRadius, SCREEN_WIDTH - maxRadius), Random(maxRadius, SCREEN_HEIGHT - maxRadius) };
        obstacle.radius = Random(5.0f, maxRadius);
    }

    // Distance offset from the player's center
    float targetOffsetDistance = 250.0f;

    // Distance threshold for the target to start pushing away from the player
    float pushAwayThreshold = 250.0f;

    // List to hold bullets
    vector<Bullet> bullets;

    bool playerEnteredLineOfSight = false;
    bool targetEnteredLineOfSight = false;

    Vector2 targetMovementDirection = { 0.0f, 1.0f }; // Default target movement direction (downwards)

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        const float playerSpeedDelta = playerSpeed * dt;
        const float playerRotationDelta = playerRotationSpeed * dt;

        if (IsKeyDown(KEY_W))
        {
            Vector2 newPosition = playerPosition + playerDirection * playerSpeedDelta;
            bool collision = false;
            for (const Circle& obstacle : obstacles)
            {
                if (CircleCollision(newPosition, playerRadius, obstacle.position, obstacle.radius))
                {
                    collision = true;
                    break;
                }
            }
            if (!collision)
                playerPosition = newPosition;
        }
        if (IsKeyDown(KEY_S))
        {
            Vector2 newPosition = playerPosition - playerDirection * playerSpeedDelta;
            bool collision = false;
            for (const Circle& obstacle : obstacles)
            {
                if (CircleCollision(newPosition, playerRadius, obstacle.position, obstacle.radius))
                {
                    collision = true;
                    break;
                }
            }
            if (!collision)
                playerPosition = newPosition;
        }
        if (IsKeyDown(KEY_D))
        {
            playerDirection = Rotate(playerDirection, playerRotationDelta);
        }
        if (IsKeyDown(KEY_A))
        {
            playerDirection = Rotate(playerDirection, -playerRotationDelta);
        }

        // Wrap the player's position around the screen boundaries
        WrapPosition(playerPosition);

        Vector2 targetEnd = targetPosition + Normalize(playerPosition - targetPosition) * viewDistance;
        bool playerVisible = LineCircle(targetPosition, targetEnd, playerPosition, playerRadius);

        // Calculate the distance between the player and the target circle
        float distanceToPlayer = Distance(targetPosition, playerPosition);

        // Check if the player is within 300 units of the target circle
        if (distanceToPlayer <= 300.0f)
        {
            if (!playerEnteredLineOfSight)
            {
                // Check if the target circle can see the player (line of sight)
                playerEnteredLineOfSight = playerVisible;
            }

            // Only start following the player's line of sight once the player enters the target's line of sight
            if (playerEnteredLineOfSight)
            {
                // Calculate the direction vector from the target to the player
                Vector2 directionToPlayer = Normalize(playerPosition - targetPosition);

                // Calculate the target position that is offset from the player's center
                Vector2 targetOffsetPosition = playerPosition + (directionToPlayer * targetOffsetDistance);

                // Move the target circle towards the targetOffsetPosition instead of directly to the player
                Vector2 directionToTarget = Normalize(targetOffsetPosition - targetPosition);

                // Calculate the new position of the target circle based on the direction and speed
                Vector2 newPosition = targetPosition + directionToTarget * playerSpeedDelta;
                bool collision = false;
                for (const Circle& obstacle : obstacles)
                {
                    if (CircleCollision(newPosition, targetRadius, obstacle.position, obstacle.radius))
                    {
                        collision = true;
                        break;
                    }
                }
                if (!collision)
                {
                    // Update the target circle's position
                    targetPosition = newPosition;
                }
                else
                {
                    // Move the target circle slightly away from the player in the opposite direction
                    targetPosition = targetPosition - directionToTarget * playerSpeedDelta;
                }

                // Check if the target circle is too close to the player and push it away
                if (distanceToPlayer < pushAwayThreshold)
                {
                    // Calculate the push-away direction
                    Vector2 pushAwayDirection = Normalize(targetPosition - playerPosition);

                    // Move the target circle away from the player
                    targetPosition = targetPosition + pushAwayDirection * playerSpeedDelta;
                }

                // Shooting behavior: Spawn bullets periodically
                static float shootTimer = 0.0f;
                const float shootInterval = 1.5f; // Adjust this value for bullet spawn frequency
                shootTimer += dt;
                if (shootTimer >= shootInterval)
                {
                    // Calculate the direction vector from the target to the player
                    Vector2 shootingDirection = Normalize(playerPosition - targetPosition);

                    // Randomize bullet direction slightly
                    float angleOffset = Random(-10.0f, 10.0f);
                    shootingDirection = Rotate(shootingDirection, angleOffset * DEG2RAD);

                    // Create a new bullet and add it to the bullets list
                    Bullet bullet;
                    bullet.rect.width = 15; // Adjust the width of the bullet rectangle
                    bullet.rect.height = 5; // Adjust the height of the bullet rectangle
                    bullet.rect.x = targetPosition.x;
                    bullet.rect.y = targetPosition.y - bullet.rect.height / 2; // Offset to align the bullet with the target center
                    bullet.direction = shootingDirection;
                    bullet.speed = 500.0f; // Adjust this value for bullet speed
                    bullets.push_back(bullet);

                    // Reset the shoot timer
                    shootTimer = 0.0f;
                }
            }
            else
            {
                // Calculate the direction vector from the target to the targetEnd point
                Vector2 directionToTargetEnd = Normalize(targetEnd - targetPosition);

                // Calculate the new position of the target circle based on the direction and speed
                Vector2 newPosition = targetPosition + directionToTargetEnd * playerSpeedDelta;
                bool collision = false;
                for (const Circle& obstacle : obstacles)
                {
                    if (CircleCollision(newPosition, targetRadius, obstacle.position, obstacle.radius))
                    {
                        collision = true;
                        break;
                    }
                }
                if (!collision)
                {
                    // Update the target circle's position
                    targetPosition = newPosition;
                }
                else
                {
                    // Move the target circle slightly away from the player in the opposite direction
                    targetPosition = targetPosition - directionToTargetEnd * playerSpeedDelta;
                }
            }

            // Check if the target is within 300 units of the player
            if (Distance(targetPosition, playerPosition) <= 300.0f)
            {
                if (!targetEnteredLineOfSight)
                {
                    // Check if the player can see the target (line of sight)
                    targetEnteredLineOfSight = LineCircle(playerPosition, playerPosition + playerDirection * viewDistance, targetPosition, targetRadius);
                }

                // Only start following the player's line of sight once the player enters the target's line of sight
                if (targetEnteredLineOfSight)
                {
                    // Calculate the direction vector from the player to the target
                    Vector2 directionToTarget = Normalize(targetPosition - playerPosition);

                    // Move the target towards the player
                    Vector2 newPosition = targetPosition - directionToTarget * playerSpeedDelta;
                    bool collision = false;
                    for (const Circle& obstacle : obstacles)
                    {
                        if (CircleCollision(newPosition, targetRadius, obstacle.position, obstacle.radius))
                        {
                            collision = true;
                            break;
                        }
                    }
                    if (!collision)
                    {
                        // Update the target's position
                        targetPosition = newPosition;
                    }
                    else
                    {
                        // Move the target slightly away from the player in the opposite direction
                        targetPosition = targetPosition + directionToTarget * playerSpeedDelta;
                    }

                    // Shooting behavior: Spawn bullets periodically
                    static float shootTimer = 0.0f;
                    const float shootInterval = 2.0f; // Adjust this value for bullet spawn frequency
                    shootTimer += dt;
                    if (shootTimer >= shootInterval)
                    {
                        // Calculate the direction vector from the target to the player
                        Vector2 shootingDirection = Normalize(playerPosition - targetPosition);

                        // Randomize bullet direction slightly
                        float angleOffset = Random(-20.0f, 20.0f);
                        shootingDirection = Rotate(shootingDirection, angleOffset * DEG2RAD);

                        // Create a new bullet and add it to the bullets list
                        Bullet bullet;
                        bullet.rect.width = 10; // Adjust the width of the bullet rectangle
                        bullet.rect.height = 3; // Adjust the height of the bullet rectangle
                        bullet.rect.x = targetPosition.x;
                        bullet.rect.y = targetPosition.y - bullet.rect.height / 2; // Offset to align the bullet with the target center
                        bullet.direction = shootingDirection;
                        bullet.speed = 600.0f; // Adjust this value for bullet speed
                        bullets.push_back(bullet);

                        // Reset the shoot timer
                        shootTimer = 0.0f;
                    }
                }
            }
            else
            {
                // Reset the targetEnteredLineOfSight flag if the target moves outside the player's view distance
                targetEnteredLineOfSight = false;
            }
        }
        else
        {
            // Reset the playerEnteredLineOfSight and targetEnteredLineOfSight flags if the player or target moves outside the view distance
            playerEnteredLineOfSight = false;
            targetEnteredLineOfSight = false;
        }

        // Wrap the target's position around the screen boundaries
        WrapPosition(targetPosition);

        // Update bullets' positions
        for (size_t i = 0; i < bullets.size(); ++i)
        {
            bullets[i].rect.x += bullets[i].direction.x * bullets[i].speed * dt;
            bullets[i].rect.y += bullets[i].direction.y * bullets[i].speed * dt;

            // Remove bullets that are out of screen to avoid unnecessary processing
            if (bullets[i].rect.x < 0 || bullets[i].rect.x > SCREEN_WIDTH ||
                bullets[i].rect.y < 0 || bullets[i].rect.y > SCREEN_HEIGHT)
            {
                bullets.erase(bullets.begin() + i);
                --i;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (const Bullet& bullet : bullets)
        {
            DrawRectangleRec(bullet.rect, RED);
        }

        // Only draw the target circle when the target starts chasing the player
        if (playerEnteredLineOfSight)
        {
            // Calculate the direction vector from the target to the player
            Vector2 directionToPlayer = Normalize(playerPosition - targetPosition);

            // Calculate the position of the target line's end point
            Vector2 targetLineEnd = targetPosition + directionToPlayer * (playerVisible ? playerLength : 0);

            // Draw the target line
            DrawLineV(targetPosition, targetLineEnd, GREEN);
        }

        // Draw the target circle regardless of whether it is chasing the player or not
        DrawCircleV(targetPosition, targetRadius, playerEnteredLineOfSight ? GREEN : RED);

        // Draw the player
        DrawCircleV(playerPosition, playerRadius, BLUE);
        DrawLineV(playerPosition, playerPosition + playerDirection * playerLength, DARKBLUE);

        for (const Circle& obstacle : obstacles)
            DrawCircleV(obstacle.position, obstacle.radius, GRAY);

        rlImGuiBegin();
        ImGui::SliderFloat("Tile Length", &viewDistance, 10.0f, 250.0f);
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
