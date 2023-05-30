#include "FluidSystem.h"

#include "Renderer.h"
#include "GlobalVariables.h"

#include <string>


CFluidSystem::CFluidSystem()
{
	float particuleRadius = m_radius / m_particleRadiusRatio;
	float volume = particuleRadius * particuleRadius * (float)M_PI;
	m_mass = volume * m_restDensity;
	m_minRadius = m_radius * 0.1f;
}

void	CFluidSystem::SetBounds(const Vec2& min, const Vec2& max)
{
	m_min = min;
	m_max = max;
}

void	CFluidSystem::SpawnParticule(const Vec2& pos, const Vec2& vel)
{
	m_positions.push_back(pos);
	m_velocities.push_back(vel);
	m_accelerations.push_back(Vec2());
	m_densities.push_back(0.0f);
	m_pressures.push_back(0.0f);
	m_surfaceNormals.push_back(Vec2());
	m_surfaceCurvatures.push_back(0.0f);
	m_keys.push_back(0);
	m_proxies.push_back(SParticleProxy(m_proxies.size()));
}

void	CFluidSystem::Spawn(const Vec2& min, const Vec2& max, float particulesPerMeter, const Vec2& speed)
{
	float width = max.x - min.x;
	size_t horiCount = (size_t)(width * particulesPerMeter) + 1;

	float height = max.y - min.y;
	size_t vertiCount = (size_t)(height * particulesPerMeter) + 1;

	size_t count = horiCount * vertiCount;

	m_positions.reserve(m_positions.size() + count);
	m_velocities.reserve(m_velocities.size() + count);
	m_accelerations.reserve(m_accelerations.size() + count);
	m_densities.reserve(m_densities.size() + count);
	m_pressures.reserve(m_pressures.size() + count);

	for (size_t i = 0; i < horiCount; ++i)
	{
		for (size_t j = 0; j < vertiCount; ++j)
		{
			float x = min.x + ((float)i) / particulesPerMeter;
			float y = min.y + ((float)j) / particulesPerMeter;

			SpawnParticule(Vec2(x, y), speed);
		}
	}
}


void CFluidSystem::Update(float dt)
{
	gVars->pRenderer->DisplayText("Particules : " + std::to_string(m_positions.size()));

	dt *= m_timeScale;
	dt = Min(dt, 1.0f / (200.0f * m_timeScale)); // clamp d for stability, better have slow simulation than exploding simulation

	ResetAccelerations();

	FindContacts();
	ComputeDensity();
	//ComputePressure();
	//ComputeSurfaceTension();

	//AddPressureForces();
	//AddViscosityForces();

	ApplyForces(dt);
	Integrate(dt);

	BorderCollisions();

	FillMesh();
	m_mesh.Draw();
}

void	CFluidSystem::ResetAccelerations()
{
	for (Vec2& acc : m_accelerations)
	{
		acc = Vec2();
	}
}

void	CFluidSystem::ComputeKeys()
{
	float h = m_radius * 2;

	size_t shift = (sizeof(int) * 8) / 2;

	for (size_t i = 0; i < m_positions.size(); ++i)
	{
		Vec2 pos = m_positions[i];
		int key = (int)floor(pos.y / h) + (((int)floor(pos.x / h)) << shift);
		m_keys[i] = key;
	}
}

int		CFluidSystem::GetRightKey(int key)
{
	size_t shift = (sizeof(int) * 8) / 2;
	return key + (1 << shift);
}

int		CFluidSystem::GetTopKey(int key)
{
	return key + 1;
}


void	CFluidSystem::UpdateProxies()
{
	for (int a = m_proxies.size() - 2; a >= 0; --a)
	{
		// insert a in sorted [a + 1, last]
		SParticleProxy aProxy = m_proxies[a];
		size_t b = (size_t)a + 1;
		int aKey = m_keys[aProxy.i];
		while (b < m_proxies.size() && aKey > m_keys[m_proxies[b].i])
		{
			// swap a (which is in b-1 currently) and b proxies
			m_proxies[b - 1] = m_proxies[b];
			m_proxies[b] = aProxy;
			++b;
		}
	}
}

void	CFluidSystem::AddProxyContacts(size_t a)
{
	float h = m_radius;

	int aKey = m_keys[m_proxies[a].i];
	int topKey = GetTopKey(aKey);

	// iterate over all particles in same cell and cell just to the top
	size_t b = a + 1;
	for (; b < m_proxies.size() && m_keys[m_proxies[b].i] <= topKey; ++b)
	{
		AddContact(m_proxies[a].i, m_proxies[b].i, h);
	}

	int rightKey = GetRightKey(aKey);
	for (; b < m_proxies.size() && m_keys[m_proxies[b].i] < rightKey; ++b);

	int topRightKey = GetTopKey(rightKey);
	for (; b < m_proxies.size() && m_keys[m_proxies[b].i] <= topRightKey; ++b)
	{
		AddContact(m_proxies[a].i, m_proxies[b].i, h);
	}
}

