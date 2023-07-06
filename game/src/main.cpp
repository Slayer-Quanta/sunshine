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
}

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

size_t Index(Cell cell)
{
    return cell.row * TILE_COUNT + cell.col;
}


float Cost(TileType type)
{
    static array<float, COUNT> costs
    {
        0.0f,  
        10.0f,  
        25.0f,  
        50.0f,  
        100.0f, 
    };

    return costs[type];
}

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
struct Tile
{
    array<Tile*, 8> neighbors;
    float g;
    float h;
    float diagonalG; 
};

int main(void)
{
    Map map
    {
        array<size_t, TILE_COUNT>{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 2, 2, 2, 2, 2, 2, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 2, 3, 3, 3, 3, 2, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 2, 3, 4, 4, 3, 2, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 2, 3, 4, 4, 3, 2, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 2, 3, 4, 4, 3, 2, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 2, 2, 0, 0, 2, 2, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
        array<size_t, TILE_COUNT>{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    };

    Cell start{ 1, 1 };
    Cell goal{ 8, 8 };
    float dist1 = Manhattan(start, goal);
    float dist2 = Euclidean(start, goal);

    bool manhattan = true;
    vector<Cell> path = FindPath(start, goal, map, manhattan);

    vector<Cell> testPath;
    testPath.push_back({ 0, 0 });
    testPath.push_back({ 9, 0 });
    testPath.push_back({ 9, 9 });
    testPath.push_back({ 0, 9 });

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);
    const float duration = 1.0f;   
    float elapsed = 0.0f;        
    Vector2 currentPosition{};
    size_t currentIndex = 0;
    Vector2 characterPosition = TileCenter(start);
    Vector2 characterVelocity = { 0.0f, 0.0f };
    const float characterSpeed = 200.0f;

    bool useInterpolation = true;
    bool useSteering = false;

    const float maxSpeed = 200.0f;
    const float maxForce = 200.0f;
    float rotation = 0.0f;     
    const float maxAngularSpeed = 5.0f;
    Vector2 steeringForce{};
    Vector2 velocity{};
    Vector2 acceleration{};
    float mass = 1.0f;

    while (!WindowShouldClose())
    {
      
        const float dt = GetFrameTime();
        const float t = elapsed / duration;

        size_t nextIndex;
        if (useInterpolation || useSteering)
            nextIndex = (currentIndex + 1) % path.size();
        else
            nextIndex = currentIndex;

        Cell currentCell = path[currentIndex];
        Cell nextCell = path[nextIndex];
        Vector2 currentCenter = TileCenter(currentCell);
        Vector2 nextCenter = TileCenter(nextCell);

        if (useInterpolation)
        {
            currentPosition = Lerp(currentCenter, nextCenter, t);
            if (elapsed >= duration)
            {
                elapsed = 0;
                currentIndex = nextIndex;
            }
            else
                elapsed += dt;
        }
        else if (useSteering)
        {
            Vector2 desired = nextCenter - currentPosition;
            float distanceToNext = Length(desired);

            if (distanceToNext > 0)
            {
                desired = Normalize(desired);

                if (distanceToNext <= 2.0f)
                {
                    currentIndex = nextIndex;
                    currentPosition = currentCenter; 
                }
                else
                {
                    float speed = Remap(distanceToNext, 0.0f, 50.0f, 0.0f, maxSpeed) * dt;
                    desired = Scale(desired, speed);

                    Vector2 steer = Subtract(desired, velocity);
                    steer.x = Clamp(steer.x, -maxForce, maxForce);
                    steer.y = Clamp(steer.y, -maxForce, maxForce);

                    steer = Scale(steer, 1.0f / mass);
                    acceleration = Add(acceleration, steer);

                    rotation = atan2f(velocity.y, velocity.x) * RAD2DEG;
                    velocity = Add(velocity, acceleration);
                    float velocityMagnitude = Length(velocity);
                    if (velocityMagnitude > maxSpeed)
                    {
                        velocity = Scale(Normalize(velocity), maxSpeed);
                    }

                    currentPosition = Add(currentPosition, velocity);
                    acceleration = Vector2Zero();
                }
            }
            else
            { 
                float distanceToNext = Distance(currentPosition, nextCenter);
                if (distanceToNext <= 2.0f)
                    currentIndex = nextIndex;

                currentPosition = currentCenter;
            }
        }
       
        rlImGuiBegin();
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int row = 0; row < TILE_COUNT; row++)
        {
            for (int col = 0; col < TILE_COUNT; col++)
            {
                Cell cell{ static_cast<int>(col), static_cast<int>(row) };
                float g = manhattan ? Manhattan(cell, goal) : Euclidean(cell, goal);
                float h = manhattan ? Manhattan(cell, goal) : Euclidean(cell, goal);
                DrawTile(cell, map);
                Vector2 texPos = TileCenter(cell);
                DrawText(TextFormat("F: %f", g + h), texPos.x, texPos.y, 10, MAROON);
                DrawText(TextFormat("H: %.1f", h), cell.col * TILE_WIDTH, cell.row * TILE_HEIGHT + 15, 10, DARKBLUE);
            }
        }

        Vector2 cursor = GetMousePosition();
        Cell cursorTile = ScreenToTile(cursor);

        for (const Cell& cell : path)
            DrawTile(cell, RED);

        DrawTile(cursorTile, GRAY);
        DrawTile(start, DARKBLUE);
        DrawTile(goal, SKYBLUE);
        DrawCircleV(currentPosition, 25.0f, GREEN);
        DrawLineV(currentPosition, nextCenter, BLACK);
       
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
        ImGui::Checkbox("Interpolation", &useInterpolation);
        ImGui::Checkbox("Steering", &useSteering);

        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
