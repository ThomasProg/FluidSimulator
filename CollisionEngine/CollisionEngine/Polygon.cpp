#include "Polygon.h"
#include <GL/glu.h>

#include "PhysicEngine.h"
#include "NarrowPhases/SeparatingAxisTest.h"
#include "NarrowPhases/GilbertJohnsonKeerthi.h"
#include "NarrowPhases/ExpandingPolytopeAlgorithm.h"

#include "InertiaTensor.h"


CPolygon::CPolygon(size_t index)
	: m_vertexBufferId(0), m_index(index), density(0.1f)
{
}

CPolygon::~CPolygon()
{
	DestroyBuffers();
}

void CPolygon::Build()
{
	m_lines.clear();

	//if (points.size() > 2)
	//{
	//	Vec2 edge = points[0] - points[1];
	//	if (Vec2::Dot(-points[0], edge.GetNormal()) > 0)
	//	{
	//		std::reverse(points.begin(), points.end());
	//	}
	//}

	ComputeArea();
	RecenterOnCenterOfMass();
	ComputeLocalInertiaTensor();

	CreateBuffers();
	BuildLines();

	OnTransformUpdated();
}

void CPolygon::Draw()
{
	// Set transforms (qssuming model view mode is set)
	float transfMat[16] = {	rotation.X.x, rotation.X.y, 0.0f, 0.0f,
							rotation.Y.x, rotation.Y.y, 0.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f,
							position.x, position.y, -1.0f, 1.0f };
	glPushMatrix();
	glMultMatrixf(transfMat);

	switch (collisionState)
	{
		case CollisionState::NOT_COLLIDING:
			glColor3f(0.0, 0.0, 0.0);
			break;
		case CollisionState::BROAD_PHASE_SUCCESS:
			glColor3f(0.0, 0.8, 0.0);
			break;

		case CollisionState::NARROW_PHASE_SUCCESS:
			glColor3f(0.0, 0.0, 0.8);
			break;
	}

	// Draw vertices
	BindBuffers();
	glDrawArrays(GL_LINE_LOOP, 0, points.size());
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
}

size_t	CPolygon::GetIndex() const
{
	return m_index;
}

float	CPolygon::GetArea() const
{
	return fabsf(m_signedArea);
}

Circle CPolygon::GetCircle() const
{
	Circle circle;
	circle.point = Vec2(0, 0);
	
	// compute medium point
	for (const Vec2& p : points)
	{
		circle.point += p;
	}
	circle.point /= points.size();

	// compute max radius
	float sqrLengthMax = 0.f;
	for (const Vec2& p : points)
	{
		circle.point += p;
		float sqrLength = (circle.point - p).GetSqrLength();

		sqrLengthMax = Max(sqrLength, sqrLengthMax);
	}
	circle.radius = sqrt(sqrLengthMax);
	
	return circle;
}

void CPolygon::OnTransformUpdated()
{
	if (onTransformUpdatedCallback) 
	{
		onTransformUpdatedCallback(*this); 
	}
}

Vec2	CPolygon::TransformPoint(const Vec2& point) const
{
	return position + rotation * point;
}

Vec2	CPolygon::InverseTransformPoint(const Vec2& point) const
{
	return rotation.GetInverse() * (point - position);
}

bool	CPolygon::IsPointInside(const Vec2& point) const
{
	float maxDist = -FLT_MAX;

	for (const Line& line : m_lines)
	{
		Line globalLine = line.Transform(rotation, position);
		float pointDist = globalLine.GetPointDist(point);
		maxDist = Max(maxDist, pointDist);
	}

	return maxDist <= 0.0f;
}

void CPolygon::UpdateTransformedPoints()
{
	transformedPoints.resize(points.size());
	for (int i = 0; i < points.size(); i++)
	{
		transformedPoints[i] = rotation * points[i] + position;
	}

	transformedPointsDirty = false;
}

