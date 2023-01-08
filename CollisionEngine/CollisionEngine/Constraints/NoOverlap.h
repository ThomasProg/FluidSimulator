#ifndef _NO_OVERLAP_
#define _NO_OVERLAP_

#include "Constraint.h"

class NoOverlap final : public Constraint
{
public:
	float steering = 0.2f;
	float maxCorrection = 5.f;
	float slop = 0.f;
	int nbTimes = 2;

	struct PreviousContactPoint
	{
		Vec3 collisionPointRelativeToA;
		Vec3 collisionPointRelativeToB;

		Vec2 point;
		Vec3 normal;
		float lastDist;
		float totalDist = 0.f;

		Vec2 impulseDelta = Vec2(0,0);
	};

	struct PerPoly
	{
		Vec2 impulseSum = Vec2(0.f, 0.f);
		Vec2 lastIterationSum = Vec2(0.f, 0.f);
	};
	std::unordered_map<CPolygonPtr, PerPoly> perPoly;


	struct Cache
	{
		float absoluteCorrectionA;
		float absoluteCorrectionB;
	};

	std::vector<Cache> ComputeCache()
	{
		std::vector<Cache> cachePerPair(m_collidingPairs->size());
		//for (const SCollision& collision : *m_collidingPairs)
		for (int i = 0; i < m_collidingPairs->size(); i++)
		{
			const SCollision& collision = (*m_collidingPairs)[i];
			Cache& cache = cachePerPair[i];

			const float absoluteCorrection = steering / collision.cache.invMassSum;
			cache.absoluteCorrectionA = absoluteCorrection * collision.polyA->invMass;
			cache.absoluteCorrectionB = absoluteCorrection * collision.polyB->invMass;
		}
		return cachePerPair;
	}

	virtual void RunConstraint() override
	{
		std::vector<Cache> cachePerPair = ComputeCache();

		for (int time = 0; time < nbTimes; time++)
		{
			for (int i = 0; i < m_collidingPairs->size(); i++)
			{
				SCollision& collision = (*m_collidingPairs)[i];
				Cache& cache = cachePerPair[i];

				auto& polyA = collision.polyA;
				auto& polyB = collision.polyB;

				// Only works because the rotation is not modified.
				// The rotation should be modified, 
				// but most of the time rotation fix is not that important, 
				// especially compared to the performance boost avoiding it gives.
				QuickSAT quickSAT;
				if (quickSAT.CheckCollision(*polyA, *polyB, collision.normal, collision.distance))
				{
					if (!polyA->IsStatic())
					{
						Vec2 impulse = collision.normal * (collision.distance * cache.absoluteCorrectionA);
						//auto it = perPoly.emplace(polyA, PerPoly());
						//it.first->second.impulseSum -= impulse;
						polyA->SetPositionUnsafe(polyA->Getposition() - impulse);

					}
					if (!polyB->IsStatic())
					{
						Vec2 impulse = collision.normal * (collision.distance * cache.absoluteCorrectionB);
						//auto it = perPoly.emplace(polyA, PerPoly());
						//it.first->second.impulseSum += impulse;
						polyB->SetPositionUnsafe(polyB->Getposition() + impulse);
					}
				}
			}
		}
	}

	void RunConstraintWithoutCache()
	{
		for (int i = 0; i < nbTimes; i++)
		{
			for (SCollision& collision : *m_collidingPairs)
			{
				auto& polyA = collision.polyA;
				auto& polyB = collision.polyB;

				QuickSAT quickSAT;
				if (quickSAT.CheckCollision(*polyA, *polyB, collision.normal, collision.distance))
				{
					float correction = (collision.distance * steering) / collision.cache.invMassSum;

					if (!polyA->IsStatic())
					{
						polyA->SetPositionUnsafe(polyA->Getposition() - collision.normal * (polyA->invMass * correction));

					}
					if (!polyB->IsStatic())
					{
						polyB->SetPositionUnsafe(polyB->Getposition() + collision.normal * (polyB->invMass * correction));
					}
				}
			}
		}
	}
};

#endif