void	CFluidSystem::AddContact(size_t i, size_t j, float h)
{
	const Vec2& iPos = m_positions[i];
	const Vec2& jPos = m_positions[j];

	float sqrLength = (iPos - jPos).GetSqrLength();

	if (sqrLength <= h * h)
	{
		float d = (iPos - jPos).GetLength();
		SParticleContact contact;
		contact.a = i;
		contact.b = j;
		contact.length = Clamp(d, m_minRadius, h);

		m_contacts.push_back(contact);
	}
}

void	CFluidSystem::FindContacts()
{
	//ComputeKeys();
	//UpdateProxies();

	m_contacts.clear();
	//for (size_t a = 0; a < m_proxies.size(); ++a)
	//{
	//	AddProxyContacts(a);
	//}

	for (int i = 0; i < m_positions.size(); i++)
	{
		for (int j = i + 1; j < m_positions.size(); j++)
		{
			AddContact(i, j, m_radius);
		}
	}
}

void	CFluidSystem::ComputeDensity()
{
	float radius = m_radius;
	float mass = m_mass;

	float baseWeight = KernelDefault(0.0f, radius);

	for (float& density : m_densities)
	{
		density = baseWeight;
	}

	for (SParticleContact& contact : m_contacts)
	{
		const Vec2& aPos = m_positions[contact.a];
		const Vec2& bPos = m_positions[contact.b];

		float weight = KernelDefault(contact.length, radius);
		m_densities[contact.a] += weight;
		m_densities[contact.b] += weight;
	}

	for (float& density : m_densities)
	{
		density *= mass;
	}
}

void	CFluidSystem::ComputePressure()
{
	for (size_t i = 0; i < m_pressures.size(); ++i)
	{
		m_pressures[i] = m_stiffness * (m_densities[i] - m_restDensity);
	}
}

void	CFluidSystem::ComputeSurfaceTension()
{
	float radius = m_radius * 1.5f;// 3.0f;
	float mass = m_mass;

	for (size_t i = 0; i < m_surfaceNormals.size(); ++i)
	{
		m_surfaceNormals[i] = Vec2();
		m_surfaceCurvatures[i] = -(mass / m_densities[i]) * KernelDefaultLaplacian(0.0f, radius);
	}

	for (SParticleContact& contact : m_contacts)
	{
		const Vec2& aPos = m_positions[contact.a];
		const Vec2& bPos = m_positions[contact.b];

		Vec2 r = aPos - bPos;
		Vec2 gradient = r * KernelDefaultGradientFactor(contact.length, radius);

		m_surfaceNormals[contact.a] += gradient * (mass / m_densities[contact.b]);
		m_surfaceNormals[contact.b] += gradient * -(mass / m_densities[contact.a]);

		float laplacian = KernelDefaultLaplacian(contact.length, radius);
		m_surfaceCurvatures[contact.a] += -(mass / m_densities[contact.b]) * laplacian;
		m_surfaceCurvatures[contact.b] += -(mass / m_densities[contact.a]) * laplacian;
	}

	float l = 0.5f; // 1f;
	for (size_t i = 0; i < m_surfaceNormals.size(); ++i)
	{
		float nSqrNorm = m_surfaceNormals[i].GetSqrLength();
		if (nSqrNorm >= l * l)
		{
			Vec2 tensionForce = m_surfaceNormals[i].Normalized() * m_surfaceCurvatures[i] * 5.0f;
			
			//m_accelerations[i] += tensionForce / m_densities[i];

			//gVars->pRenderer->DrawLine(m_positions[i], m_positions[i] + m_surfaceNormals[i].Normalized() * m_surfaceCurvatures[i] * -0.5f , 1, 0, 0);
		}


		//m_surfaceNormals[i].Normalize();
	//	gVars->pRenderer->DrawLine(m_positions[i], m_positions[i] + m_surfaceNormals[i] * m_surfaceCurvatures[i], 1, 0, 0);
		//m_surfaceNormals[i] = Vec2();
		//m_surfaceCurvatures[i] = 0.0f;
	}
}

void	CFluidSystem::AddPressureForces()
{
	float radius = m_radius;
	float mass = m_mass;

	for (SParticleContact& contact : m_contacts)
	{
		const Vec2& aPos = m_positions[contact.a];
		const Vec2& bPos = m_positions[contact.b];

		Vec2 r = aPos - bPos;
		float length = contact.length;

		Vec2 pressureAcc = r * -mass * ((m_pressures[contact.a] + m_pressures[contact.b]) / (2.0f * m_densities[contact.a] * m_densities[contact.b])) * KernelSpikyGradientFactor(length, radius);
		pressureAcc += r * 0.02f * mass * ((m_stiffness * (m_densities[contact.a] + m_densities[contact.b])) / (2.0f * m_densities[contact.a] * m_densities[contact.b])) * KernelSpikyGradientFactor(length * 0.8f, radius);
		m_accelerations[contact.a] += pressureAcc;
		m_accelerations[contact.b] -= pressureAcc;
	}
}

