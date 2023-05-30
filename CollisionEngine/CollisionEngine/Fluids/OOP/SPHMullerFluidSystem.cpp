#include "SPHMullerFluidSystem.hpp"


void SPHMullerFluidSystem::Init()
{

}

void SPHMullerFluidSystem::AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec2 worldPosition, Vec2 velocity, float radius)
{
	for (float deltaY = -radius; deltaY < radius; deltaY += radius / 2)
	{
		for (float deltaX = -radius; deltaX < radius; deltaX += radius / 2)
		{
			AddParticle(fluid, worldPosition + Vec2(deltaX, deltaY), velocity);
		}
	}
}

void SPHMullerFluidSystem::RemoveFluidAt(Vec2 worldPosition, float radius)
{

}

void	SPHMullerFluidSystem::AddParticle(const std::weak_ptr<struct Fluid>& fluid, const Vec2& pos, const Vec2& vel)
{
	Particle particle;
	particle.position = pos;
	particle.velocity = vel;
	particle.fluid = fluid;

	particles.push_back(std::move(particle));
}

void	SPHMullerFluidSystem::RemoveParticle()
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
	mesh.Fill(particles.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
	{
		const Particle& particle = particles[iVertex];

		Vec2 pos = particle.position;
		x = pos.x;
		y = pos.y;

		r = particle.fluid.lock()->color.x;
		g = particle.fluid.lock()->color.y;
		b = particle.fluid.lock()->color.z;
	});

	mesh.Draw();
}