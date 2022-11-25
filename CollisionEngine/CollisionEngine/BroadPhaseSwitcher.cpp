#include "BroadPhaseSwitcher.h"

#include "BroadPhases/BroadPhaseBrut.h"
#include "BroadPhases/BoundingVolume.h"
#include "BroadPhases/AABBTree.h"
#include "BroadPhases/Grid.h"
#include "BroadPhases/SweepAndPrune.h"
#include "BroadPhases/QuadTree.h"

BroadPhaseSwitcher::BroadPhaseSwitcher(std::vector<CPolygonPtr>& polygons) : m_polygons(polygons)
{
	broadPhasesGetters.emplace_back([&polygons]() -> std::unique_ptr<IBroadPhase> { return std::make_unique<CBroadPhaseBrut>(polygons); });
	broadPhasesGetters.emplace_back([&polygons]() -> std::unique_ptr<IBroadPhase> { return std::make_unique<CGrid>(2); });
	broadPhasesGetters.emplace_back([&polygons]() -> std::unique_ptr<IBroadPhase> { return std::make_unique<CCircleToCircle>(polygons); });
	broadPhasesGetters.emplace_back([&polygons]() -> std::unique_ptr<IBroadPhase> { return std::make_unique<CAABBToAABB>(polygons); });
	broadPhasesGetters.emplace_back([&polygons]() -> std::unique_ptr<IBroadPhase> { return std::make_unique<CSweepAndPrune>(); });
	broadPhasesGetters.emplace_back([&polygons]() -> std::unique_ptr<IBroadPhase> { return std::make_unique<CQuadTree>(); });
}

void BroadPhaseSwitcher::Reset()
{
	m_broadPhase = broadPhasesGetters[broadPhaseID]();
}

void BroadPhaseSwitcher::NextBroadPhase()
{
	broadPhaseID = (broadPhaseID + 1 + broadPhasesGetters.size()) % broadPhasesGetters.size();
	SetBroadPhase(broadPhasesGetters[broadPhaseID]());
}
void BroadPhaseSwitcher::PreviousBroadPhase()
{
	broadPhaseID = (broadPhaseID - 1 + broadPhasesGetters.size()) % broadPhasesGetters.size();
	SetBroadPhase(broadPhasesGetters[broadPhaseID]());
}


void BroadPhaseSwitcher::SetBroadPhase(std::unique_ptr<IBroadPhase>&& newBroadPhase)
{
	std::vector<CPolygonPtr> oldPolygons(m_polygons);

	if (m_broadPhase != nullptr)
	{
		for (CPolygonPtr& poly : oldPolygons)
		{
			m_polygons.erase(std::remove(m_polygons.begin(), m_polygons.end(), poly), m_polygons.end());
			m_broadPhase->OnObjectRemoved(poly);
		}
	}

	m_broadPhase = std::move(newBroadPhase);

	for (CPolygonPtr& poly : oldPolygons)
	{
		m_broadPhase->OnObjectAdded(poly);
		m_polygons.push_back(poly);
	}
}