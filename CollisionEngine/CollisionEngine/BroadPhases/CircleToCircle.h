#ifndef _CIRCLE_TO_CIRCLE_
#define _CIRCLE_TO_CIRCLE_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include <unordered_map>

class CCircleToCircle : public IBroadPhase
{
public:
	std::unordered_map<CPolygonPtr, Circle> boundingCircles;
	std::vector<CPolygonPtr>& polygonsRef;

	CCircleToCircle(std::vector<CPolygonPtr>& polygons) : polygonsRef(polygons)
	{

	}

	virtual void OnObjectAdded(const CPolygonPtr& polygon)
	{
		boundingCircles.emplace(polygon, Circle(polygon->Getposition(), polygon->points));
	}

	virtual void OnObjectRemoved(const CPolygonPtr& polygon)
	{
		boundingCircles.erase(polygon);
	}

	virtual void OnObjectUpdated(const CPolygonPtr& polygon)
	{
		boundingCircles[polygon].point = polygon->Getposition();
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		for (size_t i = 0; i < polygonsRef.size(); ++i) 
		{
			const Circle& c1 = boundingCircles[polygonsRef[i]];
			for (size_t j = i + 1; j < polygonsRef.size(); ++j)
			{
				const Circle& c2 = boundingCircles[polygonsRef[j]];
				if (c1.IsColliding(c2))
				{
					pairsToCheck.push_back(SPolygonPair(polygonsRef[i], polygonsRef[j]));
				}
			}
		}
	}
};

#endif