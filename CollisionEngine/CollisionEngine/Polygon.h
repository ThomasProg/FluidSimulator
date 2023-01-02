#ifndef _POLYGON_H_
#define _POLYGON_H_

#include <GL/glew.h>
#include <vector>
#include <memory>
#include <functional>


#include "Maths.h"

enum class CollisionState
{
	NOT_COLLIDING,
	BROAD_PHASE_SUCCESS,
	NARROW_PHASE_SUCCESS
};

class CPolygon
{
private:
	friend class CWorld;

	CPolygon(size_t index);
public:
	~CPolygon();

private:
	Vec2				position;
	Mat2				rotation;

	bool worldNormalsDirty = true; 
	std::vector<Vec2>	worldNormals;
	void				UpdateWorldNormals();

	std::vector<Vec2>	transformedPoints;
	bool transformedPointsDirty = true;
	void					 UpdateTransformedPoints();

public:
	const std::vector<Vec2>& GetWorldNormals() const;
	const std::vector<Vec2>& GetWorldPoints() const;

	CollisionState collisionState = CollisionState::NOT_COLLIDING;

	std::function<void(const CPolygon&)> onTransformUpdatedCallback;

	GETTER(position)
	//SETTER(position, OnTransformUpdated)
	void Setposition(Vec2 newPos);

	// You have to call OnTransformUpdated() after calling this function
	void SetPositionUnsafe(Vec2 newPos)
	{
		position = newPos;
		transformedPointsDirty = true;
		worldNormalsDirty = true;
	}

	GETTER(rotation)
	//SETTER(rotation, OnTransformUpdated)

	void SetRotationUnsafe(const Mat2& newRot)
	{
		rotation = newRot;
		transformedPointsDirty = true;
		worldNormalsDirty = true;
	}

	void Setrotation(const Mat2& newRot)
	{
		SetRotationUnsafe(newRot);
		OnTransformUpdated();
	}

	void SetTransform(Vec2 newPos, const Mat2& newRot)
	{
		SetPositionUnsafe(newPos);
		SetRotationUnsafe(newRot);
		OnTransformUpdated();
	}

	std::vector<Vec2>	points;

	bool				IsStatic() { return invMass <= 0.f; }
	void				GetAllNormals(std::vector<Vec2>& outNormals) const;

	void				Build();
	void				Draw();
	size_t				GetIndex() const;

	float				GetArea() const;
	Circle				GetCircle() const;
	void				OnTransformUpdated();

	Vec2				TransformPoint(const Vec2& point) const;
	Vec2				InverseTransformPoint(const Vec2& point) const;

	// if point is outside then returned distance is negative (and doesn't make sense)
	bool				IsPointInside(const Vec2& point) const;
	// If line intersect polygon, colDist is the penetration distance, and colPoint most penetrating point of poly inside the line
	bool				IsLineIntersectingPolygon(const Line& line, Vec2& colPoint, float& colDist) const;
	bool				CheckCollision(const CPolygon& poly, Vec2& colPoint, Vec2& colNormal, float& colDist) const;

	float				GetMass() const;
	float				GetInertiaTensor() const;

	Vec2				GetPointVelocity(const Vec2& point) const;

	// Physics
	float				density;
	Vec2				speed;
	float				angularVelocity = 0.0f;
	Vec2				forces;
	float				torques = 0.0f;
	//Vec3				forces;
	//Vec3				torques;

	float				invMass = 1.0f / 60.0f;
	//Mat2				invWorldTensor;
	//Mat2				invLocalTensor;
	Mat3				invWorldTensor;
	Mat3				invLocalTensor;

	void				ApplyImpulse(const Vec2& point, const Vec2& impulse);
	void				ApplyDirectImpulse(const Vec2& point, const Vec2& impulse);
	void				ApplyForceLocal(const Vec2& localPoint, const Vec2& force);
	void				ApplyForce(const Vec2& point, const Vec2& force);
	void				UpdateSpeed(float deltaTime);

private:
	void				CreateBuffers();
	void				BindBuffers();
	void				DestroyBuffers();

	void				BuildLines();

	void				ComputeArea();
	void				RecenterOnCenterOfMass(); // Area must be computed
	void				ComputeLocalInertiaTensor(); // Must be centered on center of mass


	GLuint				m_vertexBufferId;
	size_t				m_index;

	std::vector<Line>	m_lines;

	float				m_signedArea;

	// Physics
	float				m_localInertiaTensor; // don't consider mass
};

typedef std::shared_ptr<CPolygon>	CPolygonPtr;


class SPolygonPair
{
private:
	// polyA < polyB
	CPolygonPtr	polyA;
	CPolygonPtr	polyB;

public:
	GETTER(polyA)
	GETTER(polyB)

	SPolygonPair(const CPolygonPtr& _polyA, const CPolygonPtr& _polyB) : polyA(_polyA), polyB(_polyB)
	{
		if (_polyA == _polyB)
		{
			throw std::exception("SPolygonPair : polyA and polyB can't be the same.");
		}

		if (_polyA > _polyB)
		{
			swap(polyA, polyB);
		}
	}

	bool operator==(const SPolygonPair& rhs) const
	{
		return (polyA == rhs.polyA) && (polyB == rhs.polyB); // polyA < polyB => opti
	}
};

#endif