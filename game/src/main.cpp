#include "rlImGui.h"
#include "raylib.h"

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
	PlaySound(yay);
	PlayMusicStream(music);

	Vector2 enterprisePosition = { static_cast<float>(SCREEN_WIDTH) / 2, static_cast<float>(SCREEN_HEIGHT) / 2 };
	Vector2 enterpriseSpeed = { 2.5f, 1.5f };
	Color enterpriseColor = WHITE;
	float enterpriseRotation = 0.0f;

	Vector2 circlePosition = { 400, 300 };
	float circleRadius = 50;
	Color circleColor = RED;
	Texture2D background = LoadTexture("../game/assets/textures/galaxy.png");

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);
		ClearBackground(RAYWHITE);
		DrawTexture(background, 0, 0, WHITE);
		UpdateMusicStream(music);

		BeginDrawing();

		ClearBackground(RAYWHITE);

		enterprisePosition.x += enterpriseSpeed.x;
		enterprisePosition.y += enterpriseSpeed.y;

		if (enterprisePosition.x + enterprise.width >= SCREEN_WIDTH || enterprisePosition.x <= 0)
			enterpriseSpeed.x *= -1;
		if (enterprisePosition.y + enterprise.width >= SCREEN_WIDTH || enterprisePosition.y <= 0)
			enterpriseSpeed.y *= -1;
		
		enterpriseRotation += 1.0f;
		DrawTexturePro(enterprise, { 0, 0, static_cast<float>(enterprise.width), static_cast<float>(enterprise.height) },
			{ enterprisePosition.x, enterprisePosition.y, static_cast<float>(enterprise.width), static_cast<float>(enterprise.height) },
			{ static_cast<float>(enterprise.width) / 2, static_cast<float>(enterprise.height) / 2 }, enterpriseRotation, enterpriseColor);

		DrawText("Hello World", 16, 9, 20, RED);
		DrawTexture(enterprise, 0, 0, WHITE);

		DrawCircle(circlePosition.x, circlePosition.y, circleRadius, circleColor);
		Vector2 mousePosition = GetMousePosition();
		bool collision = CheckCollisionCircles(circlePosition, circleRadius, mousePosition, circleRadius);
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