const std::vector<Vec2>& CPolygon::GetWorldNormals() const
{
	if (worldNormalsDirty)
	{
		// TODO : Mutex if multithread
		static_assert(!USING_THREADS);
		CPolygon* poly = const_cast<CPolygon*>(this);
		poly->UpdateWorldNormals();
		poly->worldNormalsDirty = false;
	}
	return worldNormals;
}

void CPolygon::Setposition(Vec2 newPos)
{
	SetPositionUnsafe(newPos);
	OnTransformUpdated();
}

const std::vector<Vec2>& CPolygon::GetWorldPoints() const
{
	if (transformedPointsDirty)
	{
		// TODO : Mutex if multithread
		static_assert(!USING_THREADS);
		CPolygon* poly = const_cast<CPolygon*>(this);
		poly->UpdateTransformedPoints();
	}
	return transformedPoints;
}

bool	CPolygon::IsLineIntersectingPolygon(const Line& line, Vec2& colPoint, float& colDist) const
{
	//float dist = 0.0f;
	float minDist = FLT_MAX;
	Vec2 minPoint;
	float lastDist = 0.0f;
	bool intersecting = false;

	for (const Vec2& point : points)
	{
		Vec2 globalPoint = TransformPoint(point);
		float dist = line.GetPointDist(globalPoint);
		if (dist < minDist)
		{
			minPoint = globalPoint;
			minDist = dist;
		}

		intersecting = intersecting || (dist != 0.0f && lastDist * dist < 0.0f);
		lastDist = dist;
	}

	if (minDist <= 0.0f)
	{
		colDist = -minDist;
		colPoint = minPoint;
	}
	return (minDist <= 0.0f);
}

void CPolygon::ApplyImpulse(const Vec2& point, const Vec2& impulse)
{
	if (IsStatic())
		return;

	speed += impulse * invMass;
	Vec3 deltaAngularVelocity = invWorldTensor * Vec3(0.f, 0.f, Vec2::Cross(point - Getposition(), impulse));
	angularVelocity += deltaAngularVelocity.z;
}

void CPolygon::ApplyDirectImpulse(const Vec2& point, const Vec2& impulse)
{
	if (IsStatic())
		return;

	Setposition(Getposition() + impulse * invMass);
	//Vec3 deltaAngularVelocity = invWorldTensor * Vec3(0.f, 0.f, Vec2::Cross(point - Getposition(), impulse));
	//rotation.Rotate(RAD2DEG(deltaAngularVelocity.z));
}

void CPolygon::ApplyForceLocal(const Vec2& localPoint, const Vec2& force)
{
	if (IsStatic())
		return;

	forces += force;
	torques += Vec2::Cross(localPoint, force);
}

void CPolygon::ApplyForce(const Vec2& point, const Vec2& force)
{
	ApplyForceLocal(point - Getposition(), force);
}

void CPolygon::UpdateSpeed(float deltaTime)
{
	if (IsStatic())
		return;

	speed += forces * (invMass * deltaTime);
	invWorldTensor = Mat3(rotation) * invLocalTensor * Mat3(rotation.GetInverse());
	//angularVelocity += Mat3(invWorldTensor) * Vec3(0, 0, torques) * deltaTime;
}

void CPolygon::GetAllNormals(std::vector<Vec2>& outNormals) const
{
	GetWorldPoints();
	assert(transformedPoints.size() > 1);

	outNormals.clear();
	outNormals.reserve(transformedPoints.size());
	for (int i = 0; i < transformedPoints.size() - 1; i++)
	{
		outNormals.push_back(GetNormalBetween2Points(transformedPoints[i], transformedPoints[i + 1]));
	}

	outNormals.push_back(GetNormalBetween2Points(transformedPoints[transformedPoints.size() - 1], transformedPoints[0]));
}

void CPolygon::UpdateWorldNormals()
{
	GetAllNormals(worldNormals);
}

