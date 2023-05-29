#ifndef _OOP_FLUID_SYSTEM_HPP_
#define _OOP_FLUID_SYSTEM_HPP_

#include "Maths.h"

class IFluidSystem
{

public:
	virtual void AddFluidAt(struct Fluid& fluid, Vec2 worldPosition, Vec2 Velocity, float radius) = 0;
	virtual void RemoveFluidAt(Vec2 worldPosition, float radius) = 0;
	virtual void Update() = 0;
};

#endif