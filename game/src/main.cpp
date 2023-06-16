#include "raylib.h"
#include "Math.h"
#include "Collision.h"
#include <vector>

using namespace std;

#define SCREEN_WIDTH 1268
#define SCREEN_HEIGHT 720

struct Rigidbody
{
    Vector2 position{};
    Vector2 velocity{};
    Vector2 acceleration{};
    Vector2 direction{};
    float Rotation{};
    float Speed{};
};

struct Obstacle
{
    Vector2 position{};
    float radius{};
};

struct Food
{
    Vector2 position{};
    float radius{};
};

class Fish
{
public:
    Rigidbody rigidbody;
    Texture2D texture;
    float width;
    float height;
    float maxSpeed;
    float maxAcceleration;
    Fish(const Vector2& position, const Texture2D& texture, float width, float height, float speed, float acceleration) : maxSpeed(speed), maxAcceleration(acceleration)
    {
        this->texture = texture;
        this->width = width;
        this->height = height;
        rigidbody.position = position;
        rigidbody.velocity = { 0, 0 };
        rigidbody.direction = { 1, 0 };
        rigidbody.Rotation = 0.0f;
    }
    void UpdateRigidBody(float deltaTime)
    {
        rigidbody.velocity = rigidbody.velocity + (rigidbody.acceleration * deltaTime);
        float speed = Length(rigidbody.velocity);
        if (speed > maxSpeed)
        {
            rigidbody.velocity = Normalize(rigidbody.velocity) * maxSpeed;
        }
        rigidbody.position = rigidbody.position + (rigidbody.velocity * deltaTime);
        if (speed > 0)
        {
            rigidbody.direction = Normalize(rigidbody.velocity);
        }
        rigidbody.Rotation = atan2f(rigidbody.direction.y, rigidbody.direction.x) * RAD2DEG;
        rigidbody.acceleration = { 0, 0 };
    }
    void Draw() const
    {
        Rectangle sourceRect = { 0, 0, (float)texture.width, (float)texture.height };
        Rectangle destRect = { rigidbody.position.x, rigidbody.position.y, width, height };
        Vector2 origin = { width / 2.0f, height / 2.0f };
        DrawTexturePro(texture, sourceRect, destRect, origin, rigidbody.Rotation, WHITE);
    }
};

