#ifndef _NO_OVERLAP_
#define _NO_OVERLAP_

#include "Constraint.h"

class NoOverlap final : public Constraint
{
public:
	float steering = 0.2f;
	float maxCorrection = 5.f;
	float slop = 0.f;

	virtual void RunConstraint() override
	{
		for (SCollision& collision : *m_collidingPairs)
		{
			auto& [polyA, polyB, point, normal, distance] = collision;

			QuickSAT quickSAT;
			if (quickSAT.CheckCollision(*polyA, *polyB, normal, point, distance))
			{

				//float sumInvMass = polyA->invMass + polyB->invMass;

				//float steeringForce = Clamp(steering * (distance + slop), 0.f, maxCorrection);
				//Vec2 impulse = normal * (-steeringForce / sumInvMass);

				//polyA->ApplyDirectImpulse(collision.point, impulse);
				//polyB->ApplyDirectImpulse(collision.point, -impulse);
				//
				//polyA->UpdateTransformedPoints();
				//polyB->UpdateTransformedPoints();

				float correction = -(collision.distance * steering) / (polyA->invMass + polyB->invMass);

				polyA->Setposition(polyA->Getposition() + collision.normal * (polyA->invMass * correction));
				polyB->Setposition(polyB->Getposition() - collision.normal * (polyB->invMass * correction));

				polyA->UpdateTransformedPoints();
				polyB->UpdateTransformedPoints();
			}
		}
	}
};

#endif