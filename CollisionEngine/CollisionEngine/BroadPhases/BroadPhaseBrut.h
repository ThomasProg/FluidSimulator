#ifndef _BROAD_PHASE_BRUT_H_
#define _BROAD_PHASE_BRUT_H_

#include "BroadPhase.h"

#include "Polygon.h"
#include "GlobalVariables.h"
#include "World.h"

class CBroadPhaseBrut : public IBroadPhase
{
	std::vector<CPolygonPtr>& registeredPolygons;
	std::vector<SPolygonPair> pairsToCheck;

public:
	CBroadPhaseBrut(std::vector<CPolygonPtr>& polygons) : registeredPolygons(polygons)
	{

	}

	virtual void OnObjectAdded(CPolygonPtr newPolygon) override
	{
		for (CPolygonPtr registeredPoly : registeredPolygons)
		{
			pairsToCheck.push_back(SPolygonPair(registeredPoly, newPolygon));
		}
		registeredPolygons.push_back(newPolygon);
	}

	virtual void OnObjectRemoved(CPolygonPtr polygon) override
	{
		pairsToCheck.erase(std::remove_if(pairsToCheck.begin(), pairsToCheck.end(), [polygon](const SPolygonPair& pair)
			{
				return pair.GetpolyA() == polygon || pair.GetpolyB() == polygon;
			}),
			pairsToCheck.end()
		);

		registeredPolygons.erase(std::remove(registeredPolygons.begin(), registeredPolygons.end(), polygon), registeredPolygons.end());
	}

	virtual void OnObjectUpdated(CPolygonPtr polygon) override
	{
		
	}

	virtual void GetCollidingPairsToCheck(std::vector<SPolygonPair>& pairsToCheck) override
	{
		pairsToCheck = this->pairsToCheck;
		//AddCollidingPairsToCheck(gVars->pWorld->GetPolygons(), pairsToCheck);
	}

	static void AddCollidingPairsToCheck(const std::vector<CPolygonPtr>& polygonsToCheck, std::vector<SPolygonPair>& pairsToCheck)
	{
		pairsToCheck.reserve(polygonsToCheck.size() * polygonsToCheck.size()); // n * log(n)
		size_t size = polygonsToCheck.size();
		for (size_t i = 0; i < size; ++i)
		{
			for (size_t j = i + 1; j < size; ++j)
			{
				pairsToCheck.push_back(SPolygonPair(polygonsToCheck[i], polygonsToCheck[j]));
			}
		}
	}

	//static void GetCollidingPairsToCheck(const std::vector<CPolygonPtr>& polygonsToCheck, std::vector<SPolygonPair>& pairsToCheck)
	//{
	//	AddCollidingPairsToCheck(polygonsToCheck, pairsToCheck);
	//}
};

#endif