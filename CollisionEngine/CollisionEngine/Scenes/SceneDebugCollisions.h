#ifndef _SCENE_DEBUG_COLLISIONS_H_
#define _SCENE_DEBUG_COLLISIONS_H_

#include "BaseScene.h"

#include "Behaviors/DisplayCollision.h"
#include "BroadPhaseSwitcher.h"

class CSceneDebugCollisions : public CBaseScene
{
private:
	class CDebugCollisions : public CBehavior
	{
		std::vector<CPolygonPtr> m_polygons;
		BroadPhaseSwitcher m_broadPhase = BroadPhaseSwitcher(m_polygons);

	public:

		virtual void Start() override
		{
			m_broadPhase.Reset();

			CPolygonPtr firstPoly = gVars->pWorld->AddTriangle(15.0f, 15.0f);
			firstPoly->density = 0.0f;
			firstPoly->Setposition(Vec2(-5.0f, -5.0f));
			firstPoly->Build();
			AddPolygon(firstPoly);

			CPolygonPtr secondPoly = gVars->pWorld->AddTriangle(12.5f, 10.0f);
			secondPoly->Setposition(Vec2(5.0f, 5.0f));
			secondPoly->density = 0.0f;
			AddPolygon(secondPoly);

			CDisplayCollision* displayCollision = static_cast<CDisplayCollision*>(gVars->pWorld->AddBehavior<CDisplayCollision>(nullptr).get());
			displayCollision->polyA = firstPoly;
			displayCollision->polyB = secondPoly;
		}

		virtual void Update(float frameTime) override
		{
			if (gVars->pRenderWindow->JustPressedKey(Key::F6))
			{
				m_broadPhase.PreviousBroadPhase();
			}

			if (gVars->pRenderWindow->JustPressedKey(Key::F7))
			{
				m_broadPhase.NextBroadPhase();
			}

			std::vector<SPolygonPair> pairs;
			m_broadPhase->GetCollidingPairsToCheck(pairs);
			gVars->pRenderer->DisplayText("Amount of pairs to check : " + std::to_string(pairs.size()));
		}

		void AddPolygon(const CPolygonPtr& polygon)
		{
			m_broadPhase->OnObjectAdded(polygon);
			m_polygons.push_back(polygon);
		}

		void RemovePolygon(const CPolygonPtr& polygon)
		{
			m_polygons.erase(std::remove(m_polygons.begin(), m_polygons.end(), polygon), m_polygons.end());
			m_broadPhase->OnObjectRemoved(polygon);
		}
	};


	virtual void Create() override
	{
		CBaseScene::Create();

		gVars->pWorld->AddBehavior<CDebugCollisions>(nullptr);
	}
};

#endif