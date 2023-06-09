#ifndef _COLLISION_RESPONSE_
#define _COLLISION_RESPONSE_

#include "Constraint.h"

// Velocity / Friction response
class CollisionResponse final : public Constraint
{
public:
	float restitution = 0.6f;
	float friction = 0.1f;

	static Vec2 ToVec2(const Vec3& v)
	{
		return Vec2(v.x, v.y);
	}

	void ApplyFriction(const SCollision& collision, float impulse)
	{
		Vec3 n = Vec3(collision.normal);
		auto& polyA = collision.polyA;
		auto& polyB = collision.polyB;

		Vec3 diffSpeed = Vec3(polyA->speed - polyB->speed);
		Vec3 cross = Vec3::Cross(diffSpeed, n);
		if (cross.SqrLength() <= 0.f)
			return;

		Vec3 tangent = Vec3::Cross(cross, n).GetUnit();
		float tangentVelocity = Vec3::Dot(diffSpeed, tangent);

		float j = -tangentVelocity / collision.cache.invMassSum;
		j = Clamp(j, -abs(impulse) * friction, abs(impulse) * friction);

		if (!polyA->IsStatic())
			polyA->speed += ToVec2(tangent * (j * polyA->invMass));
		if (!polyB->IsStatic())
			polyB->speed -= ToVec2(tangent * (j * polyB->invMass));
	}

	virtual void RunConstraint() override
	{
		//gVars->pPhysicEngine->ForEachCollision([&](const SCollision& collision)
		for (const SCollision& collision : gVars->pPhysicEngine->m_collidingPairs)
		{
			auto& polyA = collision.polyA;
			auto& polyB = collision.polyB;
			auto& point = collision.point;
			auto& normal = collision.normal;
			auto& distance = collision.distance;

			Vec3 normal3D = Vec3(normal);

			Vec3 rA = Vec3(point - polyA->Getposition());
			Vec3 rB = Vec3(point - polyB->Getposition());
			Vec3 vAi = Vec3(polyA->speed) + Vec3::Cross(Vec3(0, 0, polyA->angularVelocity), rA);
			Vec3 vBi = Vec3(polyB->speed) + Vec3::Cross(Vec3(0, 0, polyB->angularVelocity), rB);

			Vec3 momentumA = polyA->invWorldTensor * Vec3::Cross(rA, normal3D);
			Vec3 momentumB = polyB->invWorldTensor * Vec3::Cross(rB, normal3D);

			float weightRotA = Vec3::Dot(Vec3::Cross(momentumA, rA), normal3D);
			float weightRotB = Vec3::Dot(Vec3::Cross(momentumB, rB), normal3D);

			float relativeSpeed = Vec3::Dot(vAi - vBi, normal3D);

			float j = (-(1 + restitution) * relativeSpeed) / (collision.cache.invMassSum + weightRotA + weightRotB);

			ApplyFriction(collision, j);

			Vec3 angularVelocityA = momentumA * j;
			Vec3 angularVelocityB = momentumB * j;

			if (!polyA->IsStatic())
			{
				polyA->speed += normal * (j * polyA->invMass);
				polyA->angularVelocity += angularVelocityA.z;
			}

			if (!polyB->IsStatic())
			{
				polyB->speed -= normal * (j * polyB->invMass);
				polyB->angularVelocity -= angularVelocityB.z;
			}
		};
	}
};

#endif