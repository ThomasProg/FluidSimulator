#include "PhysicEngine.h"

#include <iostream>
#include <string>
#include "GlobalVariables.h"
#include "World.h"
#include "Renderer.h" // for debugging only
#include "Timer.h"

#include "BroadPhase.h"
#include "BroadPhases/BroadPhaseBrut.h"
#include "BroadPhases/CircleToCircle.h"
#include "BroadPhases/AABBTree.h"
#include "BroadPhases/Grid.h"
#include "BroadPhases/SweepAndPrune.h"


void	CPhysicEngine::Reset()
{
	m_pairsToCheck.clear();
	m_collidingPairs.clear();

	m_active = true;
	 
	m_broadPhase = new CGrid(5);
	//m_broadPhase = new CCircleToCircle(m_polygons);
	//m_broadPhase = new CBroadPhaseBrut(m_polygons);
}

void	CPhysicEngine::Activate(bool active)
{
	m_active = active;
}

void	CPhysicEngine::DetectCollisions()
{
	CTimer timer;
	timer.Start();
	CollisionBroadPhase();
	timer.Stop();
	if (gVars->bDebug)
	{
		gVars->pRenderer->DisplayText("Collision broadphase duration " + std::to_string(timer.GetDuration() * 1000.0f) + " ms");
	}

	timer.Start();
	CollisionNarrowPhase();
	timer.Stop();
	if (gVars->bDebug)
	{
		gVars->pRenderer->DisplayText("Collision narrowphase duration " + std::to_string(timer.GetDuration() * 1000.0f) + " ms, collisions : " + std::to_string(m_collidingPairs.size()));
	}
}

void	CPhysicEngine::Step(float deltaTime)
{
	if (!m_active)
	{
		return;
	}

	DetectCollisions();
}

void	CPhysicEngine::CollisionBroadPhase()
{
	m_pairsToCheck.clear();
	m_broadPhase->GetCollidingPairsToCheck(m_pairsToCheck);
	gVars->pRenderer->DisplayText("Amount of pairs to check : " + std::to_string(m_pairsToCheck.size()));
}

void	CPhysicEngine::CollisionNarrowPhase()
{
	m_collidingPairs.clear();
	for (const SPolygonPair& pair : m_pairsToCheck)
	{
		SCollision collision;
		collision.polyA = pair.GetpolyA();
		collision.polyB = pair.GetpolyB();
		if (pair.GetpolyA()->CheckCollision(*(pair.GetpolyB()), collision.point, collision.normal, collision.distance)) 
		{
			m_collidingPairs.push_back(collision);
		}
	}
}

void CPhysicEngine::AddPolygon(CPolygonPtr polygon)
{
	m_broadPhase->OnObjectAdded(polygon);
	polygon->onTransformUpdatedCallback = ([this, polygon](const CPolygon& poly)
	{
		m_broadPhase->OnObjectUpdated(polygon);
	});
	m_polygons.push_back(polygon);
}

void CPhysicEngine::RemovePolygon(CPolygonPtr polygon)
{
	m_polygons.erase(std::remove(m_polygons.begin(), m_polygons.end(), polygon), m_polygons.end());
	m_broadPhase->OnObjectRemoved(polygon);
}