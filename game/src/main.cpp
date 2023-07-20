#include "rlImGui.h"
#include "Math.h"
#include <array>
constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TILE_COUNT = 10;
constexpr int TILE_WIDTH = SCREEN_WIDTH / TILE_COUNT;
constexpr int TILE_HEIGHT = SCREEN_HEIGHT / TILE_COUNT;
using namespace std;

bool LineCircle(Vector2 lineStart, Vector2 lineEnd, Vector2 circlePosition, float circleRadius)
{
    Vector2 nearest = ProjectPointLine(lineStart, lineEnd, circlePosition);
    return DistanceSqr(nearest, circlePosition) <= circleRadius * circleRadius;
}

bool LineCircle(Vector2 lineStart, Vector2 lineEnd, Vector2 circlePosition, float circleRadius, Vector2* poi = nullptr)
{
    Vector2 projection = ProjectPointLine(lineStart, lineEnd, circlePosition);
    Vector2 delta = circlePosition - projection;
    float radiiSum = circleRadius;
    float centerDistance = Length(delta);
    bool collision = centerDistance <= radiiSum;
    if (collision && poi != nullptr)
    {
        // Compute mtv (sum of radii - distance between centers)
        float mtvDistance = radiiSum - centerDistance;
        *poi = Normalize(delta) * mtvDistance + projection;
    }
    return collision;
}

struct Circle
{
    Vector2 position;
    float radius;
};

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

    Vector2 targetPosition{ SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.25f };
    float targetRadius = 25.0f;

    Vector2 obstaclePosition = Lerp(playerPosition, targetPosition, 0.5f);
    float obstacleRadius = 35.0f;

    array<Circle, 5> obstacles{};
    for (Circle& obstacle : obstacles)
    {
        float maxRadius = 50.0f;
        obstacle.position = { Random(maxRadius, SCREEN_WIDTH - maxRadius), Random(maxRadius, SCREEN_HEIGHT - maxRadius) };
        obstacle.radius = Random(5.0f, maxRadius);
    }

    float tileLength = 250.0f;
    array<array<Vector2, TILE_COUNT>, TILE_COUNT> tiles{};
    for (size_t row = 0; row < TILE_COUNT; row++)
    {
        for (size_t col = 0; col < TILE_COUNT; col++)
        {
            tiles[row][col] = { (float)col * TILE_WIDTH, (float)row * TILE_HEIGHT };
        }
    }

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        const float playerSpeedDelta = playerSpeed * dt;
        const float playerRotationDelta = playerRotationSpeed * dt;

        if (IsKeyDown(KEY_W))
        {
            playerPosition = playerPosition + playerDirection * playerSpeedDelta;
        }
        if (IsKeyDown(KEY_S))
        {
            playerPosition = playerPosition - playerDirection * playerSpeedDelta;
        }
        if (IsKeyDown(KEY_D))
        {
            playerDirection = Rotate(playerDirection, playerRotationDelta);
        }
        if (IsKeyDown(KEY_A))
        {
            playerDirection = Rotate(playerDirection, -playerRotationDelta);
        }
        Vector2 playerEnd = playerPosition + playerDirection * playerLength;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (size_t row = 0; row < TILE_COUNT; row++)
        {
            for (size_t col = 0; col < TILE_COUNT; col++)
            {
                Vector2 tileStart{ tiles[row][col].x + TILE_WIDTH * 0.5f, tiles[row][col].y + TILE_HEIGHT * 0.5f };
                Vector2 tileEnd = tileStart + Normalize(playerPosition - tileStart) * tileLength;
                Vector2 playerPoi{}, obstaclePoi{};
                Vector2 targetPoi{};
                bool playerCollision = LineCircle(tileStart, tileEnd, playerPosition, playerRadius, &playerPoi);
                bool obstacleCollision = LineCircle(tileStart, tileEnd, obstaclePosition, obstacleRadius, &obstaclePoi);
                bool playerVisible = playerCollision;
                bool targetVisible = LineCircle(tileStart, tileEnd, targetPosition, targetRadius, &targetPoi);

                if (playerCollision && targetVisible)
                {
                    DrawRectangleV(tiles[row][col], { TILE_WIDTH, TILE_HEIGHT }, MAGENTA);
                }
                else if (targetVisible) 
                {
                    DrawRectangleV(tiles[row][col], { TILE_WIDTH, TILE_HEIGHT }, PURPLE);
                }
                else 
                {
                    DrawRectangleV(tiles[row][col], { TILE_WIDTH, TILE_HEIGHT }, RED);
                }
            }
        }

        DrawLineV(playerPosition, playerEnd, BLUE);
        DrawCircleV(playerPosition, playerRadius, BLUE);
        DrawCircleV(targetPosition, targetRadius, VIOLET);
        DrawCircleV(obstaclePosition, obstacleRadius, GRAY);
        for (const Circle& obstacle : obstacles)
            DrawCircleV(obstacle.position, obstacle.radius, GRAY);

        
        for (const Circle& obstacle : obstacles)
        {
            Vector2 playerPoi{};
            if (LineCircle(playerPosition, playerEnd, obstacle.position, obstacle.radius, &playerPoi))
            {
                DrawCircleV(playerPoi, 5.0f, BLUE); 
            }
        }

        for (const Circle& obstacle : obstacles)
        {
            Vector2 targetPoi{};
            if (LineCircle(targetPosition, targetPosition, obstacle.position, obstacle.radius, &targetPoi))
            {
                DrawCircleV(targetPosition, 5.0f, VIOLET); 
                break;
            }
        }

        rlImGuiBegin();
        ImGui::SliderFloat("Tile Length", &tileLength, 10.0f, 250.0f);
        rlImGuiEnd();

        EndDrawing();
    }
}