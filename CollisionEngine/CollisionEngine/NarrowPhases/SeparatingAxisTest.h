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
		std::vector<Vec2> allAxis;
		allAxis.reserve(poly1.transformedPoints.size() + poly2.transformedPoints.size());
		
		GetAllNormals(poly2, out allAxis);
		GetAllNormals(poly1, out allAxis);
		
		float smallestOverlap = std::numeric_limits<float>::max();
		Vec2 axisWithSmallestOverlap;

		for (Vec2 axis : allAxis)
		{
			axis.Normalize();

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
					// To prevent bugs when multiple edges with the same normal axis exist
					float v = Vec2::Dot(poly1.Getposition() - poly2.Getposition(), axis);
					if (v > 0)
						axis *= -1;

					smallestOverlap = overlap;
					axisWithSmallestOverlap = axis;
				}
			}
		}

		colDist = smallestOverlap;
		colNormal = axisWithSmallestOverlap.Normalized();

		std::vector<Vec2> outPointsPoly1;
		GetFarthestPoinstInDirection(poly1.transformedPoints, colNormal, outPointsPoly1);

		std::vector<Vec2> outPointsPoly2;
		GetFarthestPoinstInDirection(poly2.transformedPoints, -colNormal, outPointsPoly2);

		if (outPointsPoly1.size() == 1)
		{
			colPoint = outPointsPoly1[0];
		}
		else if (outPointsPoly2.size() == 1)
		{
			colPoint = outPointsPoly2[0] + colNormal * colDist;
		}
		else // if 2 edges are colliding
		{
			Vec2 n = colNormal.GetNormal();

			Vec2 minPoint1n;
			Vec2 maxPoint1n;
			float minp1n, maxp1n;
			ProjectPolygonOnAxis(poly1, n, minp1n, maxp1n, minPoint1n, maxPoint1n);

			Vec2 minPoint2n;
			Vec2 maxPoint2n;
			float minp2n, maxp2n;
			ProjectPolygonOnAxis(poly2, n, minp2n, maxp2n, minPoint2n, maxPoint2n);

			Vec2 minPoint1;
			Vec2 maxPoint1;
			float minp1, maxp1;
			ProjectPolygonOnAxis(poly1, colNormal, minp1, maxp1, minPoint1, maxPoint1);

			Vec2 minPoint2;
			Vec2 maxPoint2;
			float minp2, maxp2;
			ProjectPolygonOnAxis(poly2, colNormal, minp2, maxp2, minPoint2, maxPoint2);

			float minMaxProjn = Min(maxp1n, maxp2n);
			float maxMinProjn = Max(minp1n, minp2n);
			float middlen = maxMinProjn + (minMaxProjn - maxMinProjn) / 2;

			float minMaxProj = Min(maxp1, maxp2);
			float maxMinProj = Max(minp1, minp2);
			float middle = maxMinProj + (minMaxProj - maxMinProj) / 2;
			colPoint = n * middlen + colNormal * middle;
		}

		return true;
	}
};

using SAT = SeparatingAxisTest;