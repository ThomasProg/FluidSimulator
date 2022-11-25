#pragma once

#include <vector>
#include <functional>
#include <memory>
#include "BroadPhase.h"

class BroadPhaseSwitcher
{
	int broadPhaseID = 0;
	std::unique_ptr<IBroadPhase> m_broadPhase;
	std::vector<CPolygonPtr>& m_polygons;

public:
	BroadPhaseSwitcher(std::vector<CPolygonPtr>& polygons);
	std::vector<std::function<std::unique_ptr<IBroadPhase>()>> broadPhasesGetters;

	void NextBroadPhase();
	void PreviousBroadPhase();
	void SetBroadPhase(std::unique_ptr<IBroadPhase>&& newBroadPhase);

	IBroadPhase* operator->()
	{
		return m_broadPhase.get();
	}

	void Reset();
};