#ifndef _FLUID_H_
#define _FLUID_H_

#include "Behavior.h"
#include "FluidSystem.h"
#include "GlobalVariables.h"
#include "Renderer.h"
#include "RenderWindow.h"
#include "World.h"
#include "DataOrientedHelpers.h"

// A Handle always valid
IMPLEMENT_KEY(FluidSignature, fluidID, friend class IFluidSystem;)

class IFluidSystem
{
protected:
	Vec2 gravity = Vec2(0.0f, -9.8f);

#pragma region FluidData
public:
	class Fluid;

	struct FluidPhysicalProperties
	{
		// ADD HERE NEW PROPERTIES //

		float viscosity = 0.1f;
	};

	struct FluidVisualProperties
	{
		// ADD HERE NEW PROPERTIES //

		Vec3 color = Vec3(0, 0, 1);
	};

protected:
	// ADD FLUID FIELDS HERE //
	// If you add a field, make sure to :
	// - Emplace it in AddFluid()
	// - Remove it RemoveFluid()
	// - Add a getter in the Fluid class
	std::vector<FluidSignature> fluidSignatures;
	std::vector<FluidPhysicalProperties> fluidPhysicalProperties;
	std::vector<FluidVisualProperties> fluidVisualProperties;
#pragma endregion

#pragma region FluidSystem
public:
	virtual void Update(float deltaTime) = 0;

	virtual Fluid AddFluid()
	{
		Fluid fluid = MakeFluid(fluidSignatures.size());

		FluidSignature newFluidSignature;
		newFluidSignature.fluidID = fluidSignatures.size();
		fluidSignatures.push_back(newFluidSignature);
		
		fluidPhysicalProperties.emplace_back();
		fluidVisualProperties.emplace_back();

		return fluid;
	}

	virtual void RemoveFluid(Fluid fluid)
	{
		unsigned int index = fluid.GetIndex();
		QuickRemove(fluidPhysicalProperties, index);
		QuickRemove(fluidVisualProperties, index);
		QuickRemove(fluidSignatures, index);
	}

	GENERATE_ALL_UTILITY_MACROS(Fluid, fluidSignatures);

	virtual void AddFluidAt(Fluid fluid, Vec2 pos, Vec2 velocity, unsigned int amount, float radius) = 0;
	virtual void RemoveFluidAt(Fluid fluid, Vec2 pos, float radius) = 0;
#pragma endregion

#pragma region FluidAccessor
public:
	// Utility class & functions to access Fluid attributes
	// An instance of this class can become invalid after removing a fluid from the linked system.
	// It is recommended to regenerate an instance each time you want to modify something.
	class Fluid
	{
	protected:
		GENERATE_REDIRECTOR(Fluid, IFluidSystem, FluidSignature, fluidSignatures, MakeFluid);

	public:
		GENERATE_REDIRECTOR_GETTER(PhysicalProperties, fluidPhysicalProperties);
		GENERATE_REDIRECTOR_GETTER(VisualProperties, fluidVisualProperties);

		inline void AddAt(Vec2 pos, Vec2 velocity, unsigned int amount, float radius)
		{
			assert(IsValid());
			return system->AddFluidAt(*this, pos, velocity, amount, radius);
		}

		inline void RemoveAt(Vec2 pos, float radius)
		{
			assert(IsValid());
			return system->RemoveFluidAt(*this, pos, radius);
		}
	};
#pragma endregion
};

#pragma region FluidUtilities
// UTILITY
namespace NFluid
{
	inline IFluidSystem::Fluid AddAir(IFluidSystem& system)
	{
		IFluidSystem::Fluid fluid = system.AddFluid();
		fluid.GetVisualPropertiesRef().color = { 0.6,0.6,0.6 };

		IFluidSystem::FluidPhysicalProperties& physicalAttributes = fluid.GetPhysicalPropertiesRef();
		physicalAttributes.viscosity = 0.0f;

		return fluid;
	}


	inline IFluidSystem::Fluid AddWater(IFluidSystem& system)
	{
		IFluidSystem::Fluid fluid = system.AddFluid();
		fluid.GetVisualPropertiesRef().color = {0,0.1,0.6};

		IFluidSystem::FluidPhysicalProperties& physicalAttributes = fluid.GetPhysicalPropertiesRef();
		physicalAttributes.viscosity = 1.0f;

		return fluid;
	}

	inline IFluidSystem::Fluid AddOil(IFluidSystem& system)
	{
		IFluidSystem::Fluid fluid = system.AddFluid();
		fluid.GetVisualPropertiesRef().color = { 0.4,0.4,0.0 };

		IFluidSystem::FluidPhysicalProperties& physicalAttributes = fluid.GetPhysicalPropertiesRef();
		physicalAttributes.viscosity = 1.7f;

		return fluid;
	}
}

#pragma endregion

#endif