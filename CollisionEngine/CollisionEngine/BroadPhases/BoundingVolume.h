#ifndef _BOUNDING_VOLUME_
#define _BOUNDING_VOLUME_

#include "BroadPhase.h"

#include "Conversion.h"
#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include <unordered_map>
#include "Renderer.h"

template<class SHAPE, typename CONVERTOR>
class CBoundingVolume : public IBroadPhase
{
public:
	std::unordered_map<CPolygonPtr, SHAPE> boundingVolumes;
	std::vector<CPolygonPtr>& polygonsRef;

	CBoundingVolume(std::vector<CPolygonPtr>& polygons) : polygonsRef(polygons)
	{

	}

	virtual void OnObjectAdded(const CPolygonPtr& polygon)
	{
		boundingVolumes.emplace(polygon, CONVERTOR::PolyToShape(polygon));

		polygon->onTransformUpdatedCallback = ([this, polygon](const CPolygon& poly)
		{
			CONVERTOR::UpdateShapeFromPoly(polygon, boundingVolumes[polygon]);
		});
	}

	virtual void OnObjectRemoved(const CPolygonPtr& polygon)
	{
		boundingVolumes.erase(polygon);
		polygon->onTransformUpdatedCallback = nullptr;
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		if (gVars->bDebug)
			DisplayDebug();

		for (size_t i = 0; i < polygonsRef.size(); ++i)
		{
			const SHAPE& c1 = boundingVolumes[polygonsRef[i]];
			for (size_t j = i + 1; j < polygonsRef.size(); ++j)
			{
				const SHAPE& c2 = boundingVolumes[polygonsRef[j]];
				if (CONVERTOR::AreColliding(c1, c2))
				{
					pairsToCheck.push_back(SPolygonPair(polygonsRef[i], polygonsRef[j]));
				}
			}
		}
	}

	void DisplayDebug()
	{
		gVars->pRenderer->DisplayText("BoundingVolume");
		for (const auto& debugShape : boundingVolumes)
		{
			CONVERTOR::DisplayDebugShape(debugShape.second);
		}
	}
};

#pragma region CCircleToCircle

class PolyToCircle
{
public:
	static Circle PolyToShape(const CPolygonPtr& poly)
	{
		return Circle(poly->Getposition(), poly->points);
	}

	static void UpdateShapeFromPoly(const CPolygonPtr& poly, Circle& circle)
	{
		circle.point = poly->Getposition();
	}

	static bool AreColliding(const Circle& a, const Circle& b)
	{
		return a.IsColliding(b);
	}

	static void DisplayDebugShape(const Circle& circle)
	{
		gVars->pRenderer->DrawCircle(circle.point, circle.radius, 50, 1, 0, 0);
	}
};

using CCircleToCircle = CBoundingVolume<Circle, PolyToCircle>;

#pragma endregion

#pragma region CAABBToAABB

// Used as an interface for BoundingVolume
class PolyToMoveableAABWrapper
{
public:
	static MoveableAABB PolyToShape(const CPolygonPtr& poly)
	{
		return PolyToMoveableAABB(poly);
	}

	static void UpdateShapeFromPoly(const CPolygonPtr& poly, MoveableAABB& aabb)
	{
		UpdateAABBTransformFromPolygon(aabb, *poly);
	}

	static bool AreColliding(const MoveableAABB& a, const MoveableAABB& b)
	{
		return a.GetmovedAABB().CheckCollision(b.GetmovedAABB());
	}

	static void DisplayDebugShape(const MoveableAABB& aabb)
	{
		gVars->pRenderer->DrawAABB(aabb.GetmovedAABB(), 1, 0, 0);
	}
};

using CAABBToAABB = CBoundingVolume<MoveableAABB, PolyToMoveableAABWrapper>;
#pragma endregion

#endif