#ifndef _EULER_SYSTEM_H_
#define _EULER_SYSTEM_H_

#include "Fluid.h"

// Should be accessed throught Fluid only
class EulerSystem final : public IFluidSystem
{
public:
	using Super = IFluidSystem;
	class Fluid;

#pragma region Cells
	class Cell;
	IMPLEMENT_KEY(CellSignature, cellID, friend EulerSystem;)

	std::vector<CellSignature> signaturePerCell;

	Cell AddCell()
	{
		Cell particle = MakeCell(signaturePerCell.size());

		CellSignature newParticleSignature;
		newParticleSignature.cellID = signaturePerCell.size();
		signaturePerCell.push_back(newParticleSignature);

		//positions.emplace_back();
		//velocities.emplace_back();

		return particle;
	}

	virtual void RemoveCell(Cell particle)
	{
		unsigned int index = particle.GetIndex();
		//QuickRemove(positions, index);
		//QuickRemove(velocities, index);
		QuickRemove(signaturePerCell, index);
	}

	GENERATE_ALL_UTILITY_MACROS(Cell, signaturePerCell);

#pragma region ParticleAccessor
	class Cell
	{
		GENERATE_REDIRECTOR(Cell, EulerSystem, CellSignature, signaturePerCell, MakeCell);
		//GENERATE_REDIRECTOR_GETTER(Position, positions);
		//GENERATE_REDIRECTOR_GETTER(Velocity, velocityPerCell);
	};
#pragma endregion

#pragma endregion

#pragma region CellData
private:
	// ADD ADDITIONAL CELL FIELDS HERE //
	std::vector<FluidSignature> fluidPerCell;
	std::vector<Vec2> velocityPerCell;
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
		//m_mesh.Fill(particlesPerFluid[0].particleSignatures.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
		//{
		//	Fluid f = MakeFluid(0);
		//	Particle p = particlesPerFluid[0].MakeParticle(iVertex);

		//	const Vec2& pos = p.GetPositionRef();
		//	const Vec3& color = f.GetVisualPropertiesRef().color;
		//	x = pos.x;
		//	y = pos.y;
		//	r = color.x;
		//	g = color.y;
		//	b = color.z;
		//});

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
		
		//particlesPerFluid.emplace_back();

		return fluid;
	}

	virtual void RemoveFluid(Super::Fluid fluid) override
	{
		unsigned int index = fluid.GetIndex();
		//QuickRemove(particlesPerFluid, index);
		Super::RemoveFluid(fluid);
	}

public:
	virtual void AddFluidAt(Super::Fluid superFluid, Vec2 pos, Vec2 velocity, unsigned int amount, float radius) override
	{
		Fluid fluid = superFluid;

		//// TODO : Use radius
		//for (unsigned int i = 0; i < amount; i++)
		//{
		//	Particle p = fluid.GetParticlesRef().AddParticle();
		//	const float angle = 2 * M_PI / amount * i;
		//	p.GetPositionRef() = pos + Vec2(cos(angle) * radius, sin(angle) * radius);
		//	p.GetVelocityRef() = velocity;
		//}
	}
	virtual void RemoveFluidAt(Super::Fluid superFluid, Vec2 pos, float radius) override
	{
		Fluid fluid = superFluid;

		float sqrRadius = radius * radius;

		//// TODO : Add Particles
		//Particles& particles = fluid.GetParticlesRef();
		//particles.RForEachParticle([&](Particle& particle)
		//{
		//	if (Vec2::SqrDist(pos, particle.GetPositionRef()) < sqrRadius)
		//	{
		//		particles.RemoveParticle(particle);
		//	}
		//});
	}
#pragma endregion

#pragma region FluidAccessor
public:
	// Shadowing parent class
	GENERATE_REDIRECTOR_CHILD_START(Fluid, EulerSystem::Super::Fluid, EulerSystem);
		//GENERATE_REDIRECTOR_GETTER(Particles, particlesPerFluid);
	GENERATE_REDIRECTOR_CHILD_END();
#pragma endregion
};

#endif