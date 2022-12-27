#ifndef _PHYSICS_RESPONSE_H_
#define _PHYSICS_RESPONSE_H_

#include "Behavior.h"
#include "PhysicEngine.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"

class CPhysicsResponse : public CBehavior
{
	virtual void Update(float frameTime) override
	{
		gVars->pPhysicEngine->ForEachCollision([&](const SCollision& collision)
		{
			collision.polyA->Setposition(collision.polyA->Getposition() + collision.normal * collision.distance * -0.5f);
			collision.polyB->Setposition(collision.polyB->Getposition() + collision.normal * collision.distance * 0.5f);

			collision.polyA->speed.Reflect(collision.normal);
			collision.polyB->speed.Reflect(collision.normal);
		});
	}
};

#endif