float Distance(const Vector2& v1, const Vector2& v2)
{
    float dx = v2.x - v1.x;
    float dy = v2.y - v1.y;
    return (dx * dx) + (dy * dy);
}
int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "SUNSHINE Aquarium");
    SetTargetFPS(60);

    Texture2D Background = LoadTexture("../game/assets/textures/Coral reef.png");
    Texture2D fishTexture = LoadTexture("../game/assets/textures/Fish.png");
    vector<Fish> fish;
    fish.push_back(Fish({ 100, 200 }, fishTexture, 100, 90, 250.0f, 300.0f));
  
    fish.push_back(Fish({ 200, 300 }, fishTexture, 100, 90, 300.0f, 400.0f));
    fish.push_back(Fish({ 300, 400 }, fishTexture, 100, 90, 350.0f, 500.0f));
    
    fish.push_back(Fish({ 400, 500 }, fishTexture, 100, 90, 400.0f, 350.0f));
    fish.push_back(Fish({ 500, 600 }, fishTexture, 100, 90, 300.0f, 400.0f));
   
    fish.push_back(Fish({ 600, 700 }, fishTexture, 100, 90, 350.0f, 450.0f));
    fish.push_back(Fish({ 700, 800 }, fishTexture, 100, 90, 275.0f, 250.0f));

    fish.push_back(Fish({ 800, 900 }, fishTexture, 100, 90, 400.0f, 300.0f));
    fish.push_back(Fish({ 900, 1000 }, fishTexture, 100, 90, 450.0f, 350.0f));
    fish.push_back(Fish({ 1000, 1100 }, fishTexture, 100, 90, 500.0f, 400.0f));

    vector<Obstacle> obstacles;
    vector<Food> foods;

    Vector2 targetPosition{};
    bool Seek = false;
    bool Flee = false;
    bool Arrival = false;
    bool Avoid = false;


    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE))
        {
            fish.clear();
            fish.push_back(Fish({ 100, 200 }, fishTexture, 100, 90, 250.0f, 300.0f));
           
            fish.push_back(Fish({ 200, 300 }, fishTexture, 100, 90, 300.0f, 400.0f));
            fish.push_back(Fish({ 300, 400 }, fishTexture, 100, 90, 350.0f, 500.0f));
            
            fish.push_back(Fish({ 400, 500 }, fishTexture, 100, 90, 400.0f, 350.0f));
            fish.push_back(Fish({ 500, 600 }, fishTexture, 100, 90, 300.0f, 400.0f));
           
            fish.push_back(Fish({ 600, 700 }, fishTexture, 100, 90, 350.0f, 450.0f));
            fish.push_back(Fish({ 700, 800 }, fishTexture, 100, 90, 275.0f, 250.0f));

            fish.push_back(Fish({ 800, 900 }, fishTexture, 100, 90, 400.0f, 300.0f));
            fish.push_back(Fish({ 900, 1000 }, fishTexture, 100, 90, 450.0f, 350.0f));
            fish.push_back(Fish({ 1000, 1100 }, fishTexture, 100, 90, 500.0f, 400.0f));


            Seek = false;
            Flee = false;
            Arrival = false;
            Avoid = false;
        }
        if (IsKeyPressed(KEY_ONE))
        {
            Seek = true;
            Flee = false;
            Arrival = false;
            Avoid = false;          
        }
        else if (IsKeyPressed(KEY_TWO))
        {
            Seek = false;
            Flee = true;
            Arrival = false;
            Avoid = false;
            
        }
        else if (IsKeyPressed(KEY_THREE))
        {
            Seek = false;
            Flee = false;
            Arrival = true;
            Avoid = false;
        }
        else if (IsKeyPressed(KEY_FOUR))
        {
            Seek = false;
            Flee = false;
            Arrival = false;
            Avoid = true;
        }

        if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
        {
            if (Seek)
            {
                targetPosition = GetMousePosition();
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            if (Avoid)
            {
                Obstacle obstacle;
                obstacle.position = GetMousePosition();
                obstacle.radius = 10;
                obstacles.push_back(obstacle);
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            if (Arrival)
            {
                Food food;
                food.position = GetMousePosition();
                food.radius = 10;
                foods.push_back(food);
            }
        }
        for (Fish& fish : fish)
        {
            if (Seek)
            {
                Vector2 desiredVel = targetPosition - fish.rigidbody.position;
                desiredVel = Normalize(desiredVel) * fish.maxSpeed;
                Vector2 steering = desiredVel - fish.rigidbody.velocity;
                fish.rigidbody.acceleration = fish.rigidbody.acceleration + steering;
            }
            if (Flee)
            {
                Vector2 predatorPosition = GetMousePosition();
                Vector2 desiredVel = fish.rigidbody.position - predatorPosition;
                desiredVel = Normalize(desiredVel) * fish.maxSpeed;
                Vector2 steering = desiredVel - fish.rigidbody.velocity;
                fish.rigidbody.acceleration = fish.rigidbody.acceleration + steering;
             
            }
            if (Arrival)
            {
                for (const Food& food : foods)
                {
                    Vector2 desiredVel = food.position - fish.rigidbody.position;
                    float distance = Length(desiredVel);
                    float slowRadius = 100.0f;
                    float arriveRadius = 25.0f;

                    if (distance > slowRadius)
                    {
                        desiredVel = Normalize(desiredVel) * fish.maxSpeed;
                    }
                    else if (distance > arriveRadius)
                    {
                        float t = distance / slowRadius;
                        desiredVel = Normalize(desiredVel) * fish.maxSpeed * t;
                    }
                    else
                    {
                        desiredVel = { 0, 0 };
                    }

                    Vector2 steering = desiredVel - fish.rigidbody.velocity;
                    fish.rigidbody.acceleration = fish.rigidbody.acceleration + steering;
                    for (auto it = foods.begin(); it != foods.end(); ++it)
                    {
                        const Food& food = *it;
                        if (CheckCollisionCircleRec(fish.rigidbody.position, fish.width * 0.5f, { food.position.x - food.radius, food.position.y - food.radius, food.radius * 2, food.radius * 2 }))
                        {
                          foods.erase(it);
                          break;
                        }
                    }
                }
            }
            if (Avoid)
            {
             targetPosition = GetMousePosition();
              Vector2 desiredVel = targetPosition - fish.rigidbody.position;
              desiredVel = Normalize(desiredVel) * fish.maxSpeed;
              Vector2 steering = desiredVel - fish.rigidbody.velocity;
              fish.rigidbody.acceleration = fish.rigidbody.acceleration + steering;

                for (const Obstacle& obstacle : obstacles)
                {
                    Vector2 desiredVel = fish.rigidbody.position - obstacle.position;
                    float distance = Length(desiredVel);
                    float avoidRadius = 50.0f;

                    if (distance < avoidRadius)
                    {
                        desiredVel = Normalize(desiredVel) * fish.maxSpeed;
                        Vector2 steering = desiredVel - fish.rigidbody.velocity;
                        fish.rigidbody.acceleration = fish.rigidbody.acceleration + steering;
                    }
                }
            }
            fish.UpdateRigidBody(deltaTime);
            if (fish.rigidbody.position.x > SCREEN_WIDTH)
                fish.rigidbody.position.x = 0;
            else if (fish.rigidbody.position.x < 0)
                fish.rigidbody.position.x = SCREEN_WIDTH;

            if (fish.rigidbody.position.y > SCREEN_HEIGHT)
                fish.rigidbody.position.y = 0;
            else if (fish.rigidbody.position.y < 0)
                fish.rigidbody.position.y = SCREEN_HEIGHT;
        }
        DrawTexture(Background, 0, 0, RAYWHITE);
        for (const Fish& fish : fish)
        {
            fish.Draw();
        }
        if (Seek || Avoid)
        {
            DrawCircle(targetPosition.x, targetPosition.y, 10, ORANGE);
        }
        if (Avoid)
        {
            for (const Obstacle& obstacle : obstacles)
            {
                DrawCircle(obstacle.position.x, obstacle.position.y, obstacle.radius, YELLOW);
            }
        }
        if (Arrival)
        {
            for (const Food& food : foods)
            {
                DrawCircle(food.position.x, food.position.y, food.radius, GREEN);
            }
        }
        DrawText("Press 1 for Seek", 10, 10, 20, WHITE);
        DrawText("Press 2 for Flee", 200, 10, 20, WHITE);
        DrawText("Press 3 for Arrive", 400, 10, 20, WHITE);
        DrawText("Press 4 for Avoid", 600, 10, 20, WHITE);
        DrawText("Press SPACE to reset", 800, 10, 20, WHITE);


        EndDrawing();
    }
    UnloadTexture(Background);
    UnloadTexture(fishTexture);
    CloseWindow();
    return 0;
}
