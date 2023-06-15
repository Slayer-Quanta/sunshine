#include "rlImGui.h"
#include "Math.h"
#include <array>
#include <cmath>
#include <vector>
#include <string>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_COUNT 10

using namespace std;

constexpr float TILE_WIDTH = SCREEN_WIDTH / (float)TILE_COUNT;
constexpr float TILE_HEIGHT = SCREEN_HEIGHT / (float)TILE_COUNT;

enum TileType : size_t
{
    MOUNTAIN,
    MUD,
    WATER,
    GRASS,
    AIR,
    COUNT
};

struct Cell
{
    int col = -1;
    int row = -1;
};

Cell ScreenToTile(Vector2 position)
{
    return { int(position.x / TILE_WIDTH), int(position.y / TILE_HEIGHT) };
}

Vector2 TileToScreen(Cell cell)
{
    return { cell.col * TILE_WIDTH, cell.row * TILE_HEIGHT };
}

Vector2 TileCenter(Cell cell)
{
    return TileToScreen(cell) + Vector2{ TILE_WIDTH * 0.5f, TILE_HEIGHT * 0.5f };
}

void DrawTile(Cell cell, Color color)
{
    DrawRectangle(cell.col * TILE_WIDTH, cell.row * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, color);
}

void DrawTile(Cell cell, TileType type)
{
    Color color = WHITE;
    switch (type)
    {
    case MOUNTAIN:
        color = DARKGRAY;
        break;

    case MUD:
        color = BROWN;
        break;

    case WATER:
        color = BLUE;
        color.b = 180;
        break;

    case GRASS:
        color = GREEN;
        color.g = 180;
        break;
    }
    DrawTile(cell, color);
}

int ManhattanDistance(Cell currentCell, Cell goal)
{
    int distance = abs(currentCell.col - goal.col) + abs(currentCell.row - goal.row);
    return distance;

}
vector<Cell> Neighbours(Cell cell)
{
    vector<Cell> neighbors;
    vector<Cell> offsets{
        {0, -1},  
        {0, 1},   
        {-1, 0},  
        {1, 0},   
        {-1, -1}, 
        {1, -1}, 
        {-1, 1},  
        {1, 1}   
    };
    for (const auto& offset : offsets)
    {
        Cell adjacentCell{ cell.col + offset.col, cell.row + offset.row };
        neighbors.push_back(adjacentCell);
    }

    return neighbors;
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    SetTargetFPS(60);

    array<array<size_t, TILE_COUNT>, TILE_COUNT> tiles
    {
        array<size_t, TILE_COUNT>{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 1, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 2, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 2, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 2, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 2, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 2, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 2, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 3, 3, 1, 1, 1, 4, 4, 4, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    };

    Cell start{ 1, 1 };
    Cell goal{ 8, 8 };

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                // TODO -- calculate cost of each tile to the goal tile
                DrawTile({ col, row }, (TileType)tiles[row][col]);
                Vector2 texPos = TileCenter({ col, row });
                int distance = ManhattanDistance({ col, row }, goal);
                DrawText(to_string(distance).c_str(), texPos.x, texPos.y, 10, MAROON);
            }
        }
        DrawTile(ScreenToTile(GetMousePosition()), RED);
        DrawTile(start, DARKBLUE);
        DrawTile(goal, SKYBLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

