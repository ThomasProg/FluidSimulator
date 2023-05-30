#ifndef _OOP_FLUID_HPP_
#define _OOP_FLUID_HPP_

#include "Maths.h"

struct Fluid
{
	// Rendering
	Vec3 color = Vec3(1,0,0);

	// Physics Properties
	float volumicMass = 1.f; /* density */
	float viscosity = 0.f;
};

inline Fluid GetAir()
{
	Fluid fluid;
	fluid.color = { 0.9,0.9,0.9 };
	fluid.viscosity = 0.0f;
	fluid.volumicMass = 1.204f; /* 20°C   kg/m^3*/
	return fluid;
}


inline Fluid GetWater()
{
	Fluid fluid;
	fluid.color = { 0,0.1,0.6 };
	fluid.viscosity = 0.1f;// 1.0f;
	fluid.volumicMass = 1.f; /* 20°C   kg/m^3*/
	return fluid;
}

inline Fluid GetLava()
{
	Fluid fluid;
	fluid.color = { 1,0.0,0.0 };
	fluid.viscosity = 5.0f;
	fluid.volumicMass = 1.f; /* 20°C   kg/m^3*/
	return fluid;
}

inline Fluid GetOil()
{
	Fluid fluid;
	fluid.color = { 0.4,0.4,0.0 };
	fluid.viscosity = 0.1f;// 1.7f;
	fluid.volumicMass = 881; /* 20°C   kg/m^3*/
	return fluid;
}


#endif