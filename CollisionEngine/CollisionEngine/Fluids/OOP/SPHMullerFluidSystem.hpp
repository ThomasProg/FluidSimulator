#ifndef _OOP_SPHMULLERFLUIDSYSTEM_HPP_
#define _OOP_SPHMULLERFLUIDSYSTEM_HPP_

#include "Maths.h"
#include "Particle.hpp"
#include "FluidSystem.hpp"
#include "FluidMesh.h"

#include <vector>
#include <string>

struct Contact
{
	Particle& p1;
	Particle& p2;
};

class SPHMullerFluidSystem : public IFluidSystem
{
	private:
		float radius = 1.0f;
		std::vector<Contact> contacts;
		std::vector<Particle> particles;
		CFluidMesh	mesh;

	public:
		void Init();
		void AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec2 worldPosition, Vec2 Velocity, float radius) override;
		void RemoveFluidAt(Vec2 worldPosition, float radius) override;
		void Update(float deltaTime) override;
		void Draw();

	private:
		void	UpdateContacts();
		void	ComputeDensity();
		void	ComputePressure();
		void	ComputeSurfaceTension();
		void	AddPressureForces();
		void	AddViscosityForces();
		void	AddGravityForces();
		void	BorderCollisions();
		void	ResetAcceleration();

		// Update Velocity
		void	ApplyForces(float deltaTime);

		// Update Position
		void	Integrate(float deltaTime);

		void	AddParticle(const std::weak_ptr<struct Fluid>& fluid, const Vec2& pos, const Vec2& vel);
		void	RemoveParticle();

		float	KernelDefault(float r, float h);
};

#endif