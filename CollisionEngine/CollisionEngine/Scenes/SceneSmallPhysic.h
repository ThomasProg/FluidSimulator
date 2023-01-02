#ifndef _SCENE_SMALL_PHYSIC_H_
#define _SCENE_SMALL_PHYSIC_H_

#include "BaseScene.h"


class CSceneSmallPhysic : public CBaseScene
{
public:
	CSceneSmallPhysic(float scale = 1.f) : CBaseScene(0.5f * scale, 10.0f * scale), m_scale(scale){}

private:
	virtual void Create() override
	{
		CBaseScene::Create();

		float size = 0.5f;

		//{
		//	CPolygonPtr block = gVars->pWorld->AddRectangle(1 * 130.0f, 1.0f);
		//	block->Setposition(Vec2(0.0f, -1 * 0.0f));
		//	block->density = 0.0f;
		//	block->invMass = 0.0f;
		//	gVars->pPhysicEngine->AddPolygon(block);
		//}

		Vec2 start = Vec2(0, -gVars->pRenderer->GetWorldHeight() * 0.5f + (0.5f + 0.5f * size) * m_scale) - Vec2(0, -m_scale * 0);
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				CPolygonPtr p = gVars->pWorld->AddSquare(size * m_scale);
				p->invMass = 1.f / 5.f;
				p->Setposition(start - Vec2(i * m_scale, -j * m_scale/* - 10*/) * size /*+ Vec2(Random(-0.01f, 0.01f), Random(-0.01f, 0.01f)) * m_scale*/);
				//p->density = (i == 0 && j == 0) ? 0 : p->density;
				gVars->pPhysicEngine->AddPolygon(p);
			}
		}		
		
		//for (int i = 0; i < 3; ++i)
		//{
		//	float coeff = 1 * 0.2f;
		//	CPolygonPtr sqr = gVars->pWorld->AddSquare(coeff * 10.0f);
		//	sqr->density = 0.5f;
		//	sqr->Setposition(Vec2(coeff * 15.0f - 4, coeff * 15.0f));
		//	sqr->invMass = 1.f / 5.f;
		//	gVars->pPhysicEngine->AddPolygon(sqr);
		//}

		//CPolygonPtr circle = gVars->pWorld->AddSymetricPolygon(1.0f * m_scale, 50);
		//circle->Setposition(Vec2(5.0f * m_scale, -2.5f * m_scale));		
		//circle->speed.x = -40.0f * m_scale;
		//circle->speed.y = 0.0f * m_scale;
		//circle->density = 0.1f;
		//gVars->pPhysicEngine->AddPolygon(circle);


		{
			CPolygonPtr block = gVars->pWorld->AddRectangle(1 * 130.0f, 1.0f);
			block->Setposition(Vec2(0.0f, -5.0f * 1.0f));
			block->density = 0.0f;
			block->invMass = 0.0f;
			block->invWorldTensor = block->invLocalTensor = Mat3::Zero();
			gVars->pPhysicEngine->AddPolygon(block);
		}


		gVars->pWorld->AddBehavior<CPhysicsResponse>(nullptr);
	}

	float m_scale;
};

#endif