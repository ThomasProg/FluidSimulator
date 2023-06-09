#ifndef _SCENE_BOUNCING_POLYS_H_
#define _SCENE_BOUNCING_POLYS_H_

#include "BaseScene.h"

#include "Behaviors/SimplePolygonBounce.h"
#include "Behaviors/PhysicsResponse.h"

class CSceneBouncingPolys : public CBaseScene
{
public:
	CSceneBouncingPolys(size_t polyCount)
		: m_polyCount(polyCount){}

protected:
	virtual void Create() override
	{
		CBaseScene::Create();

		gVars->pWorld->AddBehavior<CSimplePolygonBounce>(nullptr);

		float width = gVars->pRenderer->GetWorldWidth();
		float height = gVars->pRenderer->GetWorldHeight();

		SRandomPolyParams params;
		params.minRadius = 1.0f;
		params.maxRadius = 1.0f;
		params.minBounds = Vec2(-width * 0.5f + params.maxRadius * 3.0f, -height * 0.5f + params.maxRadius * 3.0f);
		params.maxBounds = params.minBounds * -1.0f;
		params.minPoints = 3;
		params.maxPoints = 8;
		params.minSpeed = 1.0f;
		params.maxSpeed = 3.0f;
		
		for (size_t i = 0; i < m_polyCount; ++i)
		{
			CPolygonPtr poly = gVars->pWorld->AddRandomPoly(params);
			poly->density = 1.0f;
			poly->invMass = 1.f / 5.f;
			gVars->pPhysicEngine->AddPolygon(poly, false);
		}

		CBehaviorPtr b = gVars->pWorld->AddBehavior<CPhysicsResponse>(nullptr);
	}

private:
	size_t m_polyCount;
};

#endif