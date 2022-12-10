#pragma once

#include "GilbertJohnsonKeerthi.h"

class ExpandingPolytopeAlgorithm
{
public:
	float tolerance = 0.0001f;

	auto FindClosestEdge(const std::vector<Vec2>& simplex) const
	{
		struct
		{
			float distToOrigin = std::numeric_limits<float>::max();
			Vec2 normal;
			int closestEdgeIndex;
		} edgeInSimplexClosestToOrigin;

		for (int i = 0; i < simplex.size(); i++)
		{
			int nextIndex;
			if (i + 1 >= simplex.size())
				nextIndex = 0;
			else 
				nextIndex = i + 1;

			Vec2 p1 = simplex[i];
			Vec2 p2 = simplex[nextIndex];

			Vec2 edge = p2 - p1;
			
			//Vec2 n = Vec2::TripleProduct(edge, p1, edge);
			Vec2 n = edge.GetNormal();

			//if (Vec2::Dot(p1, n) < 0)
			//{
			//	n *= -1;
			//}
			n.Normalize();

			double distToOrigin = Vec2::Dot(n, p1);
			if (distToOrigin < edgeInSimplexClosestToOrigin.distToOrigin)
			{
				edgeInSimplexClosestToOrigin.distToOrigin = distToOrigin;
				edgeInSimplexClosestToOrigin.normal = n;
				edgeInSimplexClosestToOrigin.closestEdgeIndex = i;
			}

		}

		return edgeInSimplexClosestToOrigin;
	}

	bool CheckCollision(const CPolygon& poly1, const CPolygon& poly2, Vec2& colPoint, Vec2& colNormal, float& colDist)
	{
		GilbertJohnsonKeerthi gjk;
		if (!gjk.CheckCollision(poly1, poly2))
			return false;

		std::vector<Vec2> simplex;
		simplex.reserve(8);
		simplex.push_back(gjk.builder.lastTriangle.a);
		simplex.push_back(gjk.builder.lastTriangle.b);
		simplex.push_back(gjk.builder.lastTriangle.c);

		while (true)
		{
			auto closestEdge = FindClosestEdge(simplex);

			std::vector<Vec2>::const_iterator farthestIt1;
			std::vector<Vec2>::const_iterator farthestIt2;
			Vec2 newPoint = gjk.Support(poly1, poly2, closestEdge.normal, farthestIt1, farthestIt2);

			double distToOrigin = Vec2::Dot(newPoint, closestEdge.normal);

			if (distToOrigin - closestEdge.distToOrigin < tolerance)
			{
				colNormal = closestEdge.normal;
				colDist = distToOrigin;

				// The collision vertex is either farthestIt1 or farthestIt2
				// We know a vertex is colliding with an edge
				// We also know colNormal is the normal of the edge
				// So if the edge is from poly1, the vertex is from poly2
				// and vice versa

				// This is not the most optimal way to get the collision point,
				// but this code is only ran 1 time per collision, 
				// so it is not that bad

				std::vector<Vec2>::const_iterator nextIt1 = farthestIt1 + 1;
				if (nextIt1 == poly1.transformedPoints.cend())
					nextIt1 = poly1.transformedPoints.cbegin();

				std::vector<Vec2>::const_iterator prevIt1 = farthestIt1;
				if (prevIt1 == poly1.transformedPoints.cbegin())
					prevIt1 = poly1.transformedPoints.cend() - 1;
				else
					prevIt1--;

				auto isTouchingEdge = [&colNormal](const Vec2& edge)
				{
					return abs(Vec2::Dot(edge, colNormal)) < 0.0001;
				};

				if (isTouchingEdge(*farthestIt1 - *nextIt1) || isTouchingEdge(*prevIt1 - *farthestIt1))
				{
					colPoint = *farthestIt2;
				}
				else
				{
					colPoint = *farthestIt1;
				}

				return true;
			}

			if (abs(closestEdge.distToOrigin) < tolerance)
			{

				return true;
			}

			simplex.insert(simplex.begin() + closestEdge.closestEdgeIndex + 1, newPoint);
		}

		return true;
	}
};


using EPA = ExpandingPolytopeAlgorithm;