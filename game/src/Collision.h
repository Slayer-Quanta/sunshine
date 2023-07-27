#pragma once
#include "Geometry.h"

// Line-Circle intersection
inline bool LineCircle(Vector2 lineStart, Vector2 lineEnd, Vector2 circlePosition, float circleRadius)
{
    Vector2 nearest = ProjectPointLine(lineStart, lineEnd, circlePosition);
    return DistanceSqr(nearest, circlePosition) <= circleRadius * circleRadius;
}

// Line-Circle intersecion test for all input circles. Writes intersecting circles in ascending order relative to line-start to out.
inline void LineCircleList(Vector2 lineStart, Vector2 lineEnd, const Circles& in, Circles& out)
{
    out.reserve(in.size());
    for (const Circle& obstacle : in)
    {
        if (LineCircle(lineStart, lineEnd, obstacle.position, obstacle.radius))
            out.push_back(obstacle);
    }
    DistanceSort(lineStart, out);
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

        return DistanceSqr(viewer, target) < DistanceSqr(viewer, intersections.front().position);
    }
    return false;
}
