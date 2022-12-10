#pragma once

#include "Polygon.h"
#include <cassert>

#undef max
#define out

class SeparatingAxisTest
{
public:
	Vec2 GetNormalBetween2Points(Vec2 p1, Vec2 p2) const
	{
		return (p1 - p2).GetNormal();
	}

	void GetAllNormals(const CPolygon& poly, out std::vector<Vec2>& outNormals) const
	{
		assert(poly.transformedPoints.size() > 1);

		for (int i = 0; i < poly.transformedPoints.size() - 1; i++)
		{
			outNormals.push_back(GetNormalBetween2Points(poly.transformedPoints[i], poly.transformedPoints[i+1]));
		}

		outNormals.push_back(GetNormalBetween2Points(poly.transformedPoints[poly.transformedPoints.size() - 1], poly.transformedPoints[0]));
	}

	void ProjectPolygonOnAxis(const CPolygon& poly, Vec2 axis, out float& min, out float& max, out Vec2& minPoint, out Vec2& maxPoint) const
	{
		min = Vec2::Dot(poly.transformedPoints[0], axis);
		max = min;
		minPoint = poly.transformedPoints[0];
		maxPoint = minPoint;
		for (Vec2 p : poly.transformedPoints)
		{
			float v = Vec2::Dot(p, axis);
			if (v < min)
			{
				min = v;
				minPoint = p;
			}
			if (v > max)
			{
				max = v;
				maxPoint = p;
			}
		}

	}

	bool CheckCollision(const CPolygon& poly1, const CPolygon& poly2, Vec2& colPoint, Vec2& colNormal, float& colDist) 
	{
		colPoint = Vec2(100, 100);

		std::vector<Vec2> allAxis;
		allAxis.reserve(poly1.transformedPoints.size() + poly2.transformedPoints.size());
		
		GetAllNormals(poly2, out allAxis);
		for (Vec2& axis : allAxis)
		{
			axis *= -1;
		}
		int poly1StartID = allAxis.size();
		GetAllNormals(poly1, out allAxis);
		
		float smallestOverlap = std::numeric_limits<float>::max();
		Vec2 axisWithSmallestOverlap;

		for (int i = 0; i < allAxis.size(); i++)
		//for (Vec2 axis : allAxis)
		{
			Vec2 axis = allAxis[i];
			Vec2 minPoint1;
			Vec2 maxPoint1;
			float minp1, maxp1;
			ProjectPolygonOnAxis(poly1, axis, minp1, maxp1, minPoint1, maxPoint1);

			Vec2 minPoint2;
			Vec2 maxPoint2;
			float minp2, maxp2;
			ProjectPolygonOnAxis(poly2, axis, minp2, maxp2, minPoint2, maxPoint2);

			if (!(maxp1 > minp2 && maxp2 > minp1))
			{
				return false;
			}
			else 
			{
				float minMaxProj = Min(maxp1, maxp2);
				float maxMinProj = Max(minp1, minp2);
				float overlap = minMaxProj - maxMinProj;
				if (overlap < smallestOverlap)
				{
					Vec2 collisionPoint;
					if (i < poly1StartID)
					{
						collisionPoint = maxPoint1;
					}
					else
					{
						collisionPoint = minPoint2;
					}

					// To prevent bugs when multiple edges with the same normal axis exist
					float v = Vec2::Dot(collisionPoint, axis);
					if (v <= minMaxProj && v >= maxMinProj)
					{
						smallestOverlap = overlap;
						axisWithSmallestOverlap = axis;
						colPoint = collisionPoint;
					}
				}
			}
		}

		if (Vec2::Dot(poly1.speed, axisWithSmallestOverlap) > 0)
			axisWithSmallestOverlap *= -1;

		colDist = smallestOverlap;
		colNormal = axisWithSmallestOverlap.Normalized();
		// TODO : set point

		return true;
	}
};

using SAT = SeparatingAxisTest;