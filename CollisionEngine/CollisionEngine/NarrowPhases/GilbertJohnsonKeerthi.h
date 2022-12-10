#pragma once

#include "Polygon.h"
#include <cassert>

#undef max
#define out

struct SimplexBuilder
{
	std::vector<Vec2> simplex;

	// AB is the base edge, that should not be computed again
	Triangle lastTriangle;
};

class GilbertJohnsonKeerthi
{
public:
	Vec2 Support(const CPolygon& poly1, const CPolygon& poly2, const Vec2& direction)
	{
		Vec2 p1 = GetFarthestPointInDirectionWithConvexShape(poly1.transformedPoints, direction);
		Vec2 p2 = GetFarthestPointInDirectionWithConvexShape(poly2.transformedPoints, -direction);
		
		return p1 - p2;
	}

	static Vec2 GetNormalOfLineToOrigin(Vec2 a, Vec2 b)
	{
		Vec2 ab = b - a;

		// normal of ab in the direction of the origin
		return Vec2::TripleProduct(ab, -a, ab); 
	}

	bool CheckCollision(const CPolygon& poly1, const CPolygon& poly2)
	{
		// arbitrary direction
		Vec2 direction = Vec2(1, 0);

		SimplexBuilder builder;

		Vec2 p1 = Support(poly1, poly2, direction);
		builder.simplex.push_back(p1);
		Vec2 p2 = Support(poly1, poly2, -direction);
		builder.simplex.push_back(p2);

		direction = GetNormalOfLineToOrigin(p1, p2);
		Vec2 newSimplexPoint = Support(poly1, poly2, direction);
		builder.simplex.push_back(newSimplexPoint);

		builder.lastTriangle.a = newSimplexPoint;
		builder.lastTriangle.b = p1;
		builder.lastTriangle.c = p2;

		Triangle newTriangle;
		while (Vec2::Dot(builder.lastTriangle.a, direction) >= 0)
		{
			if (builder.lastTriangle.GetPointsOfClosestEdge(Vec2(0, 0), newTriangle.b, newTriangle.c, direction))
				return true;

			newTriangle.a = Support(poly1, poly2, direction);
			builder.simplex.push_back(newTriangle.a);
			builder.lastTriangle = newTriangle;
		}
		return false;
	}
};

using GJK = GilbertJohnsonKeerthi;