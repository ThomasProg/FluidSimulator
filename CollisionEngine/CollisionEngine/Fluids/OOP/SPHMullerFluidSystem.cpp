#include "SPHMullerFluidSystem.hpp"

void	AddParticles(const Vec2& pos, const Vec2& vel)
{
	Particle particle;
	particle.position = pos;
	particle.velocity = vel;
	particle.

}

void	RemoveParticles()
{

}

void	Update(float dt)
{

}

void	ComputeDensity();
void	ComputePressure();
void	ComputeSurfaceTension();
void	AddPressureForces();
void	AddViscosityForces();
void	BorderCollisions();