#include "SPHMullerFluidSystem.hpp"


void SPHMullerFluidSystem::Init()
{

}

void SPHMullerFluidSystem::AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec2 worldPosition, Vec2 Velocity, float radius)
{

}

void SPHMullerFluidSystem::RemoveFluidAt(Vec2 worldPosition, float radius)
{

}

void	SPHMullerFluidSystem::AddParticles(const Vec2& pos, const Vec2& vel)
{
	Particle particle;
	particle.position = pos;
	particle.velocity = vel;
	//particle.

}

void	SPHMullerFluidSystem::RemoveParticles()
{

}

void	SPHMullerFluidSystem::Update(float dt)
{
	ComputeDensity();
	ComputePressure();
	ComputeSurfaceTension();

	AddPressureForces();
	AddViscosityForces();

	ApplyForces(dt);

	Integrate(dt);

	BorderCollisions();

	Draw();
}

void	SPHMullerFluidSystem::ComputeDensity()
{

}
void	SPHMullerFluidSystem::ComputePressure()
{

}
void	SPHMullerFluidSystem::ComputeSurfaceTension()
{

}

void	SPHMullerFluidSystem::AddPressureForces()
{

}

void	SPHMullerFluidSystem::AddViscosityForces()
{

}

void	SPHMullerFluidSystem::ApplyForces(float deltaTime)
{

}

void	SPHMullerFluidSystem::Integrate(float deltaTime)
{

}

void	SPHMullerFluidSystem::BorderCollisions()
{

}

void SPHMullerFluidSystem::Draw()
{

}