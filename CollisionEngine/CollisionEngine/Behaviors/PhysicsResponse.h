#ifndef _PHYSICS_RESPONSE_H_
#define _PHYSICS_RESPONSE_H_

#include "Behavior.h"
#include "PhysicEngine.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"
#include "Constraints/NoOverlap.h"
#include "Constraints/CollisionResponse.h"
#include <unordered_set>

#define ALIAS(source, var) auto& var = source . var ;

class CPhysicsResponse : public CBehavior
{
private:
	std::unordered_set<CPolygonPtr> set;
	void UpdateTransforms()
	{
		set.reserve(gVars->pPhysicEngine->m_collidingPairs.size() * 2);
		for (auto& collisions : gVars->pPhysicEngine->m_collidingPairs)
		{
			set.emplace(collisions.polyA);
			set.emplace(collisions.polyB);
		}

		for (CPolygonPtr p : set)
		{
			p->OnTransformUpdated();
		}
	}

public:
	virtual void Update(float frameTime) override
	{
		for (SCollision& col : gVars->pPhysicEngine->m_collidingPairs)
		{
			col.ComputeInvMassSum();
		}

		CollisionResponse colResponse;
		colResponse.SetCollisions(gVars->pPhysicEngine->m_collidingPairs);
		colResponse.RunConstraint();

		NoOverlap noOverlap;
		noOverlap.nbTimes = 30;
		noOverlap.SetCollisions(gVars->pPhysicEngine->m_collidingPairs);
		noOverlap.RunConstraint();

		UpdateTransforms();
	}
};

#endif