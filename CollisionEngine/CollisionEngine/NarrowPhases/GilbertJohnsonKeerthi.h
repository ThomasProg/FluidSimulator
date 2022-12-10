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

	bool TryGetEdgeInPointDirection(Vec2 pTarget, Vec2& p1, Vec2& p2, Vec2& n) const
	{
		const Vec2& a = lastTriangle.a;
		const Vec2& b = lastTriangle.b;
		const Vec2& c = lastTriangle.c;

		Vec2 ac = c - a;
		Vec2 ab = b - a;
		Vec2 ap = pTarget - a;

		// Try to make a triangle on AB side
		Vec2 nAB = Vec2::TripleProduct(ac, ab, ab);
		if (Vec2::Dot(nAB, ap) > 0)
		{
			p1 = a;
			p2 = b;
			n = nAB;
			return true;
		}

		// Try to make a triangle on AC side
		Vec2 nAC = Vec2::TripleProduct(ab, ac, ac);
		if (Vec2::Dot(nAC, ap) > 0)
		{
			p1 = a;
			p2 = c;
			n = nAC;
			return true;
		}

		// We know the previous triangle was on BC side, 
		// so we already know pTarget isn't there

		// If it isn't on any edge directions, 
		// then we can't make any new triangle.
		// It also means the point is inside.
		return false;
	}
};

class GilbertJohnsonKeerthi
{
public:
	SimplexBuilder builder;

	static Vec2 Support(const CPolygon& poly1, const CPolygon& poly2, const Vec2& direction, std::vector<Vec2>::const_iterator& farthestP1, std::vector<Vec2>::const_iterator& farthestP2)
	{
		farthestP1 = GetFarthestPointItInDirectionWithConvexShape(poly1.transformedPoints, direction);
		farthestP2 = GetFarthestPointItInDirectionWithConvexShape(poly2.transformedPoints, -direction);

		return *farthestP1 - *farthestP2;
	}

	static Vec2 Support(const CPolygon& poly1, const CPolygon& poly2, const Vec2& direction)
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
			if (!builder.TryGetEdgeInPointDirection(Vec2(0, 0), newTriangle.b, newTriangle.c, direction))
				return true;

			newTriangle.a = Support(poly1, poly2, direction);
			builder.simplex.push_back(newTriangle.a);
			builder.lastTriangle = newTriangle;
		}
		return false;
	}
};

using GJK = GilbertJohnsonKeerthi;