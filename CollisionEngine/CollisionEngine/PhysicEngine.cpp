#include "PhysicEngine.h"

#include <iostream>
#include <string>
#include "GlobalVariables.h"
#include "World.h"
#include "Renderer.h" // for debugging only
#include "Timer.h"

#include "BroadPhase.h"
#include "BroadPhases/BroadPhaseBrut.h"
#include "BroadPhases/BoundingVolume.h"
#include "BroadPhases/AABBTree.h"
#include "BroadPhases/Grid.h"
#include "BroadPhases/SweepAndPrune.h"
#include "BroadPhases/QuadTree.h"

void	CPhysicEngine::Reset()
{
	m_pairsToCheck.clear();
	m_collidingPairs.clear();
	m_polygons.clear();

	m_active = true;

	m_broadPhase.Reset();
}

void CPhysicEngine::SetBroadPhase(std::unique_ptr<IBroadPhase>&& broadPhase)
{
	m_broadPhase.SetBroadPhase(std::move(broadPhase));
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
		gVars->pRenderer->DisplayText("NbPolygons : " + std::to_string(m_polygons.size()));
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
	deltaTime = Min(deltaTime, 1.0f / 15.0f);

	if (!m_active)
	{
		return;
	}

	gVars->pWorld->ForEachPolygon([&](CPolygonPtr poly)
	{
		if (poly->density == 0.0f)
		{
			return;
		}
		
		Mat2 rot = poly->Getrotation();
		rot.Rotate(RAD2DEG(poly->angularVelocity * deltaTime));
		poly->Setrotation(rot);
		poly->Setposition(poly->Getposition() + poly->speed * deltaTime);
		//poly->speed += gravity * deltaTime;

		poly->UpdateSpeed(deltaTime);

		// Ground
		if (poly->Getposition().y < -4)
		{
			poly->Setposition(Vec2(poly->Getposition().x, -4));
			//poly->speed.y *= -1;
			poly->speed.y = 0;
		}
	});

	DetectCollisions();
}

void	CPhysicEngine::CollisionBroadPhase()
{
	m_pairsToCheck.clear();
	m_broadPhase.GetCollidingPairsToCheck(m_pairsToCheck);
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
		collision.polyA->UpdateTransformedPoints();
		collision.polyB->UpdateTransformedPoints();
		if (pair.GetpolyA()->CheckCollision(*(pair.GetpolyB()), collision.point, collision.normal, collision.distance)) 
		{
			m_collidingPairs.push_back(collision);
			pair.GetpolyA()->collisionState = CollisionState::NARROW_PHASE_SUCCESS;
			pair.GetpolyB()->collisionState = CollisionState::NARROW_PHASE_SUCCESS;
		}
	}
}

void CPhysicEngine::AddPolygon(CPolygonPtr polygon)
{
	if (polygon->invMass >= 0)
	{
		polygon->ApplyForce(polygon->Getposition(), gravity / polygon->invMass);
	}
	m_broadPhase->OnObjectAdded(polygon);
	m_polygons.push_back(polygon);
}

void CPhysicEngine::RemovePolygon(CPolygonPtr polygon)
{
	m_polygons.erase(std::remove(m_polygons.begin(), m_polygons.end(), polygon), m_polygons.end());
	m_broadPhase->OnObjectRemoved(polygon);
}