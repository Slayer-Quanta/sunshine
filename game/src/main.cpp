#include "rlImGui.h"
#include "Math.h"
#include <array>
#include <vector>
#include <queue>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TILE_COUNT 10

using namespace std;

constexpr float TILE_WIDTH = SCREEN_WIDTH / (float)TILE_COUNT;
constexpr float TILE_HEIGHT = SCREEN_HEIGHT / (float)TILE_COUNT;

using Map = array<array<size_t, TILE_COUNT>, TILE_COUNT>;

enum TileType : size_t
{
    AIR,
    GRASS,
    WATER,
    MUD,
    MOUNTAIN,
    COUNT
};

struct Cell
{
    int col = -1;
    int row = -1;
};

float Manhattan(Cell a, Cell b)
{
    return abs(b.col - a.col) + abs(b.row - a.row);
}

float Euclidean(Cell a, Cell b)
{
    return sqrtf(powf(b.col - a.col, 2.0f) + powf(b.row - a.row, 2.0f));
    //return sqrtf((b.col - a.col) * (b.col - a.col) + (b.row - a.row) * (b.row - a.row));
    // Identical to the above implementation
}

// From game world to graph world "Quantization"
Cell ScreenToTile(Vector2 position)
{
    return { int(position.x / TILE_WIDTH), int(position.y / TILE_HEIGHT) };
}

// From graph world to game world "Localization"
Vector2 TileToScreen(Cell cell)
{
    return { cell.col * TILE_WIDTH, cell.row * TILE_HEIGHT };
}

Vector2 TileCenter(Cell cell)
{
    return TileToScreen(cell) + Vector2{ TILE_WIDTH * 0.5f, TILE_HEIGHT * 0.5f };
}

// Go from 2d to 1d (necessary for path finding data structures)
size_t Index(Cell cell)
{
    return cell.row * TILE_COUNT + cell.col;
}

// Go from 1d to 2d
//Cell From(size_t index)
//{
//    return { index % TILE_COUNT, index / TILE_COUNT };
//}

float Cost(TileType type)
{
    static array<float, COUNT> costs
    {
        0.0f,   // AIR
        10.0f,  // GRASS
        25.0f,  // WATER
        50.0f,  // MUD
        100.0f, // MOUNTAIN
    };

    return costs[type];
}

// Returns all adjacent cells to the passed-in cell (up, down, left, right & diagonals)
vector<Cell> Neighbours(Cell cell)
{
    vector<Cell> neighbours;
    for (int row = -1; row <= 1; row++)
    {
        for (int col = -1; col <= 1; col++)
        {
            // Don't add the passed-in cell to the list
            if (row == cell.row && col == cell.col) continue;

            Cell neighbour{ cell.col + col, cell.row + row };
            if (neighbour.col >= 0 && neighbour.col < TILE_COUNT &&
                neighbour.row >= 0 && neighbour.row < TILE_COUNT)
                neighbours.push_back(neighbour);
        }
    }
    return neighbours;
}

struct Node
{
    Node()
    {
        Init();
    }

    Node(Cell cell)
    {
        Init(cell);
    }

    Node(Cell cell, float g, float h)
    {
        Init(cell, {}, g, h);
    }

    Node(Cell cell, Cell parent, float g, float h)
    {
        Init(cell, parent, g, h);
    }

    void Init(Cell cell = {}, Cell parent = {}, float g = 0.0f, float h = 0.0f)
    {
        this->cell = cell;
        this->parent = parent;
        this->g = g;
        this->h = h;
    }

    float F() { return g + h; }

    float g;
    float h;

    Cell cell;
    Cell parent;
};

bool operator==(Cell a, Cell b)
{
    return a.row == b.row && a.col == b.col;
}

bool Compare(Node a, Node b)
{
    return a.F() > b.F();
}

