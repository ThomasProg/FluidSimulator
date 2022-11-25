#ifndef _SWEEP_AND_PRUNE_H_
#define _SWEEP_AND_PRUNE_H_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include <algorithm>

class CSweepAndPrune : public IBroadPhase
{
	class PolygonData
	{
		CPolygonPtr polygon;
		MoveableAABB moveableAABB;

	private:
		bool IsOverlappingOnXAxis(const PolygonData& polyData) const
		{
			return moveableAABB.GetmovedAABB().pMin.x < polyData.moveableAABB.GetmovedAABB().pMax.x
				&& moveableAABB.GetmovedAABB().pMax.x > polyData.moveableAABB.GetmovedAABB().pMin.x;
		}

		bool IsOverlappingOnYAxis(const PolygonData& polyData) const
		{
			return moveableAABB.GetmovedAABB().pMin.y < polyData.moveableAABB.GetmovedAABB().pMax.y
				&& moveableAABB.GetmovedAABB().pMax.y > polyData.moveableAABB.GetmovedAABB().pMin.y;
		}

	public:
		PolygonData(const CPolygonPtr& poly) : polygon(poly), moveableAABB(PolyToMoveableAABB(poly))
		{
			MoveableAABB& aabb = moveableAABB;
			poly->onTransformUpdatedCallback = [&aabb, this](const CPolygon& poly)
			{
				UpdateAABBTransformFromPolygon(aabb, poly);
				UpdateAxisProjections();
			};
			UpdateAxisProjections();
		}

		PolygonData(PolygonData&& rhs) noexcept = default;

		PolygonData& operator=(PolygonData&& rhs) = default;

		void UpdateAxisProjections()
		{
			// in case we have to project on an axis that is not (0,1) or (1,0),
			// we can precompute projections here
		}

		bool IsOverlapppingOnAxis(const PolygonData& polyData) const
		{
			// Compares on different axis
			return IsOverlappingOnXAxis(polyData) && IsOverlappingOnYAxis(polyData); // currently comparing aabb
		}

		float GetAxisSortValueMin() const
		{
			return moveableAABB.GetmovedAABB().pMin.x;
		}


		float GetAxisSortValueMax() const
		{
			return moveableAABB.GetmovedAABB().pMax.x;
		}

		GETTER(polygon)
	};
	std::vector<std::unique_ptr<PolygonData>> polygons;

public:
	virtual void OnObjectAdded(const CPolygonPtr& polygon) 
	{
		polygons.emplace_back(std::make_unique<PolygonData>(polygon));
	}

	virtual void OnObjectRemoved(const CPolygonPtr& polygon) 
	{
		std::remove_if(polygons.begin(), polygons.end(), [&polygon](const std::unique_ptr<PolygonData>& polyData)
		{
			return polygon == polyData->Getpolygon();
		});
		polygon->onTransformUpdatedCallback = nullptr;
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		if (gVars->bDebug)
			DisplayDebug();

		std::sort(polygons.begin(), polygons.end(), [](const std::unique_ptr<PolygonData>& p1, const std::unique_ptr<PolygonData>& p2) -> bool
		{
			return p1->GetAxisSortValueMin() < p2->GetAxisSortValueMin();
		});

		std::vector<PolygonData*> activeIntervals;

		for (const std::unique_ptr<PolygonData>& poly1 : polygons)
		{
			auto it = activeIntervals.begin();
			while (it != activeIntervals.end())
			{
				PolygonData* poly2 = *it;
				if (poly1->GetAxisSortValueMin() > poly2->GetAxisSortValueMax())
				{
					it = activeIntervals.erase(it);
				}
				else
				{
					pairsToCheck.push_back(SPolygonPair(poly1->Getpolygon(), poly2->Getpolygon()));
					it++;
				}
			}
			activeIntervals.push_back(poly1.get());
		}
	}

	void DisplayDebug()
	{
		gVars->pRenderer->DisplayText("Sweep And Prune");

		gVars->pRenderer->DrawLine(Vec2(-50, 0), Vec2(50, 0), 1,1,0);

		for (const std::unique_ptr<PolygonData>& poly1 : polygons)
		{
			float minVal = poly1->GetAxisSortValueMin();
			float maxVal = poly1->GetAxisSortValueMax();

			gVars->pRenderer->DrawLine(Vec2(minVal, poly1->Getpolygon()->Getposition().y), Vec2(minVal, 0), 1, 0, 0);
			gVars->pRenderer->DrawLine(Vec2(maxVal, poly1->Getpolygon()->Getposition().y), Vec2(maxVal, 0), 1, 0, 0);
		}
	}
};

#endif