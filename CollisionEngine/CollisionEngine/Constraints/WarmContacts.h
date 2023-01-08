#ifndef _WARM_CONTACTS_H_
#define _WARM_CONTACTS_H_

#include "Constraint.h"

#include <map>

class WarmContacts final : public Constraint
{
public:
	struct PreviousContactPoint
	{
		Vec3 collisionPointRelativeToA;
		Vec3 collisionPointRelativeToB;

		Vec2 point;
		Vec3 normal;
		float lastDist;
		float totalDist = 0.f;
	};

	std::unordered_map<SPolygonPair, PreviousContactPoint> previousContactPoints;
	int nbTimes = 30;

	virtual void RunConstraint() override
	{
		for (const SCollision& collision : (*m_collidingPairs))
		{
			const CPolygonPtr& polyA = collision.polyA;
			const CPolygonPtr& polyB = collision.polyB;
			SPolygonPair pair = SPolygonPair(polyA, polyB);

			auto tuple = previousContactPoints.emplace(pair, PreviousContactPoint());
			PreviousContactPoint& previousContactPoint = tuple.first->second;

			Vec3 collisionPointRelativeToA = collision.point - polyA->Getposition();
			Vec3 collisionPointRelativeToB = collision.point - polyB->Getposition();

			// If the element already exists
			if (!tuple.second)
			{
				Vec2 sep = collision.normal.GetNormal();
				float proj1 = Vec2::Dot(sep, previousContactPoint.point);
				float proj2 = Vec2::Dot(sep, collision.point);


				// Is collision at the same place
				if (IsNearlyEqual(proj1, proj2)
					//IsNearlyEqual(previousContactPoint.collisionPointRelativeToA, collisionPointRelativeToA) 
					&& IsNearlyEqual(previousContactPoint.normal, collision.normal))
				{
					if (!collision.polyA->IsStatic())
					{
						polyA->SetPositionUnsafe(polyA->Getposition() - collision.normal * 0.4f * previousContactPoint.lastDist);

					}
					if (!collision.polyB->IsStatic())
					{
						polyB->SetPositionUnsafe(polyB->Getposition() + collision.normal * 0.4f * previousContactPoint.lastDist);
					}
				}

			}
			else
			{
				previousContactPoint.lastDist = collision.distance;
				previousContactPoint.normal = collision.normal;
				previousContactPoint.totalDist += previousContactPoint.lastDist;
			}

			// Update previousContactPoint
			previousContactPoint.collisionPointRelativeToA = collisionPointRelativeToA;
			previousContactPoint.collisionPointRelativeToB = collisionPointRelativeToB;
			previousContactPoint.point = collision.point;
		}
	}
};

#endif