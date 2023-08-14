#include "rlImGui.h"
#include "Physics.h"
#include <fstream>
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
using namespace std;


void Save(const Circles& obstacles, const char* path = "../game/assets/data/obstacles.txt")
{
    ofstream out(path);
    for (size_t i = 0; i < obstacles.size(); i++)
    {
        // Outputting an endl after the final obstacle is breaking file loading.
        // There's probably a better way to do this, but better this than manually editing the file!
        const Circle& obstacle = obstacles[i];
        if (i == obstacles.size() - 1)
            out << obstacle.position.x << ' ' << obstacle.position.y << ' ' << obstacle.radius;
        else
            out << obstacle.position.x << ' ' << obstacle.position.y << ' ' << obstacle.radius << endl;
    }
    out.close();
}

void Load(Circles& obstacles, const char* path = "../game/assets/data/obstacles.txt")
{
    ifstream in(path);
    while (!in.eof())
    {
        Circle obstacle;
        in >> obstacle.position.x >> obstacle.position.y >> obstacle.radius;
        obstacles.push_back(obstacle);
    }
}

struct Timer
{
    float duration = 0.0f;
    float elapsed = 0.0f;

    bool Expired() { return elapsed >= duration; }
    void Reset() { elapsed = 0.0f; }
    void Tick(float dt) { elapsed += dt; }
};

