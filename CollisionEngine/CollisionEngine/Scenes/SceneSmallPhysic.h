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

		Vec2 start = Vec2(0, -gVars->pRenderer->GetWorldHeight() * 0.5f + (0.5f + 0.5f * size) * m_scale) - Vec2(0, -m_scale * 0);
		for (int i = 0; i < 5; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				CPolygonPtr p = gVars->pWorld->AddSquare(size * m_scale);
				p->Setposition(start - Vec2(i * m_scale, -j * m_scale) * size /*+ Vec2(Random(-0.01f, 0.01f), Random(-0.01f, 0.01f)) * m_scale*/);
				//p->density = (i == 0 && j == 0) ? 0 : p->density;
				gVars->pPhysicEngine->AddPolygon(p);
			}
		}		
		
		CPolygonPtr circle = gVars->pWorld->AddSymetricPolygon(1.0f * m_scale, 50);
		circle->Setposition(Vec2(5.0f * m_scale, -2.5f * m_scale));
		
		
		circle->speed.x = -40.0f * m_scale;
		circle->speed.y = 0.0f * m_scale;
		circle->density = 0.1f;
		gVars->pPhysicEngine->AddPolygon(circle);
	}

	float m_scale;
};

#endif