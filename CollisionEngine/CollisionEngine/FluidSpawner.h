#ifndef _FLUID_SPAWNER_H_
#define _FLUID_SPAWNER_H_

#include "Behavior.h"
#include "FluidSystem.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"
//#include "Fluids/SPHMullerSystem.h"
//#include "Fluids/EulerSystem.h"
#include "Fluids/OOP/EulerFluidSystem.hpp"
#include "Fluids/OOP/SPHMullerFluidSystem.hpp"

class CFluidSpawner: public CBehavior
{
private:
	//std::unique_ptr<IFluidSystem> fluidSystem = std::make_unique<SPHMullerSystem>();
	std::unique_ptr<IFluidSystem> fluidSystem = std::make_unique<SPHMullerFluidSystem>();
	std::shared_ptr<Fluid> water = std::make_unique<Fluid>(GetLava());

	float cellSize = 0.4f;
	float mouseRadius = 0.5;//cellSize * 3;

	virtual void Start() override
	{
		InitSystem(fluidSystem.get());
	}

	void InitSystem(IFluidSystem* system)
	{
		if (EulerFluidSystem* eulerSys = dynamic_cast<EulerFluidSystem*>(system))
		{
			eulerSys->Reset(Vec2{ 0,0 }, Vec2Int{ 200,200 }, Vec2{ cellSize, cellSize });
		}

		if (SPHMullerFluidSystem* eulerSys = dynamic_cast<SPHMullerFluidSystem*>(system))
		{
			// TODO: Init

			//eulerSys->Reset(Vec2{ 0,0 }, Vec2Int{ 200,200 }, Vec2{ cellSize, cellSize });
		}
	}

	virtual void Update(float frameTime) override
	{
		bool clicking = gVars->pRenderWindow->GetMouseButton(0);
		Vec2 mousePos = gVars->pRenderer->ScreenToWorldPos(gVars->pRenderWindow->GetMousePos());

		if (!m_clicking && clicking)
		{
			fluidSystem->AddFluidAt(water, mousePos, Vec2::Zero(), mouseRadius);
		}
		if (gVars->pRenderWindow->GetMouseButton(2))
		{
			fluidSystem->RemoveFluidAt(mousePos, mouseRadius);
		}

		m_clicking = clicking;

		fluidSystem->Update(frameTime);
	}

	bool m_clicking = false;
};


#endif