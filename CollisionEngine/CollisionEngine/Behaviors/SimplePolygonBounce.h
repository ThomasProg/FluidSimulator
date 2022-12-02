#ifndef _SIMPLE_COLLISION_BOUNCE_H_
#define _SIMPLE_COLLISION_BOUNCE_H_

#include "Behavior.h"
#include "PhysicEngine.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "World.h"

class CSimplePolygonBounce : public CBehavior
{
private:
	virtual void Update(float frameTime) override
	{
		gVars->pPhysicEngine->ForEachCollision([&](const SCollision& collision)
		{
			collision.polyA->Setposition(collision.polyA->Getposition() + collision.normal * collision.distance * -0.5f);
			collision.polyB->Setposition(collision.polyB->Getposition() + collision.normal * collision.distance * 0.5f);

			collision.polyA->speed.Reflect(collision.normal);
			collision.polyB->speed.Reflect(collision.normal);
		});

		float hWidth = gVars->pRenderer->GetWorldWidth() * 0.5f;
		float hHeight = gVars->pRenderer->GetWorldHeight() * 0.5f;

		gVars->pWorld->ForEachPolygon([&](CPolygonPtr poly)
		{
			Vec2 polyPos = poly->Getposition();
			polyPos += poly->speed * frameTime;

			if (polyPos.x < -hWidth)
			{
				polyPos.x = -hWidth;
				poly->speed.x *= -1.0f;
			}
			else if (polyPos.x > hWidth)
			{
				polyPos.x = hWidth;
				poly->speed.x *= -1.0f;
			}
			if (polyPos.y < -hHeight)
			{
				polyPos.y = -hHeight;
				poly->speed.y *= -1.0f;
			}
			else if (polyPos.y > hHeight)
			{
				polyPos.y = hHeight;
				poly->speed.y *= -1.0f;
			}

			poly->Setposition(polyPos);
		});

		if (gVars->pRenderWindow->JustPressedKey(Key::F6))
		{
			gVars->pPhysicEngine->PreviousBroadPhase();
		}

		if (gVars->pRenderWindow->JustPressedKey(Key::F7))
		{
			gVars->pPhysicEngine->NextBroadPhase();
		}
	}
};

#endif