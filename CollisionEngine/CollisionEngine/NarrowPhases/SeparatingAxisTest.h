#pragma once

#include "Polygon.h"
#include <cassert>

#undef max

class SeparatingAxisTest
{
public:
	static void ProjectPolygonOnAxis(const CPolygon& poly, Vec2 axis, float& min, float& max, Vec2& minPoint, Vec2& maxPoint) 
	{
		const std::vector<Vec2>& worldPoints = poly.GetWorldPoints();

		min = Vec2::Dot(worldPoints[0], axis);
		max = min;
		minPoint = worldPoints[0];
		maxPoint = minPoint;
		for (Vec2 p : worldPoints)
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

	static Vec2 GetCollisionPoint(const CPolygon& poly1, const CPolygon& poly2, Vec2 colNormal, float colDist)
	{
		static std::vector<Vec2> outPointsPoly1;
		outPointsPoly1.clear();
		GetFarthestPoinstInDirection(poly1.GetWorldPoints(), colNormal, outPointsPoly1);

		static std::vector<Vec2> outPointsPoly2;
		outPointsPoly2.clear();
		GetFarthestPoinstInDirection(poly2.GetWorldPoints(), -colNormal, outPointsPoly2);

		if (outPointsPoly1.size() == 1)
		{
			return outPointsPoly1[0];
		}
		else if (outPointsPoly2.size() == 1)
		{
			return outPointsPoly2[0] + colNormal * colDist;
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
			return n * middlen + colNormal * middle;
		}
	}

	bool CheckCollision(const CPolygon& poly1, const CPolygon& poly2, Vec2& colPoint, Vec2& colNormal, float& colDist) 
	{
		//std::vector<Vec2> allAxes;
		//allAxes.reserve(poly1.transformedPoints.size() + poly2.transformedPoints.size());
		
		//poly2.GetAllNormals(allAxes);
		//poly1.GetAllNormals(allAxes);
		//allAxes.push_back(Vec2(0,1));
		//allAxes.push_back(Vec2(1,0));
		
		float smallestOverlap = std::numeric_limits<float>::max();
		Vec2 axisWithSmallestOverlap;

		// Array of pointers to vectors
		const std::vector<Vec2>* allNormals[] = {&poly1.GetWorldNormals(), &poly2.GetWorldNormals()};

		for (const std::vector<Vec2>* normals : allNormals)
		{
			for (const Vec2 axis : *normals)
			{
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
						smallestOverlap = overlap;
						axisWithSmallestOverlap = axis;

						// To prevent bugs when multiple edges with the same normal axis exist
						float v = Vec2::Dot(poly1.Getposition() - poly2.Getposition(), axisWithSmallestOverlap);
						if (v > 0)
							axisWithSmallestOverlap *= -1;

					}
				}
			}
		}

		colDist = smallestOverlap;
		colNormal = axisWithSmallestOverlap;
		colPoint = GetCollisionPoint(poly1, poly2, colNormal, colDist);

		return true;
	}
};

using SAT = SeparatingAxisTest;

class QuickSeparatingAxisTest
{
public:

	bool CheckCollision(const CPolygon& poly1, const CPolygon& poly2, const Vec2& axis, Vec2& colPoint, float& colDist)
	{
		assert(axis.IsNormalized());

		Vec2 minPoint1;
		Vec2 maxPoint1;
		float minp1, maxp1;
		SeparatingAxisTest::ProjectPolygonOnAxis(poly1, axis, minp1, maxp1, minPoint1, maxPoint1);

		Vec2 minPoint2;
		Vec2 maxPoint2;
		float minp2, maxp2;
		SeparatingAxisTest::ProjectPolygonOnAxis(poly2, axis, minp2, maxp2, minPoint2, maxPoint2);

		if (!(maxp1 > minp2 && maxp2 > minp1))
		{
			return false;
		}
		else
		{
			float minMaxProj = Min(maxp1, maxp2);
			float maxMinProj = Max(minp1, minp2);
			float overlap = minMaxProj - maxMinProj;
			{
				colDist = overlap;
				colPoint = SeparatingAxisTest::GetCollisionPoint(poly1, poly2, axis, colDist);
			}
		}
	}
};


using QuickSAT = QuickSeparatingAxisTest;