#ifndef _OOP_SPHMULLERFLUIDSYSTEM_HPP_
#define _OOP_SPHMULLERFLUIDSYSTEM_HPP_

#include "Maths.h"
#include "Particle.hpp"
#include "FluidSystem.hpp"

#include <vector>
#include <string>

class SPHMullerFluidSystem : public IFluidSystem
{
	private:
		std::vector<Particle> particles;

	public:
		void AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec2 worldPosition, Vec2 Velocity, float radius) override;
		void RemoveFluidAt(Vec2 worldPosition, float radius) override;
		void Update(float deltaTime) override;

	private:
		void	ComputeDensity();
		void	ComputePressure();
		void	ComputeSurfaceTension();
		void	AddPressureForces();
		void	AddViscosityForces();
		void	BorderCollisions();
};

#endif