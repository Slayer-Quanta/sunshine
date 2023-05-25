#include "rlImGui.h"
#include "raylib.h"
#include "Math.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SUNSHINE");
    InitAudioDevice();
    SetTargetFPS(60);

    Texture2D enterprise = LoadTexture("../game/assets/textures/enterprise.png");
    Music music = LoadMusicStream("../game/assets/audio/DANGERZONE.MP3");
    Sound yay = LoadSound("../game/assets/audio/yay.ogg");
    PlayMusicStream(music);

    Vector2 enterprisePosition = { static_cast<float>(SCREEN_WIDTH) / 2, static_cast<float>(SCREEN_HEIGHT) / 2 };
    Vector2 enterpriseSpeed = { 5.5f, 4.5f };
    Color enterpriseColor = WHITE;
    float enterpriseRotation = 0.0f;

    Vector2 circlePosition = { 400, 300 };
    float circleRadius = 50;
    Color circleColor = RED;

    Vector2 shapePosition = { 600, 400 };
    float shapeSize = 40;
    Color shapeColor = GREEN;
    Vector2 shapeVelocity = { 4.0f, 4.5f };

    Texture2D background = LoadTexture("../game/assets/textures/galaxy.png");

    bool circleCollision = false;
    bool prevCircleCollision = false;

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);
        ClearBackground(RAYWHITE);
        DrawTexture(background, 0, 0, WHITE);

        if (IsKeyDown(KEY_W))
            enterprisePosition.y -= 10.0f;
        if (IsKeyDown(KEY_S))
            enterprisePosition.y += 10.0f;
        if (IsKeyDown(KEY_A))
            enterprisePosition.x -= 7.0f;
        if (IsKeyDown(KEY_D))
            enterprisePosition.x += 7.0f;

        BeginDrawing();
        ClearBackground(RAYWHITE);
        enterprisePosition.x += enterpriseSpeed.x;
        enterprisePosition.y += enterpriseSpeed.y;

        if (enterprisePosition.x + enterprise.width >= SCREEN_WIDTH || enterprisePosition.x <= 0)
        {
            enterpriseSpeed.x *= -1;
            enterprisePosition.x = Clamp(enterprisePosition.x, 0, SCREEN_WIDTH - enterprise.width);
        }
        if (enterprisePosition.y + enterprise.width >= SCREEN_HEIGHT || enterprisePosition.y <= 0)
        {
            enterpriseSpeed.y *= -1;
            enterprisePosition.y = Clamp(enterprisePosition.y, 0, SCREEN_HEIGHT - enterprise.height);
        }

        enterpriseRotation += 1.0f;
        DrawTexturePro(enterprise, { 0, 0, static_cast<float>(enterprise.width), static_cast<float>(enterprise.height) },
            { enterprisePosition.x, enterprisePosition.y, static_cast<float>(enterprise.width), static_cast<float>(enterprise.height) },
            { static_cast<float>(enterprise.width) / 2, static_cast<float>(enterprise.height) / 2 }, enterpriseRotation, enterpriseColor);

        Vector2 mousePosition = GetMousePosition();

        DrawCircle(circlePosition.x, circlePosition.y, circleRadius, circleColor);
        circleCollision = CheckCollisionCircles(circlePosition, circleRadius, mousePosition, circleRadius);
        if (circleCollision)
            circleColor = BLUE;
        else
            circleColor = RED;
        DrawCircle(mousePosition.x, mousePosition.y, circleRadius, circleColor);

        shapePosition.x += shapeVelocity.x;
        shapePosition.y += shapeVelocity.y;

        if (shapePosition.x + shapeSize >= SCREEN_WIDTH || shapePosition.x <= 0)
        {
            shapeVelocity.x *= -1;
            shapePosition.x = Clamp(shapePosition.x, 0, SCREEN_WIDTH - shapeSize);
        }
        if (shapePosition.y + shapeSize >= SCREEN_HEIGHT || shapePosition.y <= 0)
        {
            shapeVelocity.y *= -1;
            shapePosition.y = Clamp(shapePosition.y, 0, SCREEN_HEIGHT - shapeSize);
        }

        DrawRectangle(shapePosition.x, shapePosition.y, shapeSize, shapeSize, shapeColor);

        if (circleCollision && !prevCircleCollision)
        {
            PlaySound(yay);
        }

        Vector2 circleCenter = { circlePosition.x + circleRadius, circlePosition.y + circleRadius };
        DrawLine(circleCenter.x, circleCenter.y, mousePosition.x, mousePosition.y, YELLOW);

        prevCircleCollision = circleCollision;

        EndDrawing();
    }

    UnloadTexture(enterprise);
    UnloadTexture(background);
    UnloadSound(yay);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
