#ifndef _BROAD_PHASE_BRUT_H_
#define _BROAD_PHASE_BRUT_H_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"
#include "Renderer.h"

class CBroadPhaseBrut : public IBroadPhase
{
	std::vector<CPolygonPtr>& registeredPolygons;
	std::vector<SPolygonPair> pairsToCheck;

public:
	CBroadPhaseBrut(std::vector<CPolygonPtr>& polygons) : registeredPolygons(polygons)
	{

	}

	virtual void OnObjectAdded(const CPolygonPtr& newPolygon) override
	{
		for (const CPolygonPtr& registeredPoly : registeredPolygons)
		{
			pairsToCheck.push_back(SPolygonPair(registeredPoly, newPolygon));
		}
	}

	virtual void OnObjectRemoved(const CPolygonPtr& polygon) override
	{
		pairsToCheck.erase(std::remove_if(pairsToCheck.begin(), pairsToCheck.end(), [polygon](const SPolygonPair& pair)
			{
				return pair.GetpolyA() == polygon || pair.GetpolyB() == polygon;
			}),
			pairsToCheck.end()
		);
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		gVars->pRenderer->DisplayText("Brut");
		pairsToCheck = this->pairsToCheck;
	}

	static void AddCollidingPairsToCheck(const std::vector<CPolygonPtr>& polygonsToCheck, std::vector<SPolygonPair>& pairsToCheck)
	{
		pairsToCheck.reserve(polygonsToCheck.size() * polygonsToCheck.size()); // ideal sizee : n * log(n), but log long to compute
		size_t size = polygonsToCheck.size();
		for (size_t i = 0; i < size; ++i)
		{
			for (size_t j = i + 1; j < size; ++j)
			{
				pairsToCheck.push_back(SPolygonPair(polygonsToCheck[i], polygonsToCheck[j]));
			}
		}
	}
};

#endif