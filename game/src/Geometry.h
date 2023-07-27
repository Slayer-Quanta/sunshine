#pragma once
#include "Math.h"
#include <vector>
#include <algorithm>

struct Line
{
    Vector2 start;
    Vector2 end;
};

struct Circle
{
    Vector2 position;
    float radius;
};

// Obstacle-avoidance sensor data
struct Probe
{
    float angle;    // degrees relative to seeker direction (rb.vel)
    float length;
};

using Points = std::vector<Vector2>;
using Lines = std::vector<Line>;
using Circles = std::vector<Circle>;
using Probes = std::vector<Probe>;

// Sorts points in ascending order based on distance to position
inline void DistanceSort(const Vector2& position, Points& points)
{
    std::sort(points.begin(), points.end(),
        [position](const Vector2& lhs, const Vector2& rhs)
        {
            return DistanceSqr(position, lhs) < DistanceSqr(position, rhs);
        }
    );
}

// Sorts obstacles in ascending order based on distance to position
inline void DistanceSort(const Vector2& position, Circles& circles)
{
    std::sort(circles.begin(), circles.end(),
        [position](const Circle& lhs, const Circle& rhs)
        {
            return DistanceSqr(position, lhs.position) < DistanceSqr(position, rhs.position);
        }
    );
}

// This is nice in terms of code re-use, however code generation should be avoided if you can instead use a reasonable amount of function overloading.
// T must have a Vector2 position member
//template<typename T>
//inline void DistanceSort(const Vector2& position, std::vector<T>& objects)
//{
//    std::sort(objects.begin(), objects.end(),
//        [position](const T& lhs, const T& rhs)
//        {
//            return DistanceSqr(position, lhs.position) < DistanceSqr(position, rhs.position);
//        }
//    );
//}
