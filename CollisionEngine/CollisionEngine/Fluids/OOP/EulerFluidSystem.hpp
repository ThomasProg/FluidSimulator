#ifndef _OOP_EULER_FLUID_SYSTEM_HPP_
#define _OOP_EULER_FLUID_SYSTEM_HPP_

#include "Maths.h"
#include "Fluids/OOP/FluidSystem.hpp"
#include "Fluids/OOP/Fluid.hpp"
#include <memory>
#include "FluidMesh.h"

class EulerFluidSystem : public IFluidSystem
{
	using Vec = Vec2;
	using VecInt = Vec2Int;

	struct Vertex
	{
		Vec velocity = Vec::Zero();

		std::weak_ptr<Fluid> fluid;
	};

	struct Cell
	{
		std::weak_ptr<Fluid> fluid;
	};

public:
	Vec worldPosition; // world pos of bottom left corner

	std::vector<Cell> cells;
	VecInt cellsCount;
	Vec cellSize;

	std::vector<Vertex> vertices;
	VecInt verticesCount;

	std::shared_ptr<Fluid> defaultFluid = std::make_shared<Fluid>(GetAir());

	void Reset(Vec newWorldPosition, VecInt newCellsCount)
	{
		worldPosition = newWorldPosition;

		cellsCount = newCellsCount;
		verticesCount = newCellsCount - VecInt::One();

		cells.resize(cellsCount.Sum());
		vertices.resize(verticesCount.Sum());
	}

	virtual void AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec fluidWorldPosition, Vec Velocity, float radius) override
	{
		Vec relativePos = fluidWorldPosition - worldPosition;

		VecInt indices =  (relativePos / cellSize).Floor();
		Vec2Int offset = (radius / cellSize).Floor();

		for (int y = Max(0,indices.y - offset.y); y < Min(verticesCount.x, indices.y + offset.y); y++)
		{
			for (int x = Max(0, indices.x - offset.x); x < Min(verticesCount.y, indices.x + offset.x); x++)
			{
				int vertexIndex = x + y * verticesCount.x;
				vertices[vertexIndex].velocity = Velocity;
				vertices[vertexIndex].fluid = fluid;
			}
		}
	}

	virtual void RemoveFluidAt(Vec2 fluidWorldPosition, float radius) override
	{
		AddFluidAt(defaultFluid, fluidWorldPosition, Vec::Zero(), radius);
	}

	virtual void Update(float deltaTime) override
	{


		Draw();
	}

	VecInt GetCellCoordsFromIndex(int index) const
	{
		Vec2Int Coords;
		Coords.y = index / cellsCount.x;
		Coords.x = index - Coords.y * cellsCount.x;
		return Coords;
	}

	Vec GetWorldPosFromCoords(VecInt coords) const
	{
		return (Vec2(coords) * cellSize) + worldPosition;
	}

	CFluidMesh	m_mesh;
	void Draw()
	{
		// TODO : For each fluid
		m_mesh.Fill(cells.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
		{
			const Cell& cell = cells[iVertex];

			Vec2 pos = GetWorldPosFromCoords(GetCellCoordsFromIndex(iVertex));
			x = pos.x;
			y = pos.y;
			if (std::shared_ptr<Fluid> fluid = cell.fluid.lock())
			{
				r = fluid->color.x;
				g = fluid->color.y;
				b = fluid->color.z;
			}
		});

		m_mesh.Draw();
	}
};

#endif