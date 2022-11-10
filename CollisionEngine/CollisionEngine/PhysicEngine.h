#ifndef _PHYSIC_ENGINE_H_
#define _PHYSIC_ENGINE_H_

#include <vector>
#include <unordered_map>
#include "Maths.h"
#include "Polygon.h"

class IBroadPhase;

class SPolygonPair
{
private:
	// polyA < polyB
	CPolygonPtr	polyA; 
	CPolygonPtr	polyB;

public:
	GETTER(polyA)
	GETTER(polyB)

	SPolygonPair(const CPolygonPtr& _polyA, const CPolygonPtr& _polyB) : polyA(_polyA), polyB(_polyB)
	{
		if (_polyA == _polyB)
		{
			throw std::exception("SPolygonPair : polyA and polyB can't be the same.");
		}

		if (_polyA > _polyB)
		{
			swap(polyA, polyB);
		}
	}
	
	bool operator==(const SPolygonPair& rhs) const
	{
		return (polyA == rhs.polyA) && (polyB == rhs.polyB); // polyA < polyB => opti
	}
};

struct SCollision
{
	SCollision() = default;
	SCollision(CPolygonPtr _polyA, CPolygonPtr _polyB, Vec2	_point, Vec2 _normal, float _distance)
		: polyA(_polyA), polyB(_polyB), point(_point), normal(_normal), distance(_distance){}

	CPolygonPtr	polyA, polyB;

	Vec2	point;
	Vec2	normal;
	float	distance;
};

class CPhysicEngine
{
private:
	friend class CPenetrationVelocitySolver;

	void						CollisionBroadPhase();
	void						CollisionNarrowPhase();

	bool						m_active = true;

	// Collision detection
	IBroadPhase* m_broadPhase;
	std::vector<SPolygonPair>	m_pairsToCheck;
	std::vector<SCollision>		m_collidingPairs;
	std::vector<CPolygonPtr>    m_polygons;

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

	void AddPolygon(CPolygonPtr polygon);
	void RemovePolygon(CPolygonPtr polygon);
};

#endif