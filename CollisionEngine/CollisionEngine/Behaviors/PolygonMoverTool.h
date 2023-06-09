#ifndef _POLYGON_MOVER_TOOL_H_
#define _POLYGON_MOVER_TOOL_H_

#include "Behavior.h"
#include "PhysicEngine.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"

class CPolygonMoverTool : public CBehavior
{
	CPolygonPtr	GetClickedPolygon()
	{
		Vec2 pt, n;
		Vec2 mousePoint = gVars->pRenderer->ScreenToWorldPos(gVars->pRenderWindow->GetMousePos());
		CPolygonPtr clickedPoly;

		gVars->pWorld->ForEachPolygon([&](CPolygonPtr poly)
		{
			if (poly->IsPointInside(mousePoint))
			{
				clickedPoly = poly;
			}
		});

		return clickedPoly;
	}

	virtual void Update(float frameTime) override
	{
		if (gVars->pRenderWindow->GetMouseButton(0) || gVars->pRenderWindow->GetMouseButton(2))
		{
			if (!m_selectedPoly)
			{
				m_selectedPoly = GetClickedPolygon();
				m_prevMousePos = gVars->pRenderer->ScreenToWorldPos(gVars->pRenderWindow->GetMousePos());
				m_translate = gVars->pRenderWindow->GetMouseButton(0);
				m_clickMousePos = m_prevMousePos;

				if (m_selectedPoly)
					m_clickAngle = m_selectedPoly->Getrotation().GetAngle();
			}
			else
			{
				Vec2 mousePoint = gVars->pRenderer->ScreenToWorldPos(gVars->pRenderWindow->GetMousePos());

				if (m_translate)
				{
					Vec2 diff = m_selectedPoly->Getposition();
					m_selectedPoly->Setposition(m_selectedPoly->Getposition() + mousePoint - m_prevMousePos);
					diff -= m_selectedPoly->Getposition();
					m_selectedPoly->speed = diff;
					m_selectedPoly->angularVelocity = 0.0f;
				}
				else
				{
					Vec2 from = m_clickMousePos - m_selectedPoly->Getposition();
					Vec2 to = mousePoint - m_selectedPoly->Getposition();

					Mat2 rot;
					rot.SetAngle(m_clickAngle + from.Angle(to));
					m_selectedPoly->Setrotation(rot);
					m_selectedPoly->angularVelocity = 0.0f;
					m_selectedPoly->speed = Vec2();
				}

				m_prevMousePos = mousePoint;
			}
		}
		else
		{
			m_selectedPoly.reset();
		}
	}

private:
	CPolygonPtr	m_selectedPoly;
	bool		m_translate;
	Vec2		m_prevMousePos;
	Vec2		m_clickMousePos;
	float		m_clickAngle;
};

#endif