#ifndef _SPHERE_SIMULATION_H_
#define _SPHERE_SIMULATION_H_

#include "Behavior.h"
#include "PhysicEngine.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "World.h"

#include "BroadPhase.h"
#include "BroadPhases/BroadPhaseBrut.h"
#include "BroadPhases/CircleToCircle.h"
#include "BroadPhases/AABBTree.h"
#include "BroadPhases/Grid.h"
#include "BroadPhases/SweepAndPrune.h"

#define RADIUS 2.0f
#define DISTANCE 5.0f

struct SConstraint
{
	SConstraint(CPolygonPtr _A, CPolygonPtr _B)
		: A(_A), B(_B){}

	CPolygonPtr A, B;
	float distance = DISTANCE;
};

class CSphereSimulation : public CBehavior
{
private:
	std::vector<CPolygonPtr>	m_circles;
	std::vector<CPolygonPtr>	m_chain;

	//CBroadPhaseBrut broadPhase = CBroadPhaseBrut(polygons);
	//CGrid broadPhase = CGrid(RADIUS * 2);
	CCircleToCircle broadPhase = CCircleToCircle(m_circles);

	void InitChain(size_t count, const Vec2& start)
	{
		for (size_t i = 0; i < count; ++i)
		{
			m_chain.push_back(AddCircle(start + Vec2(0.0f, -(float)i * DISTANCE)));
		}
	}

	void SolveChainConstraints()
	{
		//m_chain[0]->speed = Vec2();
	}

	virtual void Start() override
	{
		for (float x = -12.0f; x < 12.0f; x += 5.0f)
		{
			for (float y = -22.0f; y < 22.0f; y += 10.0f)
			{
				AddCircle(Vec2(x + Random(-0.1f, 0.1f), y + Random(-0.1f, 0.1f)))->speed.x = 50.0f;
			}
		}

		InitChain(8, Vec2(20.0f, 20.0f));

		gVars->pPhysicEngine->Activate(false);
	}

	virtual void Update(float frameTime) override
	{
		for (CPolygonPtr& circle : m_circles)
		{
			circle->speed.y -= 20.0f * frameTime; // apply gravity 
			circle->speed -= circle->speed * 0.3f * frameTime; // apply friction
		}

		std::vector<SPolygonPair> pairsToCheck;
		broadPhase.GetCollidingPairsToCheck(pairsToCheck);
		gVars->pRenderer->DisplayText("Amount of pairs to check : " + std::to_string(pairsToCheck.size()));

		for (SPolygonPair pair : pairsToCheck)
		{
			CPolygonPtr c1 = pair.GetpolyA();
			CPolygonPtr c2 = pair.GetpolyB();
			
			Vec2 diffPos = c2->Getposition() - c1->Getposition();
			Vec2 diffSpeed = c2->speed - c1->speed;
			if (diffPos.GetSqrLength() < 4.0f * RADIUS * RADIUS && ((diffSpeed | diffPos) < 0.0f))
			{
				// TODO : Handle collisions here
				c1->speed = Vec2();
				c2->speed = Vec2();
			}
		}

		float hWidth = gVars->pRenderer->GetWorldWidth() * 0.5f;
		float hHeight = gVars->pRenderer->GetWorldHeight() * 0.5f;

		for (CPolygonPtr& circle : m_circles)
		{
			if (circle->Getposition().x < -hWidth + RADIUS && circle->speed.x < 0)
			{
				circle->speed.x *= -1.0f;
			}
			else if (circle->Getposition().x > hWidth - RADIUS && circle->speed.x > 0)
			{
				circle->speed.x *= -1.0f;
			}
			if (circle->Getposition().y < -hHeight + RADIUS && circle->speed.y < 0)
			{
				circle->speed.y *= -1.0f;
			}
			else if (circle->Getposition().y > hHeight - RADIUS && circle->speed.y > 0)
			{
				circle->speed.y *= -1.0f;
			}
		}
			
		SolveChainConstraints();

		for (CPolygonPtr& circle : m_circles)
		{
			if (circle->speed.GetSqrLength() > 0.1) // to prevent spheres from moving when idle, because of float imprecision
				circle->Setposition(circle->Getposition() + circle->speed * frameTime);
		}
	}

	CPolygonPtr AddCircle(const Vec2& pos, float radius = RADIUS)
	{
		CPolygonPtr circle = gVars->pWorld->AddSymetricPolygon(radius, 50);
		circle->density = 0.0f;
		circle->Setposition(pos);

		broadPhase.OnObjectAdded(circle);
		circle->onTransformUpdatedCallback = ([this, circle](const CPolygon& poly)
		{
			broadPhase.OnObjectUpdated(circle);
		});
		m_circles.emplace_back(circle);

		return circle;
	}
};

#endif