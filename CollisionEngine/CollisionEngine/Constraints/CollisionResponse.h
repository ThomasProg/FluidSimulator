#ifndef _COLLISION_RESPONSE_
#define _COLLISION_RESPONSE_

#include "Constraint.h"

class CollisionResponse final : public Constraint
{
public:
	float restitution = 0.0f;
	float friction = 0.5f;

	static Vec3 ToVec3(const Vec2& v)
	{
		return Vec3(v.x, v.y, 0.f);
	}

	static Vec2 ToVec2(const Vec3& v)
	{
		return Vec2(v.x, v.y);
	}

	virtual void RunConstraint() override
	{
		for (SCollision& collision : *m_collidingPairs)
		{
			auto& [polyA, polyB, point, normal, distance] = collision;

			Vec3 normal3D = ToVec3(normal);

			Vec3 rA = ToVec3(point - polyA->Getposition());
			Vec3 rB = ToVec3(point - polyB->Getposition());
			Vec3 vAi = ToVec3(polyA->speed) + Vec3::Cross(Vec3(0, 0, polyA->angularVelocity), rA);
			Vec3 vBi = ToVec3(polyB->speed) + Vec3::Cross(Vec3(0, 0, polyB->angularVelocity), rB);

			Vec3 momentumA = polyA->invWorldTensor * Vec3::Cross(rA, normal3D);
			Vec3 momentumB = polyB->invWorldTensor * Vec3::Cross(rB, normal3D);

			float weightRotA = Vec3::Dot(Vec3::Cross(momentumA, rA), normal3D);
			float weightRotB = Vec3::Dot(Vec3::Cross(momentumB, rB), normal3D);

			float relativeSpeed = Vec3::Dot(vAi - vBi, normal3D);

			if (relativeSpeed < 0)
				return;

			float j = (-(1 + restitution) * relativeSpeed) / (polyA->invMass + polyB->invMass + weightRotA + weightRotB);

			//collision.polyA->Setposition(collision.polyA->Getposition() - collision.polyA->speed * frameTime);
			//collision.polyB->Setposition(collision.polyB->Getposition() - collision.polyB->speed * frameTime);

			polyA->speed += normal * (j * polyA->invMass);
			polyB->speed -= normal * (j * polyB->invMass);

			//ApplyFriction(collision, j);

			Vec3 angularVelocityA = momentumA * j;
			Vec3 angularVelocityB = momentumB * j;

			polyA->angularVelocity += angularVelocityA.z;
			polyB->angularVelocity -= angularVelocityB.z;
		}
	}
};

#endif