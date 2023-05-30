#ifndef __FLUID_SYSTEM_H__
#define __FLUID_SYSTEM_H__

#include "FluidMesh.h"
#include "Maths.h"

#include <vector>
#include <string>

float KernelDefault(float r, float h);
float KernelDefaultGradientFactor(float r, float h);
float KernelDefaultLaplacian(float r, float h);
float KernelSpikyGradientFactorNorm(float r, float h);
float KernelSpikyGradientFactor(float r, float h);
float KernelViscosityLaplacian(float r, float h);
float KernelPoly6hGradientFactor(float r, float h);


struct SParticleContact
{
	size_t	a, b;
	float	length;
};

struct SParticleProxy
{
	SParticleProxy(size_t _i) : i(_i){}

	size_t i;
};

class CFluidSystem
{
public:
	static	CFluidSystem&	Get()
	{
		static CFluidSystem instance;
		return instance;
	}

private:
	CFluidSystem();
	CFluidSystem(const CFluidSystem&);
	CFluidSystem& operator=(const CFluidSystem&);


public:
	void	SetBounds(const Vec2& min, const Vec2& max);
	void	SpawnParticule(const Vec2& pos, const Vec2& vel);
	void	Spawn(const Vec2& min, const Vec2& max, float particulesPerMeter, const Vec2& speed);
	void	Update(float dt);

private:
	void	ResetAccelerations();

	void	ComputeKeys();
	int		GetRightKey(int key);
	int		GetTopKey(int key);
	void	UpdateProxies();
	void	AddProxyContacts(size_t a);
	void	AddContact(size_t i, size_t j, float h);
	void	FindContacts();

	void	ComputeDensity();
	void	ComputePressure();
	void	ComputeSurfaceTension();
	void	AddPressureForces();
	void	AddViscosityForces();
	void	BorderCollisions();



	void	ApplyForces(float dt);
	void	Integrate(float dt);

	void	ClampArray(std::vector<Vec2>& array, float limit);
	void	FillMesh();

	float				m_radius = 0.1f;
	float				m_minRadius;
	float				m_restDensity = 0.59f;
	float				m_stiffness = 500.0f;
	float				m_particleRadiusRatio = 3.0f;
	float				m_viscosity = 0.1f;
	float				m_maxSpeed = 10.0f;
	float				m_maxAcceleration = 900.0f;
	float				m_timeScale = 1.0f; // use this to make simulation more stable
	float				m_wallFriction = 0.4f;
	float				m_wallRestitution = 0.4f;

	float				m_mass;

	std::vector<Vec2>	m_positions;
	std::vector<Vec2>	m_accelerations;
	std::vector<Vec2>	m_velocities;
	std::vector<float>	m_densities;
	std::vector<float>	m_pressures;
	std::vector<Vec2>	m_surfaceNormals;
	std::vector<float>	m_surfaceCurvatures;
	std::vector<int>	m_keys;
	std::vector<SParticleProxy>	m_proxies;


	std::vector<SParticleContact>	m_contacts;

	Vec2		m_min, m_max;
	CFluidMesh	m_mesh;
};

#endif