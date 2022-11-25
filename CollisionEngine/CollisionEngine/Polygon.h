#ifndef _POLYGON_H_
#define _POLYGON_H_

#include <GL/glew.h>
#include <vector>
#include <memory>
#include <functional>


#include "Maths.h"

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

public:
	std::function<void(const CPolygon&)> onTransformUpdatedCallback;

	GETTER(position)
	SETTER(position, OnTransformUpdated)

	GETTER(rotation)
	SETTER(rotation, OnTransformUpdated)

	std::vector<Vec2>	points;

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

	bool				CheckCollision(const CPolygon& poly, Vec2& colPoint, Vec2& colNormal, float& colDist) const;

	// Physics
	float				density;
	Vec2				speed;

private:
	void				CreateBuffers();
	void				BindBuffers();
	void				DestroyBuffers();

	void				BuildLines();

	GLuint				m_vertexBufferId;
	size_t				m_index;

	std::vector<Line>	m_lines;
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