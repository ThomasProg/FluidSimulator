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

	// ADD ADDITIONAL CELL FIELDS HERE //
	std::vector<CellSignature> signaturePerCell;
	std::vector<FluidSignature> fluidPerCell;
	std::vector<Vec2> velocityPerCell;
	std::vector<Vec2> worldPositionPerCell;

	Cell AddCell()
	{
		Cell particle = MakeCell(signaturePerCell.size());

		CellSignature newParticleSignature;
		newParticleSignature.cellID = signaturePerCell.size();
		signaturePerCell.push_back(newParticleSignature);

		fluidPerCell.emplace_back();
		velocityPerCell.emplace_back();
		worldPositionPerCell.emplace_back();

		return particle;
	}

	virtual void RemoveCell(Cell particle)
	{
		unsigned int index = particle.GetIndex();
		QuickRemove(fluidPerCell, index);
		QuickRemove(velocityPerCell, index);
		QuickRemove(worldPositionPerCell, index);
		QuickRemove(signaturePerCell, index);
	}

	GENERATE_ALL_UTILITY_MACROS(Cell, signaturePerCell);

#pragma region CellAccessor
	class Cell
	{
		GENERATE_REDIRECTOR(Cell, EulerSystem, CellSignature, signaturePerCell, MakeCell);
		GENERATE_REDIRECTOR_GETTER(Position, worldPositionPerCell);
		GENERATE_REDIRECTOR_GETTER(Velocity, velocityPerCell);
		GENERATE_REDIRECTOR_GETTER(Fluid, fluidPerCell);
		GENERATE_REDIRECTOR_GETTER(WorldPos, worldPositionPerCell);
	};
#pragma endregion

#pragma endregion

#pragma region FluidSystem
private:
	CFluidMesh m_mesh;

	Vec2 cellSize; //  the size of the cell in world space
	Vec2Int nbCellsPerAxis; // the amount of cells per axis
	Vec2 gridLocation; // the location of the grid in world space

public:
	inline const Vec2Int& GetNbCellsPerAxis() const
	{
		return nbCellsPerAxis;
	}

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
		m_mesh.Fill(worldPositionPerCell.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
		{
			Cell cell = MakeCell(iVertex);
			Super::Fluid f;
			FindFluid(cell.GetFluidRef(), f);

			const Vec2& pos = cell.GetWorldPosRef();
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
	void SetSize(const Vec2& newCellSize, const Vec2Int& newNbCellsPerAxis, FluidSignature defaultFluid)
	{
		// TODO : update previous cells to new cells

		signaturePerCell.clear();

		// y
		for (int yIndex = 0; yIndex < newNbCellsPerAxis.y; yIndex++)
		{
			// x
			for (int xIndex = 0; xIndex < newNbCellsPerAxis.x; xIndex++)
			{
				Cell cell = AddCell();
				cell.GetFluidRef() = defaultFluid;

				Vec2 pos;
				pos.x = xIndex * newCellSize.x + (yIndex % 2) * newCellSize.x / 2;
				pos.y = yIndex * newCellSize.y;
				cell.GetWorldPosRef() = pos + gridLocation;
			}
		}

		cellSize = newCellSize;
		nbCellsPerAxis = newNbCellsPerAxis;
	}

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

		float sqrRadius = radius * radius;

		// TODO : Get cell by position, and update those close to it for better perf
		ForEachCell([&](Cell& cell)
		{
			if (Vec2::SqrDist(cell.GetWorldPosRef(), pos) < sqrRadius)
			{
				cell.GetFluidRef() = fluid;
			}
		});

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