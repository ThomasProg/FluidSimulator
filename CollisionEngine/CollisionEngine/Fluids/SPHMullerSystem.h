#ifndef _SPH_MULLER_SYSTEM_H_
#define _SPH_MULLER_SYSTEM_H_

#include "Fluid.h"

// Should be accessed throught Fluid only
class SPHMullerSystem final : public IFluidSystem
{
public:
	using Super = IFluidSystem;
	class Fluid;

#pragma region Particles
	class Particle;
	class Particles;
	IMPLEMENT_KEY(ParticleSignature, particleID, friend Particles;)

public:
	// Particles data per fluid
	class Particles
	{
	public:
		#pragma region ParticleData
		// ADD PARTICLE FIELDS HERE //
		// If you add a field, make sure to :
		// - Emplace it in AddParticle()
		// - Remove it RemoveParticle()
		// - Add a getter in the Particle class
		std::vector<ParticleSignature> particleSignatures;
		std::vector<Vec2> positions;
		std::vector<Vec2> velocities;
		#pragma endregion


		Particle AddParticle()
		{
			Particle particle = MakeParticle(particleSignatures.size());

			ParticleSignature newParticleSignature;
			newParticleSignature.particleID = particleSignatures.size();
			particleSignatures.push_back(newParticleSignature);

			positions.emplace_back();
			velocities.emplace_back();

			return particle;
		}

		virtual void RemoveParticle(Particle particle)
		{
			unsigned int index = particle.GetIndex();
			QuickRemove(positions, index);
			QuickRemove(velocities, index);
			QuickRemove(particleSignatures, index);
		}

		GENERATE_ALL_UTILITY_MACROS(Particle, particleSignatures);
	};

#pragma region ParticleAccessor
	class Particle
	{
		GENERATE_REDIRECTOR(Particle, Particles, ParticleSignature, particleSignatures, MakeParticle);
		GENERATE_REDIRECTOR_GETTER(Position, positions);
		GENERATE_REDIRECTOR_GETTER(Velocity, velocities);
	};
#pragma endregion

#pragma endregion

#pragma region FluidData
private:
	// ADD ADDITIONAL FLUID FIELDS HERE //
	// If you add a field, make sure to :
	// - Emplace it in AddFluid()
	// - Remove it RemoveFluid()
	// - Add a getter in the Fluid class
	std::vector<Particles> particlesPerFluid;
#pragma endregion

#pragma region FluidSystem
private:
	CFluidMesh m_mesh;

private:
	// TODO 
	// TODO : std::async for each fluid
	void ComputeDensity() {}
	void ComputePressure() {}
	void ComputeInternalForces() {}
	void ComputeExternalForces() {}
	void ComputeForces() 
	{
		ComputeInternalForces();
		ComputeExternalForces();
	}
	void Draw()
	{
		// TODO : For each fluid
		m_mesh.Fill(particlesPerFluid[0].particleSignatures.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
		{
			Fluid f = MakeFluid(0);
			Particle p = particlesPerFluid[0].MakeParticle(iVertex);

			const Vec2& pos = p.GetPositionRef();
			const Vec3& color = f.GetVisualPropertiesRef().color;
			x = pos.x;
			y = pos.y;
			r = color.x;
			g = color.y;
			b = color.z;
		});

		m_mesh.Draw();
	}

public:
	virtual void Update(float deltaTime) override 
	{


		Draw();
	}

	virtual Super::Fluid AddFluid() override
	{
		Fluid fluid = Super::AddFluid();
		
		particlesPerFluid.emplace_back();

		return fluid;
	}

	virtual void RemoveFluid(Super::Fluid fluid) override
	{
		unsigned int index = fluid.GetIndex();
		QuickRemove(particlesPerFluid, index);
	}

public:
	virtual void AddFluidAt(Super::Fluid superFluid, Vec2 pos, Vec2 velocity, unsigned int amount, float radius) override
	{
		Fluid fluid = superFluid;

		// TODO : Use radius
		for (unsigned int i = 0; i < amount; i++)
		{
			Particle p = fluid.GetParticlesRef().AddParticle();
			const float angle = 2 * M_PI / amount * i;
			p.GetPositionRef() = pos + Vec2(cos(angle) * radius, sin(angle) * radius);
			p.GetVelocityRef() = velocity;
		}
	}
	virtual void RemoveFluidAt(Super::Fluid superFluid, Vec2 pos, float radius) override
	{
		Fluid fluid = superFluid;

		float sqrRadius = radius * radius;

		// TODO : Add Particles
		Particles& particles = fluid.GetParticlesRef();
		particles.RForEachParticle([&](Particle& particle)
		{
			if (Vec2::SqrDist(pos, particle.GetPositionRef()) < sqrRadius)
			{
				particles.RemoveParticle(particle);
			}
		});
	}
#pragma endregion

#pragma region FluidAccessor
public:
	// Shadowing parent class
	GENERATE_REDIRECTOR_CHILD_START(Fluid, SPHMullerSystem::Super::Fluid, SPHMullerSystem);
		GENERATE_REDIRECTOR_GETTER(Particles, particlesPerFluid);
	GENERATE_REDIRECTOR_CHILD_END();
#pragma endregion
};

#endif