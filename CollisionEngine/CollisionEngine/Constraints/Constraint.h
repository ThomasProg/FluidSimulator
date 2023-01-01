#ifndef _CONSTRAINT_H_
#define _CONSTRAINT_H_

#include <vector>
#include "PhysicEngine.h"


class Constraint
{
protected:
	std::vector<SCollision>* m_collidingPairs = nullptr;;

public:
	virtual void Reset()
	{
		m_collidingPairs = nullptr;
	}

	virtual void SetCollisions(std::vector<SCollision>& collisions)
	{
		m_collidingPairs = &collisions;
	}
	virtual void RunConstraint() = 0;
};

#endif