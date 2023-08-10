#pragma once
#include "Geometry.h"

// ***MTV resolves circle2 from circle1***
inline bool CircleCircle(const Circle& circle1, const Circle& circle2, Vector2* mtv = nullptr)
{
    Vector2 delta = circle2.position - circle1.position;
    float radiiSum = circle1.radius + circle2.radius;
    float centerDistance = Length(delta);
    bool collision = centerDistance <= radiiSum;
    if (collision && mtv != nullptr)
    {
        // Compute mtv (sum of radii - distance between centers)
        float mtvDistance = radiiSum - centerDistance;
        *mtv = Normalize(delta) * mtvDistance;
    }
    return collision;
}

// Resolves position from circles
inline void ResolveCircles(Vector2& position, float radius, const Circles& circles)
{
    for (const Circle& circle : circles)
    {
        Vector2 mtv{};
        CircleCircle(circle, { position, radius }, &mtv);
        position = position + mtv;
    }
}

// Line-Circle intersection
inline bool LineCircle(Vector2 lineStart, Vector2 lineEnd, Vector2 circlePosition, float circleRadius)
{
    Vector2 nearest = ProjectPointLine(lineStart, lineEnd, circlePosition);
    return DistanceSqr(nearest, circlePosition) <= circleRadius * circleRadius;
}

// Line-Circle intersecion test for all input circles. Writes intersecting circles to out
inline void LineCircleList(Vector2 lineStart, Vector2 lineEnd, const Circles& in, Circles& out)
{
    out.reserve(in.size());
    for (const Circle& obstacle : in)
    {
        if (LineCircle(lineStart, lineEnd, obstacle.position, obstacle.radius))
            out.push_back(obstacle);
    }
    // Better to explicitly distance-sort so we don't waste CPU cyclines incase we can get by with an unsorted list
    //DistanceSort(lineStart, out);
}

// Line-of-sight test for all input circles (line-circle intersection + occlusion test)
inline bool IsVisible(Vector2 viewer, float viewDistance, Vector2 target, float targetRadius, const Circles& circles)
{
    Vector2 viewerEnd = viewer + Normalize(target - viewer) * viewDistance;
    if (LineCircle(viewer, viewerEnd, target, targetRadius))
    {
        Circles intersections;
        LineCircleList(viewer, viewerEnd, circles, intersections);

        if (intersections.empty())
            return true;

        DistanceSort(viewer, intersections);
        return DistanceSqr(viewer, target) < DistanceSqr(viewer, intersections.front().position);
    }
    return false;
}
