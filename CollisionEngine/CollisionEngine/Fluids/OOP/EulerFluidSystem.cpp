#include "EulerFluidSystem.hpp"

void EulerFluidSystem::ResetPressure()
{
	for (Cell& cell : cells)
	{
		cell.pressure = 0.f;
	}
}

void EulerFluidSystem::ApplyForces(float deltaTime)
{
	const Vec acceleration = Vec{3, -9.8} *0.01;
	for (Vertex& edge : rightEdges)
	{
		edge.velocity = acceleration;
	}
	for (Vertex& edge : upEdges)
	{
		edge.velocity = acceleration;
	}
}

void EulerFluidSystem::Projection(float deltaTime)
{
	ResetPressure();

	int n = 5;
	for (int i = 0; i < n; i++)
	{
		for (int y = 0; y < cellsCount.y; y++)
		{
			for (int x = 0; x < cellsCount.x; x++)
			{
				Cell& cell = cells[x + y * cellsCount.x];

				Vertex* rightEdge = nullptr;
				Vertex* upEdge = nullptr;
				Vertex* downEdge = nullptr;
				Vertex* leftEdge = nullptr;

				int nbEdges = 0;
				constexpr Vec zero = Vec::Zero();
				Vec divergence = zero;
				Vec velocity = zero;

				if (x < rightEdgesCount.x)
				{
					rightEdge = &rightEdges[x + y * rightEdgesCount.x];
					nbEdges++;
					divergence.x += rightEdge->velocity.x;
					velocity += rightEdge->velocity;
				}

				if (x > 0)
				{
					leftEdge = &rightEdges[(x - 1) + y * rightEdgesCount.x];
					nbEdges++;
					divergence.x -= leftEdge->velocity.x;
					velocity += leftEdge->velocity;
				}

				if (y < upEdgesCount.y)
				{
					upEdge = &upEdges[x + y * upEdgesCount.x];
					nbEdges++;
					divergence.y += upEdge->velocity.y;
					velocity += upEdge->velocity;
				}

				if (y > 0)
				{
					downEdge = &upEdges[x + (y - 1) * upEdgesCount.x];
					nbEdges++;
					divergence.y -= downEdge->velocity.y;
					velocity += downEdge->velocity;
				}

				divergence *= 1.9; // overrelaxation

				if (rightEdge)
				{
					rightEdge->velocity.x -= divergence.x / nbEdges;
				}
				if (upEdge)
				{
					upEdge->velocity.y -= divergence.y / nbEdges;
				}

				if (leftEdge)
				{
					leftEdge->velocity.x += divergence.x / nbEdges;
				}
				if (downEdge)
				{
					downEdge->velocity.y += divergence.y / nbEdges;
				}

				float density = 1.f;
				cell.pressure += divergence.GetLength() / nbEdges * density * cellSize.x /* arbitrary */ / deltaTime;
			
				cell.velocity = velocity / nbEdges;
			}
		}
	}
}

void EulerFluidSystem::Advection(float deltaTime)
{
	static std::vector<Cell> oldCells;
	oldCells.resize(cells.size());
	ResetCells(oldCells);

	for (int y = 0; y < cellsCount.y; y++)
	{
		for (int x = 0; x < cellsCount.x; x++)
		{
			Cell& cell = cells[x + y * cellsCount.x];

			if (cell.fluid.lock().get() != defaultFluid.get())
			{

				Vec oldWorldPos = GetWorldPosFromCoords(Vec2Int{ x, y });
				Vec newWorldPos = oldWorldPos + cell.velocity * deltaTime;

				VecInt coords = GetCellCoordsFromWorldPos(newWorldPos);

				if (coords.x >= 0 && coords.x < cellsCount.x && coords.y >= 0 && coords.y < cellsCount.y)
				{
					Cell& newCell = oldCells[coords.x + coords.y * cellsCount.x];
					newCell.fluid = cell.fluid;
					newCell.pressure += cell.pressure;
				}
				else
				{
					cell.fluid = errorFluid;
				}
			}
			else
			{
				Vec oldWorldPos = GetWorldPosFromCoords(Vec2Int{ x, y });
				Vec newWorldPos = oldWorldPos + cell.velocity * deltaTime;
				newWorldPos.x += 10;
			}
		}
	}

	cells = std::move(oldCells);
}

void EulerFluidSystem::Update(float deltaTime)
{
	//deltaTime = 1.f / 30.f;

	ApplyForces(deltaTime);

	Projection(deltaTime);

	Advection(deltaTime);

	Draw();
}