#include "Polygon.h"
#include <GL/glu.h>


#include "PhysicEngine.h"

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

	CreateBuffers();
	BuildLines();

	centeredAABB = ConstructCenteredAABB();
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

AABB CPolygon::ConstructCenteredAABB() const
{
	if (points.size() == 0)
		return AABB();

	AABB aabb (points[0], points[0]);

	for (const Vec2& p : points)
	{
		// TODO : remove first iteration?
		aabb.pMin.x = Min(aabb.pMin.x, p.x);
		aabb.pMin.y = Min(aabb.pMin.y, p.y);
		aabb.pMax.x = Max(aabb.pMax.x, p.x);
		aabb.pMax.y = Max(aabb.pMax.y, p.y);
	}

	return aabb;
}

void CPolygon::OnTransformUpdated()
{
	transformedAABB = AABB(centeredAABB.pMin + position, centeredAABB.pMax + position);
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

bool	CPolygon::CheckCollision(const CPolygon& poly, Vec2& colPoint, Vec2& colNormal, float& colDist) const
{
	return false;
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


