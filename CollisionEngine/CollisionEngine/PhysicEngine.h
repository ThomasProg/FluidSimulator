#ifndef _PHYSIC_ENGINE_H_
#define _PHYSIC_ENGINE_H_

#include <vector>
#include <unordered_map>
#include "Maths.h"
#include "Polygon.h"
#include "BroadPhase.h"
#include "BroadPhaseSwitcher.h"

struct SCollision
{
	SCollision() = default;
	SCollision(CPolygonPtr _polyA, CPolygonPtr _polyB, Vec2	_point, Vec2 _normal, float _distance)
		: polyA(_polyA), polyB(_polyB), point(_point), normal(_normal), distance(_distance){}

	CPolygonPtr	polyA, polyB;

	Vec2	point;
	Vec2	normal;
	float	distance;

	// Additional cached data
	struct
	{
		float invMassSum;

		//Vec3 aToPoint;
		//Vec3 bToPoint;
		//Vec3 vAi;
		//Vec3 vBi;
		//Vec3 momentumA;
		//Vec3 momentumB;
		//float weightRotA;
		//float weightRotB;
		//float relativeSpeed;

		//float impulse;

	} cache;

	void ComputeInvMassSum()
	{
		cache.invMassSum = polyA->invMass + polyB->invMass;
	}
};

class CPhysicEngine
{
public:
	friend class CPenetrationVelocitySolver;

	void						CollisionBroadPhase();
	void						CollisionNarrowPhase();

	bool						m_active = true;

	// Collision detection
	std::vector<SPolygonPair>	m_pairsToCheck;
	std::vector<SCollision>		m_collidingPairs;
	std::vector<CPolygonPtr>    m_polygons;
	BroadPhaseSwitcher			m_broadPhase = BroadPhaseSwitcher(m_polygons);

	Vec2 gravity = Vec2(0, -9.8f);
	float elasticity = 0.6f;

public:
	void	Reset();
	void	Activate(bool active);

	void	DetectCollisions();

	void	Step(float deltaTime);

	template<typename TFunctor>
	void	ForEachCollision(TFunctor functor)
	{
		for (const SCollision& collision : m_collidingPairs)
		{
			functor(collision);
		}
	}

	void AddPolygon(CPolygonPtr polygon, bool addGravity = true);
	void RemovePolygon(CPolygonPtr polygon);
	void SetBroadPhase(std::unique_ptr<IBroadPhase>&& broadPhase);
	void NextBroadPhase()
	{
		m_broadPhase.NextBroadPhase();
	}
	void PreviousBroadPhase()
	{
		m_broadPhase.PreviousBroadPhase();
	}
};

#endif