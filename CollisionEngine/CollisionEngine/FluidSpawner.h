#ifndef _FLUID_SPAWNER_H_
#define _FLUID_SPAWNER_H_

#include "Behavior.h"
#include "FluidSystem.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"
#include "Fluids/SPHMullerSystem.h"

class CFluidSpawner: public CBehavior
{
private:
	std::unique_ptr<IFluidSystem> fluidSystem = std::make_unique<SPHMullerSystem>();

	FluidSignature water;
	FluidSignature oil;

	FluidSignature* currentlyModifiedFluid = &water;

	float mouseRadius = 0.2;

	virtual void Start() override
	{
		water = NFluid::AddWater(*fluidSystem);
		oil = NFluid::AddOil(*fluidSystem);
	}

	virtual void Update(float frameTime) override
	{
		bool clicking = gVars->pRenderWindow->GetMouseButton(0);
		Vec2 mousePos = gVars->pRenderer->ScreenToWorldPos(gVars->pRenderWindow->GetMousePos());
		SPHMullerSystem::Fluid fluid = fluidSystem->FindOrAddFluid(*currentlyModifiedFluid);

		if (!m_clicking && clicking)
		{
			fluid.AddAt(mousePos, Vec2(0,0.5), 10, mouseRadius);

			//CFluidSystem::Get().Spawn(mousePos - Vec2(0.5f, 0.5f), mousePos + Vec2(0.5f, 0.5f), 10.0f, Vec2(15.0f, 15.0f));
		}
		if (gVars->pRenderWindow->GetMouseButton(2))
		{
			SPHMullerSystem::Fluid fluid = fluidSystem->FindOrAddFluid(water);
			fluid.RemoveAt(mousePos, mouseRadius);

			//CFluidSystem::Get().Clear();
		}

		m_clicking = clicking;

		//CFluidSystem::Get().Update(frameTime);
		fluidSystem->Update(frameTime);
	}

	bool m_clicking = false;
};


#endif