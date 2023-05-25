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
	Vector2 enterpriseSpeed = { 2.5f, 1.5f };
	Color enterpriseColor = WHITE;
	float enterpriseRotation = 0.0f;

	Vector2 circlePosition = { 400, 300 };
	float circleRadius = 50;
	Color circleColor = RED;
	Texture2D background = LoadTexture("../game/assets/textures/galaxy.png");

	bool collision = false; 
	bool prevCollision = false; 

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);
		ClearBackground(RAYWHITE);
		DrawTexture(background, 0, 0, WHITE);

		if (IsKeyDown(KEY_W))
			enterprisePosition.y -= 2.0f;
		if (IsKeyDown(KEY_S))
			enterprisePosition.y += 2.0f;
		if (IsKeyDown(KEY_A))
			enterprisePosition.x -= 2.0f;
		if (IsKeyDown(KEY_D))
			enterprisePosition.x += 2.0f;

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

		DrawCircle(circlePosition.x, circlePosition.y, circleRadius, circleColor);
		Vector2 mousePosition = GetMousePosition();
		prevCollision = collision;
		collision = CheckCollisionCircles(circlePosition, circleRadius, mousePosition, circleRadius);

		if (collision && !prevCollision)
		{
			PlaySound(yay); 
		}

		if (collision)
			circleColor = BLUE;
		else
			circleColor = RED;

		DrawCircle(mousePosition.x, mousePosition.y, circleRadius, circleColor);
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
