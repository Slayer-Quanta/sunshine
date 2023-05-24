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

	Vector2 circlePosition = { 400, 300 };
	float circleRadius = 50;
	Color circleColor = RED;
	Texture2D background = LoadTexture("../game/assets/textures/galaxy.png");

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);
		BeginDrawing();
		ClearBackground(RAYWHITE);
		DrawTexture(background, 0, 0, WHITE);
		UpdateMusicStream(music);

		BeginDrawing();

		ClearBackground(RAYWHITE);

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
