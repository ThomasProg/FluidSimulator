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
		float pressure;
		Vec velocity;
	};

public:
	Vec worldPosition; // world pos of bottom left corner

	std::vector<Cell> cells;
	VecInt cellsCount;
	Vec cellSize = Vec::One();

	std::vector<Vertex> rightEdges;
	VecInt rightEdgesCount;
	std::vector<Vertex> upEdges;
	VecInt upEdgesCount;
	//std::vector<Vertex> vertices;
	//VecInt verticesCount;

	std::shared_ptr<Fluid> defaultFluid = std::make_shared<Fluid>(GetAir());
	std::shared_ptr<Fluid> errorFluid = std::make_shared<Fluid>(GetWater());

	void ResetPressure();
	
	void Reset(Vec newWorldPosition, VecInt newCellsCount, Vec newCellSize)
	{
		worldPosition = newWorldPosition;

		cellsCount = newCellsCount;
		//verticesCount = newCellsCount - VecInt::One();
		rightEdgesCount = newCellsCount;
		rightEdgesCount.x--;

		upEdgesCount = newCellsCount;
		upEdgesCount.y--;

		cells.resize(cellsCount.Product());
		rightEdges.resize(rightEdgesCount.Product());
		upEdges.resize(upEdgesCount.Product());

		//for (Vertex& vertex : vertices)
		//{
		//	vertex.fluid = defaultFluid;
		//}

		ResetCells(cells);

		cellSize = newCellSize;

		m_mesh.pointSize = newCellSize.x * 50;
	}

	void ResetCells(std::vector<Cell>& inCells)
	{
		for (Cell& cell : inCells)
		{
			cell.fluid = defaultFluid;
		}
	}

	virtual void AddFluidAt(const std::weak_ptr<struct Fluid>& fluid, Vec fluidWorldPosition, Vec Velocity, float radius) override
	{
		Vec relativePos = fluidWorldPosition - worldPosition;

		VecInt indices =  (relativePos / cellSize).Floor();
		Vec2Int offset = (radius / cellSize).Floor();

		for (int y = Max(0,indices.y - offset.y); y < Min(cellsCount.y, indices.y + offset.y); y++)
		{
			for (int x = Max(0, indices.x - offset.x); x < Min(cellsCount.x, indices.x + offset.x); x++)
			{
				//if (x >= 0 && x < verticesCount.x && y >= 0 && y < verticesCount.y) // Check Radius
				{
					int vertexIndex = x + y * (cellsCount.x);
					cells[vertexIndex].fluid = fluid;
					//cells[vertexIndex].velocity = Velocity;
					//rightEdges[x + y * rightEdgesCount.x].velocity = Velocity;
				}
			}
		}
	}

	void Projection(float deltaTime);

	void Advection(float deltaTime);

	virtual void RemoveFluidAt(Vec2 fluidWorldPosition, float radius) override
	{
		AddFluidAt(defaultFluid, fluidWorldPosition, Vec::Zero(), radius);
	}

	void ApplyForces(float deltaTime);
	virtual void Update(float deltaTime) override;

	static VecInt GetCoordsFromIndex(int index, VecInt count) 
	{
		Vec2Int Coords;
		Coords.y = index / count.x;
		Coords.x = index - Coords.y * count.x;
		return Coords;
	}

	VecInt GetCellCoordsFromIndex(int index) const
	{
		return GetCoordsFromIndex(index, cellsCount);
	}

	Vec GetWorldPosFromCoords(VecInt coords) const
	{
		return (Vec2(coords) * cellSize) + worldPosition;
	}

	VecInt GetCellCoordsFromWorldPos(Vec cellWorldPos) const
	{
		return ((cellWorldPos - worldPosition) / cellSize).Floor();
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

		//m_mesh.Fill(vertices.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
		//{
		//	const Vertex& vertex = vertices[iVertex];

		//	Vec2 pos = GetWorldPosFromCoords(GetCoordsFromIndex(iVertex, verticesCount));
		//	x = pos.x;
		//	y = pos.y;
		//	if (std::shared_ptr<Fluid> fluid = vertex.fluid.lock())
		//	{
		//		r = fluid->color.x;
		//		g = fluid->color.y;
		//		b = fluid->color.z;
		//	}
		//});

		m_mesh.Draw();
	}
};

#endif