vector<Cell> FindPath(Cell start, Cell end, Map map, bool manhattan)
{
    // 1:1 mapping of graph nodes to tile map
    const int nodeCount = TILE_COUNT * TILE_COUNT;
    vector<Node> tileNodes(nodeCount);
    vector<bool> closedList(nodeCount, false);
    priority_queue<Node, vector<Node>, decltype(&Compare)> openList(Compare);
    tileNodes[Index(start)].parent = start;
    openList.push(start);

    // Loop until we've reached the goal, or explored every tile
    while (!openList.empty())
    {
        const Cell currentCell = openList.top().cell;

        // Stop exploring once we've found the goal
        if (currentCell == end)
            break;

        // Otherwise, add current cell to closed list and update g & h values of its neighbours
        openList.pop();
        closedList[Index(currentCell)] = true;

        float gNew, hNew;
        for (const Cell& neighbour : Neighbours(currentCell))
        {
            const size_t neighbourIndex = Index(neighbour);

            // Skip if already explored
            if (closedList[neighbourIndex]) continue;

            // Calculate scores
            gNew = manhattan ? Manhattan(currentCell, neighbour) : Euclidean(currentCell, neighbour);
            if (!manhattan)
            {
                // Calculate diagonal g-score
                const float diagonalG = tileNodes[Index(currentCell)].g + sqrtf(2.0f) * Cost((TileType)map[neighbour.row][neighbour.col]);
                if (diagonalG < gNew)
                    gNew = diagonalG;
            }
            hNew = manhattan ? Manhattan(neighbour, end) : Euclidean(neighbour, end);
            hNew += Cost((TileType)map[neighbour.row][neighbour.col]);

            // Append if unvisited or best score
            if (tileNodes[neighbourIndex].F() <= FLT_EPSILON /*unexplored*/ ||
                gNew + hNew < tileNodes[neighbourIndex].F() /*better score*/)
            {
                openList.push({ neighbour, gNew, hNew });
                tileNodes[neighbourIndex] = { neighbour, currentCell, gNew, hNew };
            }
        }
    }

    vector<Cell> path;
    Cell currentCell = end;
    size_t currentIndex = Index(currentCell);

    while (!(tileNodes[currentIndex].parent == currentCell))
    {
        path.push_back(currentCell);
        currentCell = tileNodes[currentIndex].parent;
        currentIndex = Index(currentCell);
    }
    path.push_back(start);
    reverse(path.begin(), path.end());

    return path;
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

void DrawTile(Cell cell, Map map)
{
    DrawTile(cell, (TileType)map[cell.row][cell.col]);
}

// Late task 1:
// Consider building a persistent grid to handle g scores of diagonals when using euclidean distance if you want full marks on LE4 late submission.
struct Tile
{
    // Store neighbors (4 directions + diagonals)
    array<Tile*, 8> neighbors;
    float g;
    float h;
    float diagonalG; // New member for diagonal g-scores
};


int main(void)
{
    Map map
    {
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 0, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 0, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 4, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 0, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 0, 0, 4, 0, 0, 0, 0, 0 },
        array<size_t, TILE_COUNT>{ 0, 0, 0, 0, 4, 4, 0, 0, 0, 0 },
    };

    Cell start{ 1, 1 };
    Cell goal{ 8, 8 };
    float dist1 = Manhattan(start, goal);
    float dist2 = Euclidean(start, goal);

    bool manhattan = true;
    vector<Cell> path = FindPath(start, goal, map, manhattan);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // TODO (bonus) write code to move an object along the path using interpolation

        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                Cell cell{ col, row };
                float g = manhattan ? Manhattan(cell, goal) : Euclidean(cell, goal);
                float h = Manhattan(cell, goal);

                DrawTile(cell, map);
                Vector2 texPos = TileCenter(cell);
                DrawText(TextFormat("F: %f", g + h), texPos.x, texPos.y, 10, MAROON);
            }
        }

        Vector2 cursor = GetMousePosition();
        Cell cursorTile = ScreenToTile(cursor);

        for (const Cell& cell : path)
            DrawTile(cell, RED);

        DrawTile(cursorTile, GRAY);
        DrawTile(start, DARKBLUE);
        DrawTile(goal, SKYBLUE);

        // We can see quantization & localization in-action if we convert the cursor to tile coordinates
        //DrawText(TextFormat("row %i, col %i", cursorTile.row, cursorTile.col), cursor.x, cursor.y, 20, DARKGRAY);

        rlImGuiBegin();


        bool startChanged = false;
        bool goalChanged = false;
        bool manhattanChanged = false;

        if (ImGui::Button("Find path"))
        {
            path = FindPath(start, goal, map, true);
        }

        startChanged = ImGui::SliderInt2("Start", &start.col, 0, TILE_COUNT - 1);
        goalChanged = ImGui::SliderInt2("Goal", &goal.col, 0, TILE_COUNT - 1);
        ImGui::Checkbox("Toggle Manhattan", &manhattan);

        if (startChanged || goalChanged || manhattanChanged)
        {
            path = FindPath(start, goal, map, manhattan);
        }

        rlImGuiEnd();

        EndDrawing();
      
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}