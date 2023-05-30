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
	ResetAcceleration();
	UpdateContacts();

	ComputeDensity();
	ComputePressure();
	//ComputeSurfaceTension();


	AddPressureForces();
	//AddViscosityForces();

	AddGravityForces();
	ApplyForces(dt);

	Integrate(dt);

	BorderCollisions();

	Draw();
}

void	SPHMullerFluidSystem::ComputeDensity()
{
	float baseWeight = KernelDefault(0.0f, radius);

	for (int i = 0; i < contacts.size(); i++)
	{
		const Vec2& aPos = contacts[i].p1.position;
		const Vec2& bPos = contacts[i].p2.position;

		float weight = KernelDefault(contacts[i].GetContactLength(), radius);
		contacts[i].p1.density += weight;
		contacts[i].p2.density += weight;
	}

	for (int j = 0; j < particles.size(); j++)
	{
		particles[j].density *= particles[j].GetMass();
	}
}

void	SPHMullerFluidSystem::ComputePressure()
{
	float stiffness = 500.f;
	for (Particle& particle : particles)
	{
		particle.pressure = stiffness * (particle.density - defaultFluid->volumicMass);
	}
}
void	SPHMullerFluidSystem::ComputeSurfaceTension()
{

}

void	SPHMullerFluidSystem::UpdateContacts()
{
	contacts.clear();
	for (int i = 0; i < particles.size(); i++)
	{
		Particle& p1 = particles[i];
		for (int j = i + 1; j < particles.size(); j++)
		{
			Particle& p2 = particles[j];
			
			if ((p2.position - p1.position).GetSqrLength() <= Sqr(p1.radius + p2.radius))
			{
				contacts.push_back(Contact{ p1, p2 });
			}
		}
	}
}

void	SPHMullerFluidSystem::ResetAcceleration()
{
	for (Particle& particle : particles)
	{
		particle.acceleration = Vec2(0,0);
	}
}

void	SPHMullerFluidSystem::AddPressureForces()
{

}

void	SPHMullerFluidSystem::AddViscosityForces()
{

}

void	SPHMullerFluidSystem::AddGravityForces()
{
	Vec2 gravity = { 0.f, -3.81f };

	for (Particle& particle : particles)
	{
		particle.acceleration += gravity;
	}
}

void	SPHMullerFluidSystem::ApplyForces(float deltaTime)
{
	for (Particle& particle : particles)
	{
		particle.velocity += particle.acceleration * deltaTime;
	}
}

void	SPHMullerFluidSystem::Integrate(float deltaTime)
{
	for (Particle& particle : particles)
	{
		particle.position += particle.velocity * deltaTime;
	}
}

void	SPHMullerFluidSystem::BorderCollisions()
{
	const float restitution = 0.4f;
	const float friction = 0.4f;

	for (Particle& particle : particles)
	{
		if (particle.position.y < 0 && particle.velocity.y < 0)
		{
			particle.velocity.y *= -restitution;
			particle.velocity.x *= friction;
			particle.position += particle.velocity.Normalized() * (- particle.position.y); // put back behind the border
		}
	}
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

float SPHMullerFluidSystem::KernelDefault(float r, float h)
{
	float h2 = Sqr(h);
	float h4 = Sqr(h2);
	float kernel = h2 - Sqr(r);
	return (kernel * kernel * kernel) * (4.0f / (((float)M_PI) * Sqr(h4)));
}
