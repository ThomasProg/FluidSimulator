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
	float length = 1.f;

	float GetContactLength() const
	{
		return (p1.radius + p2.radius) - GetLength();
	}

	float GetLength() const
	{
		return length;
		//return (p1.position - p2.position).GetLength();
	}
};

class SPHMullerFluidSystem : public IFluidSystem
{
	private:
		float stiffness = 500.f;
		float radius = 0.1f;
		float restitution = 0.4f;
		float friction = 0.4f;
		float maxSpeed = 10.0f;
		float maxAcceleration = 900.0f;

		std::vector<Contact> contacts;
		std::vector<Particle> particles;
		CFluidMesh	mesh;

	public:
		std::shared_ptr<Fluid> defaultFluid = std::make_shared<Fluid>(GetAir());
		float restDensity = 0.59f;

		float GetMass();

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
		float	KernelSpikyGradientFactor(float r, float h);
		float	KernelViscosityLaplacian2(float r, float h);
};

#endif