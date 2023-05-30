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
	std::shared_ptr<Fluid> water = std::make_unique<Fluid>(GetWater());
	std::shared_ptr<Fluid> lava = std::make_unique<Fluid>(GetOil());

	float cellSize = 0.4f;
	float mouseRadius = 0.5;//cellSize * 3;

	int id = 0;

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
			switch (id)
			{
			case 0:
				fluidSystem->AddFluidAt(water, mousePos + Vec2(0.0, 0), Vec2::Zero(), mouseRadius);
				break;
				//CFluidSystem::Get().Spawn(mousePos - Vec2(0.5f, 0.5f), mousePos + Vec2(0.5f, 0.5f), 10.0f, Vec2(0.0f, 0.0f));
			case 1:
				fluidSystem->AddFluidAt(lava, mousePos + Vec2(0.0, 0), Vec2::Zero(), mouseRadius);
				break;
			}

			id++;
			if (id > 1)
				id = 0;
		}
		if (gVars->pRenderWindow->GetMouseButton(2))
		{
			fluidSystem->RemoveFluidAt(mousePos, mouseRadius);
		}

		m_clicking = clicking;

		fluidSystem->Update(frameTime);
		//CFluidSystem::Get().Update(frameTime);
	}

	bool m_clicking = false;
};
//
//class CFluidSpawner : public CBehavior
//{
//private:
//	virtual void Update(float frameTime) override
//	{
//		bool clicking = gVars->pRenderWindow->GetMouseButton(0);
//		if (!m_clicking && clicking)
//		{
//			Vec2 mousePoint = gVars->pRenderer->ScreenToWorldPos(gVars->pRenderWindow->GetMousePos());
//			CFluidSystem::Get().Spawn(mousePoint - Vec2(0.5f, 0.5f), mousePoint + Vec2(0.5f, 0.5f), 10.0f, Vec2(15.0f, 15.0f));
//		}
//		if (gVars->pRenderWindow->GetMouseButton(1))
//		{
//			//CFluidSystem::Get().Clear();
//		}
//
//		m_clicking = clicking;
//
//		CFluidSystem::Get().Update(frameTime);
//	}
//
//	bool m_clicking = false;
//};


#endif