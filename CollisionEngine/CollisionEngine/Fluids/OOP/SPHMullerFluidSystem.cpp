#include "SPHMullerFluidSystem.hpp"

float SPHMullerFluidSystem::GetMass()
{
	float particleRadiusRatio = 3.0f;
	float particuleRadius = radius / particleRadiusRatio;
	float volume = particuleRadius * particuleRadius * (float)M_PI;
	return volume * restDensity;
}

void SPHMullerFluidSystem::Init()
{

}

void SPHMullerFluidSystem::AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec2 worldPosition, Vec2 velocity, float radius)
{
	Vec2 min = worldPosition - Vec2(0.5f, 0.5f);
	Vec2 max = worldPosition + Vec2(0.5f, 0.5f);
	float particulesPerMeter = 10.0f;
	//CFluidSystem::Get().Spawn(mousePos - Vec2(0.5f, 0.5f), mousePos + Vec2(0.5f, 0.5f), 10.0f, Vec2(0.0f, 0.0f));


	float width = max.x - min.x;
	size_t horiCount = (size_t)(width * particulesPerMeter) + 1;

	float height = max.y - min.y;
	size_t vertiCount = (size_t)(height * particulesPerMeter) + 1;

	size_t count = horiCount * vertiCount;

	for (size_t i = 0; i < horiCount; ++i)
	{
		for (size_t j = 0; j < vertiCount; ++j)
		{
			float x = min.x + ((float)i) / particulesPerMeter;
			float y = min.y + ((float)j) / particulesPerMeter;

			AddParticle(fluid, Vec2(x, y), velocity);
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

	particles.emplace_back(std::move(particle));
}

void	SPHMullerFluidSystem::RemoveParticle()
{

}

void	SPHMullerFluidSystem::Update(float dt)
{
	float m_timeScale = 1.f;
	dt *= m_timeScale;
	dt = Min(dt, 1.0f / (200.0f * m_timeScale));


	ResetAcceleration(); // OK
	UpdateContacts(); // OK

	ComputeDensity(); // OK
	ComputePressure();
	ComputeSurfaceTension();


	AddPressureForces();
	AddViscosityForces();

	//AddGravityForces();
	ApplyForces(dt); // OK

	Integrate(dt); // OK

	BorderCollisions(); // OK

	Draw(); // OK
}

void	SPHMullerFluidSystem::ComputeDensity()
{
	float mass = GetMass();

	float baseWeight = KernelDefault(0.0f, radius);

	for (Particle& particle : particles)
	{
		particle.density = baseWeight;
	}

	for (Contact& contact : contacts)
	{
		float weight = KernelDefault(contact.length, radius);
		contact.p1.density += weight;
		contact.p2.density += weight;
	}

	for (Particle& particle : particles)
	{
		particle.density *= mass;
	}
}

void	SPHMullerFluidSystem::ComputePressure()
{
	for (Particle& particle : particles)
	{
		particle.pressure = stiffness * (particle.density - restDensity);// defaultFluid->volumicMass);
	}
}
void	SPHMullerFluidSystem::ComputeSurfaceTension()
{

}

void	SPHMullerFluidSystem::UpdateContacts()
{
	contacts.clear();
	contacts.reserve(Sqr(particles.size()));
	for (int i = 0; i < particles.size(); i++)
	{
		Particle& p1 = particles[i];
		for (int j = i + 1; j < particles.size(); j++)
		{
			Particle& p2 = particles[j];
			
			float sqrLength = (p1.position - p2.position).GetSqrLength();
			if (sqrLength <= Sqr(radius))
			{
				float d = sqrt(sqrLength);

				Contact contact{ p1, p2 };
				contact.length = Clamp(d, radius * 0.1f, radius);

				contacts.push_back(contact);
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
	for (Contact& contact : contacts)
	{
		auto& [p1, p2, length] = contact;

		float radius = (p1.radius + p2.radius) / 2.f;
		float mass = GetMass();// (p1.GetMass() + p2.GetMass()) / 2.f;

		Vec2 dist = p1.position - p2.position;

		float pressureMean = (p1.pressure + p2.pressure) / 2.f;

		Vec2 acc = - dist * mass * pressureMean / (p1.density * p2.density) * KernelSpikyGradientFactor(length, radius);

		acc += dist * 0.02f * mass * ((stiffness * (p1.density + p2.density)) / (2.0f * p1.density * p2.density)) * KernelSpikyGradientFactor(length * 0.8f, radius);

		p1.acceleration += acc;
		p2.acceleration -= acc;
	}
}

void	SPHMullerFluidSystem::AddViscosityForces()
{
	for (Contact& contact : contacts)
	{
		float mass = GetMass();
		//float viscosity = 0.1f;
		float viscosity = (contact.p1.fluid.lock()->viscosity + contact.p2.fluid.lock()->viscosity) / 2;

		Vec2 deltaVel = contact.p1.velocity - contact.p2.velocity;
		Vec2 viscosityAcc = deltaVel * -mass * (viscosity / (2.0f * contact.p1.density * contact.p2.density)) * KernelViscosityLaplacian2(contact.GetLength(), radius);

		contact.p1.acceleration += viscosityAcc;
		contact.p2.acceleration -= viscosityAcc;
	}
}

void	SPHMullerFluidSystem::AddGravityForces()
{
	Vec2 gravity = { 0.f, -5.f };

	for (Particle& particle : particles)
	{
		particle.acceleration += gravity;
	}
}

void	SPHMullerFluidSystem::ApplyForces(float deltaTime)
{
	Vec2 gravity(0.0f, -5);// -9.8f);

	for (Particle& particle : particles)
	{
		if (particle.acceleration.GetSqrLength() > Sqr(maxAcceleration))
		{
			particle.acceleration *= maxAcceleration / particle.acceleration.GetLength();
		}

		particle.velocity += (particle.acceleration + gravity) * deltaTime;
	}
}

void	SPHMullerFluidSystem::Integrate(float deltaTime)
{
	for (Particle& particle : particles)
	{
		if (particle.velocity.GetSqrLength() > Sqr(maxSpeed))
		{
			particle.velocity *= maxSpeed / particle.velocity.GetLength();
		}

		particle.position += particle.velocity * deltaTime;
	}
}

void	SPHMullerFluidSystem::BorderCollisions()
{
	for (Particle& particle : particles)
	{
		if (particle.position.y <= 0 && particle.velocity.y < 0)
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

float SPHMullerFluidSystem::KernelSpikyGradientFactor(float r, float h)
{
	float h2 = Sqr(h);
	float h5 = Sqr(h2) * h;
	float kernel = h - r;
	return Sqr(kernel) * (-15.0f / ((float)M_PI * h5 * r));
}

float SPHMullerFluidSystem::KernelViscosityLaplacian2(float r, float h)
{
	float h2 = h * h;
	float kernel = h - r;
	return kernel * (30.0f / ((float)M_PI * h2 * h2 * h));
}
