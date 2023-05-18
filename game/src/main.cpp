#include "rlImGui.h"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SUNSHINE");
	InitAudioDevice();
	SetTargetFPS(60);

	Texture Enterprise = LoadTexture("../game/assets/textures/enterprise.png");
	Music music = LoadMusicStream("../game/assets/audio/DANGERZONE.MP3");
	Sound Yay = LoadSound("../game/assets/audio/yay.ogg");
	PlaySound(Yay);
	PlayMusicStream(music);

	while (!WindowShouldClose())
	{
		UpdateMusicStream(music);

		BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawText("Hello World",16,9, 20, RED);
			DrawTexture(Enterprise, 0, 0, WHITE);
			EndDrawing();
	}
	UnloadTexture(Enterprise);
	UnloadSound(Yay);
	UnloadMusicStream(music);
	CloseAudioDevice();
	CloseWindow();
	return 0;
}