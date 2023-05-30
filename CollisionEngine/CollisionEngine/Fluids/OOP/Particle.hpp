#ifndef _OOP_PARTICLE_HPP_
#define _OOP_PARTICLE_HPP_

#include "Maths.h"
#include "Fluid.hpp"

#include <vector>
#include <string>
#include <memory>

struct Particle
{
	float pressure = 0.f;
	float density = 1.f; /* volumic mass */
	Vec2 position;
	Vec2 velocity;
	Vec2 acceleration;

	std::weak_ptr<Fluid> fluid;

	inline float GetViscosity() const
	{
		const std::shared_ptr<Fluid> fluidSp = fluid.lock();
		return fluidSp->viscosity;
	}
};

#endif