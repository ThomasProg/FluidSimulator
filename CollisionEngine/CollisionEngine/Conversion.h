#pragma once

#include "Polygon.h"

inline AABB PolyToBaseAABB(const CPolygonPtr& poly)
{
	if (poly->points.size() == 0)
		return AABB();

	AABB baseAABB = AABB(poly->points[0], poly->points[0]);

	for (const Vec2& p : poly->points)
	{
		// TODO : remove first iteration?
		baseAABB.pMin.x = Min(baseAABB.pMin.x, p.x);
		baseAABB.pMin.y = Min(baseAABB.pMin.y, p.y);
		baseAABB.pMax.x = Max(baseAABB.pMax.x, p.x);
		baseAABB.pMax.y = Max(baseAABB.pMax.y, p.y);
	}

	return baseAABB;
}

inline AABB TransformAABB(AABB baseAABB, const Vec2& pos)
{
	baseAABB.pMin += pos;
	baseAABB.pMax += pos;
	return baseAABB;
}

inline AABB PolyToTransformedAABB(const CPolygonPtr& poly, AABB baseAABB)
{
	return TransformAABB(baseAABB, poly->Getposition());
}

inline void UpdateAABBTransformFromPolygon(MoveableAABB& aabb, const CPolygon& poly)
{
	aabb.SetTransform(poly.Getposition(), poly.Getrotation());
}

inline MoveableAABB PolyToMoveableAABB(const CPolygonPtr& poly)
{
	MoveableAABB moveableAABB;
	moveableAABB.SetbaseAABB(PolyToBaseAABB(poly));
	UpdateAABBTransformFromPolygon(moveableAABB, *poly);
	return moveableAABB;
}

