#ifndef _CIRCLE_TO_CIRCLE_
#define _CIRCLE_TO_CIRCLE_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"

class CCircleToCircle : public IBroadPhase
{
public:
	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		AddCollidingPairsToCheck(gVars->pWorld->GetPolygons(), pairsToCheck);
	}

	static void AddCollidingPairsToCheck(const std::vector<CPolygonPtr>& polygonsToCheck, std::vector<SPolygonPair>& pairsToCheck)
	{
		std::vector<Circle> boundingCircles(polygonsToCheck.size());
		for (size_t i = 0; i < polygonsToCheck.size(); ++i)
		{
			CPolygonPtr poly = polygonsToCheck[i];
			boundingCircles.emplace_back(poly->Getposition(), poly->points);
		}

		for (size_t i = 0; i < polygonsToCheck.size(); ++i)
		{
			for (size_t j = i + 1; j < polygonsToCheck.size(); ++j)
			{
				if (boundingCircles[i].CheckCollision(boundingCircles[j]))
				{
					pairsToCheck.push_back(SPolygonPair(polygonsToCheck[i], polygonsToCheck[j]));
				}
			}
		}
	}
};

#endif