void	CFluidSystem::AddViscosityForces()
{
	float radius = m_radius;
	float mass = m_mass;
	float viscosity = m_viscosity;

	for (SParticleContact& contact : m_contacts)
	{
		const Vec2& aPos = m_positions[contact.a];
		const Vec2& bPos = m_positions[contact.b];

		Vec2 deltaVel = m_velocities[contact.a] - m_velocities[contact.b];
		Vec2 viscosityAcc = deltaVel * -mass * (viscosity / (2.0f * m_densities[contact.a] * m_densities[contact.b])) * KernelViscosityLaplacian(contact.length, radius);

		m_accelerations[contact.a] += viscosityAcc;
		m_accelerations[contact.b] -= viscosityAcc;
	}
}


void	CFluidSystem::BorderCollisions()
{
	const float restitution = m_wallRestitution;
	const float friction = m_wallFriction;

	for (size_t i = 0; i < m_positions.size(); ++i)
	{
		Vec2& pos = m_positions[i];
		//if (pos.x <= m_min.x && m_velocities[i].x < 0.0f)
		//{
		//	pos.x = m_min.x;
		//	m_velocities[i].x *= -restitution;
		//	m_velocities[i].y *= friction;
		//}
		//else if (pos.x >= m_max.x && m_velocities[i].x > 0.0f)
		//{
		//	pos.x = m_max.x;
		//	m_velocities[i].x *= -restitution;
		//	m_velocities[i].y *= friction;
		//}

		if (pos.y <= m_min.y  && m_velocities[i].y < 0.0f)
		{
			pos.y = m_min.y;
			m_velocities[i].y *= -restitution;
			m_velocities[i].x *= friction;
		}
		//else if (pos.y >= m_max.y  && m_velocities[i].y > 0.0f)
		//{
		//	pos.y = m_max.y;
		//	m_velocities[i].y *= -restitution;
		//	m_velocities[i].x *= friction;
		//}
	}
}

void	CFluidSystem::ApplyForces(float dt)
{
	ClampArray(m_accelerations, m_maxAcceleration);

	Vec2 gravity(0.0f, -5);// -9.8f);
	for (size_t i = 0; i < m_velocities.size(); ++i)
	{
		m_velocities[i] += (m_accelerations[i] + gravity) * dt;
	}
}

void	CFluidSystem::Integrate(float dt)
{
	ClampArray(m_velocities, m_maxSpeed);
	for (size_t i = 0; i < m_positions.size(); ++i)
	{
		m_positions[i] += m_velocities[i] * dt;
	}
}

void	CFluidSystem::ClampArray(std::vector<Vec2>& array, float limit)
{
	for (Vec2& vec : array)
	{
		if (vec.GetSqrLength() > limit * limit)
		{
			vec *= limit / vec.GetLength();
		}
	}
}

void	CFluidSystem::FillMesh()
{
	m_mesh.Fill(m_positions.size(), [&](size_t iVertex, float& x, float& y, float& r, float& g, float& b)
	{
		const Vec2& pos = m_positions[iVertex];
		x = pos.x;
		y = pos.y;
		r = 0.0f;
		g = 0.0f;
		b = 1.0f;
	});
}


float KernelDefault(float r, float h)
{
	float h2 = h * h;
	float h4 = h2 * h2;
	float kernel = h2 - r * r;
	return (kernel * kernel * kernel) * (4.0f / (((float)M_PI) * h4 * h4));
}

float KernelDefaultGradientFactor(float r, float h)
{
	float h2 = h * h;
	float h4 = h2 * h2;
	float kernel = h2 - r * r;
	return -(kernel * kernel) * (6.0f / (((float)M_PI) * h4 * h4));
}

float KernelDefaultLaplacian(float r, float h)
{
	float h2 = h * h;
	float h4 = h2 * h2;
	float kernel = h2 - r * r;
	return -(kernel * kernel) * (6.0f / (((float)M_PI) * h4 * h4)) * (3.0f * h2 - 7.0f * r * r);
}


float KernelSpikyGradientFactorNorm(float r, float h)
{
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float kernel = h - r;
	return kernel * kernel * (-15.0f / ((float)M_PI * h5));
}

float KernelSpikyGradientFactor(float r, float h)
{
	float h2 = h * h;
	float h5 = h2 * h2 * h;
	float kernel = h - r;
	return kernel * kernel * (-15.0f / ((float)M_PI * h5 * r));
}

float KernelViscosityLaplacian(float r, float h)
{
	float h2 = h * h;
	float kernel = h - r;
	return kernel * (30.0f / ((float)M_PI * h2 * h2 * h));
}

float KernelPoly6hGradientFactor(float r, float h)
{
	float h2 = h * h;
	float kernel = h2 - r * r;
	return kernel * kernel * (24.0f / ((float)M_PI * h2 * h2 * h2 * h * r));
}
