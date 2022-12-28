#ifndef _PHYSICS_RESPONSE_H_
#define _PHYSICS_RESPONSE_H_

#include "Behavior.h"
#include "PhysicEngine.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"

class CPhysicsResponse : public CBehavior
{
public:
	float restitution = 0.f;
	float friction = 0.5f;

	CPolygonPtr polyToSpeed;

	static Vec3 ToVec3(const Vec2& v)
	{
		return Vec3(v.x, v.y, 0.f);
	}

	static Vec2 ToVec2(const Vec3& v)
	{
		return Vec2(v.x, v.y);
	}


	void ApplyFriction(const SCollision& collision, float impulse)
	{
		Vec3 n = ToVec3(collision.normal);
		auto& polyA = collision.polyA;
		auto& polyB = collision.polyB;

		Vec3 diffSpeed = ToVec3(polyA->speed - polyB->speed);
		if (Vec3::Cross(diffSpeed, n).SqrLength() < 0.000001)
			return;
		Vec3 tangent = Vec3::Cross(Vec3::Cross(diffSpeed, n), n).GetUnit();
		float tangentVelocity = Vec3::Dot(diffSpeed, tangent);

		float j = -tangentVelocity / (polyA->invMass + polyB->invMass);
		j = Clamp(j, -abs(impulse) * friction, abs(impulse) * friction);
		
		polyA->speed += ToVec2(tangent * (j * polyA->invMass));
		polyB->speed -= ToVec2(tangent * (j * polyB->invMass));
	}

	virtual void Update(float frameTime) override
	{
		gVars->pPhysicEngine->ForEachCollision([&](const SCollision& collision)
		{
				//collision.polyA->Setposition(collision.polyA->Getposition() + collision.normal * collision.distance * 0.5f);
				//collision.polyB->Setposition(collision.polyB->Getposition() - collision.normal * collision.distance * 0.5f);


				//collision.polyA->speed.Reflect(collision.normal);
				//collision.polyB->speed.Reflect(collision.normal);
				 
				//collision.polyA->speed = collision.normal;
				//collision.polyB->speed = -collision.normal;
				//return;

			auto& [polyA, polyB, point, normal, distance] = collision;

			Vec3 normal3D = ToVec3(normal);

			Vec3 rA = ToVec3(point - polyA->Getposition());
			Vec3 rB = ToVec3(point - polyB->Getposition());
			Vec3 vAi = ToVec3(polyA->speed) + Vec3::Cross(Vec3(0, 0, polyA->angularVelocity), rA);
			Vec3 vBi = ToVec3(polyB->speed) + Vec3::Cross(Vec3(0,0,polyB->angularVelocity), rB);

			Vec3 momentumA = Vec3(0,0,0);//Mat3(polyA->invWorldTensor) * Vec3::Cross(rA, normal3D);
			Vec3 momentumB = Vec3(0,0,0);//Mat3(polyB->invWorldTensor) * Vec3::Cross(rB, normal3D);

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

			ApplyFriction(collision, j);

			Vec3 angularVelocityA = momentumA * j;
			Vec3 angularVelocityB = momentumB * j;

			polyA->angularVelocity += angularVelocityA.z;
			polyB->angularVelocity -= angularVelocityB.z;
		});

		if (polyToSpeed != nullptr && gVars->pRenderWindow->JustPressedKey(Key::F6))
		{
			polyToSpeed->speed.x = -5;
		}
	}
};

#endif