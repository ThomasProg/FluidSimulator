#pragma once

#include "Polygon.h"

inline AABB PointsToBaseAABB(const std::vector<Vec2>& points)
{
	// TODO : asssert instead?
	if (points.size() == 0)
		return AABB();

	AABB baseAABB = AABB(points[0], points[0]);

	for (const Vec2& p : points)
	{
		// TODO : remove first iteration?
		baseAABB.pMin.x = Min(baseAABB.pMin.x, p.x);
		baseAABB.pMin.y = Min(baseAABB.pMin.y, p.y);
		baseAABB.pMax.x = Max(baseAABB.pMax.x, p.x);
		baseAABB.pMax.y = Max(baseAABB.pMax.y, p.y);
	}

	return baseAABB;
}

inline AABB PolyToBaseAABB(const CPolygonPtr& poly)
{
	return PointsToBaseAABB(poly->points);
}

inline AABB TransformAABB(AABB baseAABB, const Vec2& pos, const Mat2& rot)
{
	MoveableAABB moveableAABB;
	moveableAABB.SetbaseAABB(baseAABB);
	moveableAABB.SetTransform(pos, rot);
	return moveableAABB.GetmovedAABB();
}

inline AABB PolyToTransformedAABB(const CPolygonPtr& poly, AABB baseAABB)
{
	return TransformAABB(baseAABB, poly->Getposition(), poly->Getrotation());
}

inline void UpdateAABBTransformFromPolygon(MoveableAABB& aabb, const CPolygon& poly)
{
	// arbitrary number
	int nbPointsToOpti = 8;
	if (poly.points.size() > nbPointsToOpti)
		aabb.SetTransform(poly.Getposition(), poly.Getrotation());
	else
		aabb.SetMovedAABB(PointsToBaseAABB(poly.GetWorldPoints()));
}

inline MoveableAABB PolyToMoveableAABB(const CPolygonPtr& poly)
{
	MoveableAABB moveableAABB;
	moveableAABB.SetbaseAABB(PolyToBaseAABB(poly));
	UpdateAABBTransformFromPolygon(moveableAABB, *poly);
	return moveableAABB;
}