bool	CPolygon::CheckCollision(const CPolygon& poly, Vec2& colPoint, Vec2& colNormal, float& colDist) const
{
	SeparatingAxisTest sat;
	return sat.CheckCollision(*this, poly, colPoint, colNormal, colDist);

	//GilbertJohnsonKeerthi gjk;
	//return gjk.CheckCollision(*this, poly);

	//ExpandingPolytopeAlgorithm epa;
	//bool b = epa.CheckCollision(*this, poly, colPoint, colNormal, colDist);
	//colNormal.Normalize();
	//return b;

	//SeparatingAxisTest sat;
	//Vec2 colPoint2; Vec2 colNormal2; float colDist2;
	//if (sat.CheckCollision(*this, poly, colPoint, colNormal, colDist))
	//{
	//	ExpandingPolytopeAlgorithm epa;
	//	if (epa.CheckCollision(*this, poly, colPoint2, colNormal2, colDist2))
	//	{
	//		//assert(IsNearlyEqual(colPoint2, colPoint) && IsNearlyEqual(colNormal2, colNormal) && IsNearlyEqual(colDist2, colDist));
	//		assert(IsNearlyEqual(colNormal2, colNormal) && IsNearlyEqual(colDist2, colDist));

	//		return true;
	//	}
	//}
	//return false;
}

float CPolygon::GetMass() const
{
	return density * GetArea();
}

float CPolygon::GetInertiaTensor() const
{
	return m_localInertiaTensor * GetMass();
}

Vec2 CPolygon::GetPointVelocity(const Vec2& point) const
{
	return speed + (point - position).GetNormal() * angularVelocity;
}


void CPolygon::CreateBuffers()
{
	DestroyBuffers();

	float* vertices = new float[3 * points.size()];
	for (size_t i = 0; i < points.size(); ++i)
	{
		vertices[3 * i] = points[i].x;
		vertices[3 * i + 1] = points[i].y;
		vertices[3 * i + 2] = 0.0f;
	}

	glGenBuffers(1, &m_vertexBufferId);

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * points.size(), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] vertices;
}

void CPolygon::BindBuffers()
{
	if (m_vertexBufferId != 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (void*)0);
	}
}


void CPolygon::DestroyBuffers()
{
	if (m_vertexBufferId != 0)
	{
		glDeleteBuffers(1, &m_vertexBufferId);
		m_vertexBufferId = 0;
	}
}

void CPolygon::BuildLines()
{
	for (size_t index = 0; index < points.size(); ++index)
	{
		const Vec2& pointA = points[index];
		const Vec2& pointB = points[(index + 1) % points.size()];

		Vec2 lineDir = (pointA - pointB).Normalized();

		m_lines.push_back(Line(pointB, lineDir));
	}
}


void CPolygon::ComputeArea()
{
	m_signedArea = 0.0f;
	for (size_t index = 0; index < points.size(); ++index)
	{
		const Vec2& pointA = points[index];
		const Vec2& pointB = points[(index + 1) % points.size()];
		m_signedArea += pointA.x * pointB.y - pointB.x * pointA.y;
	}
	m_signedArea *= 0.5f;
}

void CPolygon::RecenterOnCenterOfMass()
{
	Vec2 centroid;
	for (size_t index = 0; index < points.size(); ++index)
	{
		const Vec2& pointA = points[index];
		const Vec2& pointB = points[(index + 1) % points.size()];
		float factor = pointA.x * pointB.y - pointB.x * pointA.y;
		centroid.x += (pointA.x + pointB.x) * factor;
		centroid.y += (pointA.y + pointB.y) * factor;
	}
	centroid /= 6.0f * m_signedArea;

	for (Vec2& point : points)
	{
		point -= centroid;
	}
	position += centroid;
}

void CPolygon::ComputeLocalInertiaTensor()
{
	m_localInertiaTensor = 0.0f;
	for (size_t i = 0; i + 1 < points.size(); ++i)
	{
		const Vec2& pointA = points[i];
		const Vec2& pointB = points[i + 1];

		m_localInertiaTensor += ComputeInertiaTensor_Triangle(Vec2(), pointA, pointB);
	}
}