// TODO for late submission (20% per task):
// 1. Resets attack timer if entering ATTACK state (transition function)
// 2. Sets waypointIndex to nearest waypoint if entering PATROL state (transition function)
// 3. Change bullet spawning so that bullets don't spawn inside the AI (define a bullet radius, see game_concepts branch 39)
// 4. Remove bullets that are colliding with things, or off-screen (remove_if, see game_concepts branch 39)
// 5. Render health bars
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sunshine");
    rlImGuiSetup(true);
    SetTargetFPS(60);

    Vector2 targetPosition{ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    float targetRadius = 20.0f;
    float targetViewDistance = 1000.0f;

    Points waypoints
    {
        { SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.25f },
        { SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.25f },
        { SCREEN_WIDTH * 0.75f, SCREEN_HEIGHT * 0.75f },
        { SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.75f },
    };
    float waypointRadius = 25.0f;

    Circles obstacles;
    Load(obstacles);

    Probes probes(4);
    probes[0].angle = 30.0f;
    probes[1].angle = 15.0f;
    probes[2].angle = -15.0f;
    probes[3].angle = -30.0f;
    probes[0].length = probes[3].length = 100.0f;
    probes[1].length = probes[2].length = 250.0f;

    Rigidbody rb;
    rb.pos = { SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    rb.angularSpeed = 100.0f * DEG2RAD;
    float seekerRadius = 20.0f;
    float seekerProximity = 150.0f; // distance for waypoints and player detection
    float seekerSpeed = 250.0f;
    Color proximityColor = SKYBLUE;
    proximityColor.a = 64;
    size_t waypointIndex = 1;

    Timer timer;
    timer.duration = 0.5f;

    vector<Rigidbody> bullets;

    while (!WindowShouldClose())
    {
        const float dt = GetFrameTime();
        targetPosition = GetMousePosition();
        ResolveCircles(targetPosition, targetRadius, obstacles);

        // Attack if within proximity, otherwise patrol
        if (Distance(rb.pos, targetPosition) <= seekerProximity)
        {
            if (IsVisible(rb.pos, seekerProximity, targetPosition, targetRadius, obstacles))
            {
                if (timer.Expired())
                {
                    timer.Reset();
                    // Calculate a spawn position that is outside the AI
                    Vector2 spawnPosition = rb.pos + Normalize(targetPosition - rb.pos) * seekerRadius;

                    Rigidbody bullet;
                    bullet.pos = spawnPosition;
                    bullet.vel = Normalize(targetPosition - spawnPosition) * Random(250.0f, 500.0f);
                    bullets.push_back(bullet);
                    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](const Rigidbody& bullet) {
                        bool removeBullet = false;

                        // Check for collision with obstacles
                        for (const Circle& obstacle : obstacles)
                        {
                            Circle bulletCircle = { bullet.pos, 10.0f }; // Create a Circle object for the bullet
                            if (CircleCircle(bulletCircle, obstacle, nullptr))
                            {
                                removeBullet = true; 
                                break; 
                            }
                        }
                        if (!removeBullet && (bullet.pos.x < 0 || bullet.pos.x > SCREEN_WIDTH ||
                            bullet.pos.y < 0 || bullet.pos.y > SCREEN_HEIGHT))
                        {
                            removeBullet = true;
                        }
                        return removeBullet;
                        }), bullets.end());



                }
            }

            timer.Tick(dt);
            rb.acc = Seek(targetPosition, rb.pos, rb.vel, seekerSpeed);

        }
        else
        {
            // Find the nearest waypoint
            float nearestDistance = Distance(rb.pos, waypoints[0]);
            size_t nearestIndex = 0;

            for (size_t i = 1; i < waypoints.size(); i++)
            {
                float distance = Distance(rb.pos, waypoints[i]);
                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestIndex = i;
                }
            }

            waypointIndex = nearestIndex; // Set the waypointIndex to the nearest waypoint
            rb.acc = Seek(waypoints[waypointIndex], rb.pos, rb.vel, seekerSpeed);
        }

        rb.acc = rb.acc + Avoid(rb, dt, obstacles, probes);
        Update(rb, dt);
        ResolveCircles(rb.pos, seekerRadius, obstacles);
        bool seekerVisible = IsVisible(rb.pos, targetViewDistance, targetPosition, targetRadius, obstacles);

        for (Rigidbody& bullet : bullets)
            Update(bullet, dt);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawCircleV(targetPosition, seekerRadius, seekerVisible ? GREEN : RED);
        DrawLineV(targetPosition, targetPosition + Normalize(rb.pos - targetPosition) * targetViewDistance, seekerVisible ? GREEN : RED);

        DrawCircleV(rb.pos, seekerRadius, BLUE);
        DrawCircleV(rb.pos, seekerProximity, proximityColor);
        DrawLineV(rb.pos, rb.pos + rb.dir * 250.0f, DARKBLUE);

        for (const Probe& probe : probes)
            DrawLineV(rb.pos, rb.pos + Rotate(Normalize(rb.vel), probe.angle * DEG2RAD) * probe.length, PURPLE);

        for (const Circle& obstacle : obstacles)
            DrawCircleV(obstacle.position, obstacle.radius, GRAY);

        for (const Vector2& point : waypoints)
            DrawCircleV(point, waypointRadius, DARKBLUE);

        for (const Rigidbody& bullet : bullets)
            DrawCircleV(bullet.pos, 10.0f, RED);

        rlImGuiBegin();
        ImGui::SliderFloat("View Distance", &targetViewDistance, 10.0f, 1250.0f);

        if (ImGui::Button("Save Obstacles"))
        {
            Save(obstacles);
        }

        if (ImGui::Button("Load Obstacles"))
        {
            obstacles.clear();
            Load(obstacles);
        }

        if (ImGui::Button("Add Obstacle"))
        {
            Circle obstacle;
            obstacle.position = { 0.0f, 0.0f };
            obstacle.radius = 100.0f;
            obstacles.push_back(obstacle);
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove Obstacle"))
        {
            if (!obstacles.empty())
            {
                obstacles.pop_back();
            }
        }

        for (size_t i = 0; i < obstacles.size(); i++)
        {
            char obstacleLabel[64];
            sprintf(obstacleLabel, "Obstacle %zu", i);

            if (ImGui::TreeNode(obstacleLabel))
            {
                ImGui::SliderFloat2("Position", (float*)&obstacles[i].position, 0.0f, SCREEN_WIDTH);
                ImGui::SliderFloat("Radius", &obstacles[i].radius, 0.0f, 250.0f);
                if (ImGui::Button("Remove"))
                {
                    obstacles.erase(obstacles.begin() + i);
                    i--;
                }
                ImGui::TreePop();
            }
        }
        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
