#ifndef _SCENE_SIMPLE_PHYSIC_H_
#define _SCENE_SIMPLE_PHYSIC_H_

#include "BaseScene.h"
#include "Behaviors/PhysicsResponse.h"

class CSceneSimplePhysic: public CBaseScene
{
public:
	CSceneSimplePhysic(float scale = 1.0f) : CBaseScene(0.5f * scale, 10.0f * scale), m_scale(scale){}

private:
	virtual void Create() override
	{
		CBaseScene::Create();

		float coeff = m_scale * 0.2f;

		CPolygonPtr block = gVars->pWorld->AddRectangle(coeff * 13.0f, coeff * 15.0f);
		block->Setposition(Vec2(0.0f, -coeff * 7.0f));
		block->density = 0.0f;
		gVars->pPhysicEngine->AddPolygon(block);

		CPolygonPtr rectangle = gVars->pWorld->AddRectangle(coeff * 30.0f, coeff * 10.0f);
		rectangle->Setposition(Vec2(coeff * 15.0f, coeff * 5.0f));
		gVars->pPhysicEngine->AddPolygon(rectangle);

		for (int i = 0; i < 4; ++i)
		{

			CPolygonPtr sqr = gVars->pWorld->AddSquare(coeff * 10.0f);
			sqr->density = 0.5f;
			sqr->Setposition(Vec2(coeff * 15.0f, coeff * 15.0f));
			gVars->pPhysicEngine->AddPolygon(sqr);
		}
		CPolygonPtr tri = gVars->pWorld->AddTriangle(coeff * 5.0f, coeff * 5.0f);
		tri->Setposition(Vec2(coeff * 5.0f, coeff * 15.0f));
		tri->density *= 5.0f;
		gVars->pPhysicEngine->AddPolygon(tri);
		//
		CPolygonPtr sphere = gVars->pWorld->AddSymetricPolygon(coeff * 10.0f, 50);
		sphere->Setposition(Vec2(-coeff * 20.0f, coeff * 5.0f));
		gVars->pPhysicEngine->AddPolygon(sphere);

		gVars->pWorld->AddBehavior<CPhysicsResponse>(nullptr);

	}

	float m_scale;
};

#endif