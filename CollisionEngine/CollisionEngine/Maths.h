#ifndef _MATHS_H_
#define _MATHS_H_

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <cassert>

#define GETTER(var) inline const decltype(var)& Get##var() const { return var; }
#define SETTER(var, callback) inline void Set##var(const decltype(var)& value) { var = value; callback(); }

#define RAD2DEG(x) ((x)*(180.0f/(float)M_PI))
#define DEG2RAD(x) ((x)*((float)M_PI/180.0f))

#define USING_THREADS false

template<typename T>
T Select(bool condition, T a, T b)
{
	return ((T)condition) * a + (1 - ((T)condition)) * b;
}

template<typename T>
T Min(T a, T b)
{
	return Select(a < b, a, b);
}

template<typename T>
T Max(T a, T b)
{
	return Select(a > b, a, b);
}

template<typename T>
T Clamp(T val, T min, T max)
{
	return Min(Max(val, min), max);
}

inline bool IsNearlyEqual(float lhs, float rhs, float precision = 0.00001)
{
	return abs(lhs - rhs) < precision;
}

float Sign(float a);

float Random(float from, float to);

struct Vec2
{
	using TConstArg = const Vec2&;

	float x, y;

	Vec2() : x(0.0f), y(0.0f){}

	Vec2(float _x, float _y) : x(_x), y(_y){}

	bool operator==(const Vec2& rhs) const
	{
		return (*this - rhs).GetSqrLength() < 0.001;
	}

	Vec2 operator+(const Vec2& rhs) const
	{
		return Vec2(x + rhs.x, y + rhs.y);
	}

	Vec2& operator+=(const Vec2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	Vec2 operator-(const Vec2& rhs) const
	{
		return Vec2(x - rhs.x, y - rhs.y);
	}

	Vec2 operator-() const
	{
		return Vec2(-x, -y);
	}

	Vec2& operator-=(const Vec2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	Vec2 operator*(float factor) const
	{
		return Vec2(x * factor, y * factor);
	}

	Vec2 operator*(Vec2 rhs) const
	{
		return Vec2(x * rhs.x, y * rhs.y);
	}

	Vec2& operator*=(float factor)
	{
		*this = Vec2(x * factor, y * factor);
		return *this;
	}

	Vec2 operator/(float factor) const
	{
		return Vec2(x / factor, y / factor);
	}

	Vec2 operator/(Vec2 rhs) const
	{
		return Vec2(x / rhs.x, y / rhs.y);
	}

	Vec2& operator/=(float factor)
	{
		*this = Vec2(x / factor, y / factor);
		return *this;
	}

	float operator^(const Vec2& rhs) const
	{
		return x * rhs.y - y * rhs.x;
	}

	float GetLength() const
	{
		return sqrtf(x*x + y*y);
	}

	float GetSqrLength() const
	{
		return x*x + y*y;
	}

	static float SqrDist(Vec2 v1, Vec2 v2)
	{
		return (v2 - v1).GetSqrLength();
	}

	void	Normalize()
	{
		float length = GetLength();
		x /= length;
		y /= length;
	}

	bool IsNormalized() const
	{
		return IsNearlyEqual(GetLength(), 1.f);
	}

	Vec2	Normalized() const
	{
		Vec2 res = *this;
		res.Normalize();
		return res;
	}

	void Reflect(Vec2 normal, float elasticity = 1.0f)
	{
		*this = *this - normal * (1.0f + elasticity) * Dot(*this, normal);
	}

	Vec2 GetNormal() const
	{
		return Vec2(-y, x);
	}

	struct Vec2Int Floor() const;

	float Angle(const Vec2& to)
	{
		float cosAngle = Clamp(Dot(Normalized(), to.Normalized()), -1.0f, 1.0f);
		float angle = RAD2DEG(acosf(cosAngle)) * Sign(*this ^ to);
		return angle;
	}

	static float Dot(const Vec2& v1, const Vec2& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	static Vec2 TripleProduct(Vec2 a, Vec2 b, Vec2 c)
	{
		return b * Vec2::Dot(c, a) - a * Vec2::Dot(c, b);
	}

	static float Cross(const Vec2& lhs, const Vec2& rhs)
	{
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	static Vec2 Zero()
	{
		return { 0, 0 };
	}
};

inline Vec2 operator/(float lhs, Vec2 rhs) 
{
	return Vec2(lhs / rhs.x, lhs / rhs.y);
}

inline Vec2 GetNormalBetween2Points(Vec2 p1, Vec2 p2)
{
	return (p1 - p2).GetNormal().Normalized();
}

inline bool IsNearlyEqual(Vec2 lhs, Vec2 rhs, float precision = 0.00001)
{
	return IsNearlyEqual(lhs.x, rhs.x) && IsNearlyEqual(lhs.y, rhs.y);
}

struct Vec3
{
	float x, y, z;

	Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	Vec3(const Vec2& rhs) : x(rhs.x), y(rhs.y), z(0.f) {}

	static float Dot(const Vec3& lhs, const Vec3& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	static Vec3 Cross(const Vec3& lhs, const Vec3& rhs)
	{
		Vec3 result;
		result.x = lhs.y * rhs.z - lhs.z * rhs.y;
		result.y = lhs.z * rhs.x - lhs.x * rhs.z;
		result.z = lhs.x * rhs.y - lhs.y * rhs.x;
		return result;
	}

	Vec3 operator+(const Vec3& rhs) const
	{
		return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vec3& operator*=(float rhs)
	{
		(*this) = (*this) * rhs;
		return *this;
	}

	Vec3 operator-() const
	{
		return Vec3(-x, -y, -z);
	}

	Vec3 operator-(const Vec3& rhs) const
	{
		return -rhs + *this;
	}

	Vec3 operator*(const float rhs) const
	{
		return Vec3(x * rhs, y * rhs, z * rhs);
	}

	Vec3 operator/(const float rhs) const
	{
		return (*this) * (1.f / rhs);
	}

	float SqrLength() const
	{
		return x * x + y * y + z * z;
	}

	float Length() const
	{
		return sqrt(SqrLength());
	}

	Vec3 GetUnit() const
	{
		return (*this) / Length();
	}
};

inline bool IsNearlyEqual(Vec3 lhs, Vec3 rhs, float precision = 0.00001)
{
	return IsNearlyEqual(lhs.x, rhs.x) && IsNearlyEqual(lhs.y, rhs.y) && IsNearlyEqual(lhs.z, rhs.z);
}

struct Vec2Int
{
	int x, y;
	bool operator==(const Vec2Int& rhs) const
	{
		return rhs.x == x && rhs.y == y;
	}

	int GetArea() const
	{
		return x * y;
	}

	Vec2Int operator-(const Vec2Int& rhs) const
	{
		return (*this) + (-rhs);
	}

	Vec2Int operator+(const Vec2Int& rhs) const
	{
		return Vec2Int{ x + rhs.x, y + rhs.y };
	}

	Vec2Int operator-() const
	{
		return Vec2Int{ - x, - y };
	}

	int Sum() const
	{
		return x + y;
	}

	static Vec2Int One() 
	{
		return Vec2Int{1, 1};
	}

	static Vec2Int Zero()
	{
		return Vec2Int{ 0, 0 };
	}

	operator Vec2() const
	{
		return Vec2{ float(x), float(y) };
	}
};

inline Vec2Int Vec2::Floor() const
{
	return Vec2Int{int(floor(x)), int(floor(y))};
}


struct Mat2
{
	Vec2 X, Y;

	Mat2() : X(1.0f, 0.0f), Y(0.0f, 1.0f){}

	Mat2(float a, float b, float c, float d) : X(a, c), Y(b, d){}

	Mat2	GetInverse() const
	{
		return Mat2(X.x, X.y, Y.x, Y.y);
	}

	float	GetAngle() const
	{
		return Vec2(1.0f, 0.0f).Angle(X);
	}

	void	SetAngle(float angle)
	{
		float c = cosf(angle * ((float)M_PI / 180.0f));
		float s = sinf(angle * ((float)M_PI / 180.0f));

		X.x = c; X.y = s;
		Y.x = -s; Y.y = c;
	}

	void Rotate(float angle)
	{
		Mat2 matRot;
		matRot.SetAngle(angle);
		*this = *this * matRot;
	}

	Mat2 operator*(const Mat2& rhs) const
	{
		return Mat2(X.x*rhs.X.x + Y.x*rhs.X.y, X.x*rhs.Y.x + Y.x*rhs.Y.y, X.y*rhs.X.x + Y.y*rhs.X.y, X.y*rhs.Y.x + Y.y*rhs.Y.y);
	}

	Vec2 operator*(const Vec2& vec) const
	{
		return Vec2(X.x*vec.x + Y.x*vec.y, X.y*vec.x + Y.y*vec.y);
	}
};

struct Mat3
{
	Vec3 X, Y, Z;

	Mat3() : X(1.0f, 0.0f, 0.0f), Y(0.0f, 1.0f, 0.0f), Z(0.0f, 0.0f, 0.1f) {}

	Mat3(const Mat2& rhs) : X(rhs.X.x, rhs.X.y, 0.0f), Y(rhs.Y.x, rhs.Y.y, 0.0f), Z(0.0f, 0.0f, 1.0f) {}

	Vec3 operator*(const Vec3& vec) const
	{
		return Vec3(X.x * vec.x + Y.x * vec.y + Z.x * vec.z, X.y * vec.x + Y.y * vec.y + Z.y * vec.z, X.z * vec.x + Y.z * vec.y + Z.z * vec.z);
	}

	Mat3 operator*(const Mat3& rhs) const
	{
		Mat3 m;

		m.X.x = X.x * rhs.X.x + X.y * rhs.Y.x + X.z * rhs.Z.x;
		m.X.y = X.x * rhs.X.y + X.y * rhs.Y.y + X.z * rhs.Z.y;
		m.X.z = X.x * rhs.X.z + X.y * rhs.Y.z + X.z * rhs.Z.z;

		m.Y.x = Y.x * rhs.X.x + Y.y * rhs.Y.x + Y.z * rhs.Z.x;
		m.Y.y = Y.x * rhs.X.y + Y.y * rhs.Y.y + Y.z * rhs.Z.y;
		m.Y.z = Y.x * rhs.X.z + Y.y * rhs.Y.z + Y.z * rhs.Z.z;

		m.Z.x = Z.x * rhs.X.x + Z.y * rhs.Y.x + Z.z * rhs.Z.x;
		m.Z.y = Z.x * rhs.X.y + Z.y * rhs.Y.y + Z.z * rhs.Z.y;
		m.Z.z = Z.x * rhs.X.z + Z.y * rhs.Y.z + Z.z * rhs.Z.z;

		return m;
	}

	Mat3 operator*(float rhs) const
	{
		Mat3 m = (*this);
		m.X *= rhs;
		m.Y *= rhs;
		m.Z *= rhs;
		return m;
	}

	static Mat3 Zero()
	{
		Mat3 m;
		m.X = Vec3(0,0,0);
		m.Y = Vec3(0, 0, 0);
		m.Z = Vec3(0, 0, 0);
		return m;
	}

	static Mat3 Identity()
	{
		Mat3 m;
		m.X = Vec3(1, 0, 0);
		m.Y = Vec3(0, 1, 0);
		m.Z = Vec3(0, 0, 1);
		return m;
	}
};

struct Line
{
	Vec2 point, dir;

	Line() = default;
	Line(Vec2 _point, Vec2 _dir) : point(_point), dir(_dir){}

	Vec2	GetNormal() const
	{
		return dir.GetNormal();
	}

	// positive value means point above line, negative means point is under line
	float	GetPointDist(const Vec2& pt) const
	{
		return Vec2::Dot(pt - point, GetNormal());
	}

	Line	Transform(const Mat2& rotation, const Vec2& position) const
	{
		return Line(position + rotation * point, rotation * dir);
	}

	Vec2	Project(const Vec2& pt) const
	{
		return point + dir * Vec2::Dot((pt - point), dir);
	}
};

struct Circle
{
	Vec2 point;
	float radius;

	Circle() = default;
	Circle(Vec2 _point, float _radius) : point(_point), radius(_radius) {}
	Circle(Vec2 _point, const std::vector<Vec2>& _polygonPoints);

	bool IsColliding(const Circle& circle) const
	{
		float radiusSum = radius + circle.radius;
		return (point - circle.point).GetSqrLength() < radiusSum * radiusSum;
	}
};

struct AABB
{
	Vec2 pMin, pMax;

	AABB() = default;
	AABB(Vec2 _min, Vec2 _max) : pMin(_min), pMax(_max) {}

	static AABB Make(Vec2 p1, Vec2 p2)
	{
		AABB aabb;

		if (p1.x < p2.x)
		{
			aabb.pMin.x = p1.x;
			aabb.pMax.x = p2.x;
		}
		else
		{
			aabb.pMin.x = p2.x;
			aabb.pMax.x = p1.x;
		}

		if (p1.y < p2.y)
		{
			aabb.pMin.y = p1.y;
			aabb.pMax.y = p2.y;
		}
		else 
		{
			aabb.pMin.y = p2.y;
			aabb.pMax.y = p1.y;
		}

		return aabb;
	}

	void EnlargeWithPoint(const Vec2& p)
	{
		pMin.x = Min(pMin.x, p.x);
		pMin.y = Min(pMin.y, p.y);
		pMax.x = Max(pMax.x, p.x);
		pMax.y = Max(pMax.y, p.y);
	}

	bool CheckCollision(const AABB& aabb) const
	{
		return pMax.x > aabb.pMin.x && pMin.x < aabb.pMax.x 
			&& pMax.y > aabb.pMin.y && pMin.y < aabb.pMax.y;
	}

	void Translate(const Vec2& translation)
	{
		pMin += translation;
		pMax += translation;
	}

	AABB Merge(const AABB& aabb) const
	{
		return AABB(Vec2(Min(aabb.pMin.x, pMin.x), Min(aabb.pMin.y, pMin.y)), Vec2(Max(aabb.pMax.x, pMax.x), Max(aabb.pMax.y, pMax.y)));
	}

	float GetArea() const
	{
		return (pMax.x - pMin.x) * (pMax.y - pMin.y);
	}
};
using AABB2 = AABB;

struct MoveableAABB
{
private:
	AABB baseAABB;
	AABB movedAABB;

	void OnBaseChanged()
	{
		movedAABB = baseAABB;
	}

public:
	SETTER(baseAABB, OnBaseChanged)
	GETTER(movedAABB)
	GETTER(baseAABB)

	void SetTransform(const Vec2& newPos)
	{
		movedAABB = baseAABB;
		movedAABB.Translate(newPos);
	}

	void SetTransform(const Vec2& newPos, const Mat2& rotation)
	{
		movedAABB = AABB::Make(rotation * baseAABB.pMin, rotation * baseAABB.pMax);
		movedAABB.EnlargeWithPoint(Vec2(rotation * Vec2(baseAABB.pMin.x, baseAABB.pMax.y)));
		movedAABB.EnlargeWithPoint(Vec2(rotation * Vec2(baseAABB.pMax.x, baseAABB.pMin.y)));
		movedAABB.Translate(newPos);
	}

	void SetMovedAABB(const AABB& newMovedAABB)
	{
		movedAABB = newMovedAABB;
	}
};

inline void GetFarthestPoinstInDirection(const std::vector<Vec2>& points, Vec2::TConstArg direction, std::vector<Vec2>& outPoints)
{
	assert(points.size() != 0);
	float projMax = std::numeric_limits<float>::lowest();
	Vec2 pointWithProjMax;
	for (auto it = points.begin(); it != points.end(); it++)
	{
		float proj = Vec2::Dot(*it, direction);
		if (proj > projMax)
		{
			outPoints.clear();
			outPoints.push_back(*it);
			projMax = proj;
		}
		else if (IsNearlyEqual(proj, projMax, 0.001))
		{
			outPoints.push_back(*it);
		}
	}
}

template<typename CONTAINER>
Vec2 GetFarthestPointInDirection(const CONTAINER& container, Vec2::TConstArg direction)
{
	assert(container.size() != 0);
	float projMax = std::numeric_limits<float>::lowest();
	Vec2 pointWithProjMax;
	for (auto it = container.begin(); it != container.end(); it++)
	{
		float proj = Vec2::Dot(*it, direction);
		if (proj > projMax)
		{
			projMax = proj;
			pointWithProjMax = *it;
		}
	}
	return pointWithProjMax;
}

// TODO : same but returning iterator, and taking start iterator as input to use spatial coherence
template<typename CONTAINER>
Vec2 GetFarthestPointInDirectionWithConvexShape(const CONTAINER& container, Vec2::TConstArg direction)
{
	assert(container.size() != 0);
	Vec2 pointWithProjMax = *container.begin();
	float projMax = Vec2::Dot(pointWithProjMax, direction);
	for (auto it = container.begin() + 1; it != container.end(); it++)
	{
		float proj = Vec2::Dot(*it, direction);
		if (proj < projMax)
			continue;

		do 
		{
			projMax = proj;
			pointWithProjMax = *it;
			it++;
		} while (it != container.end() && (proj = Vec2::Dot(*it, direction)) > projMax);
		return pointWithProjMax;
	}
	return pointWithProjMax;
}

// TODO : same but returning iterator, and taking start iterator as input to use spatial coherence
template<typename CONTAINER>
auto GetFarthestPointItInDirectionWithConvexShape(const CONTAINER& container, Vec2::TConstArg direction)
{
	assert(container.size() != 0);
	CONTAINER::const_iterator itWithProjMax = container.begin();
	float projMax = Vec2::Dot(*itWithProjMax, direction);
	for (auto it = container.begin() + 1; it != container.end(); it++)
	{
		float proj = Vec2::Dot(*it, direction);
		if (proj < projMax)
			continue;

		do
		{
			projMax = proj;
			itWithProjMax = it;
			it++;
		} while (it != container.end() && (proj = Vec2::Dot(*it, direction)) > projMax);
		return itWithProjMax;
	}
	return itWithProjMax;
}

struct Triangle
{
	Vec2 a, b, c;

	bool ContainsPoint(Vec2 p) const 
	{
		bool b1 = Vec2::Dot(p - a, (b - a).GetNormal()) < 0;
		bool b2 = Vec2::Dot(p - b, (c - b).GetNormal()) < 0;
		bool b3 = Vec2::Dot(p - c, (a - c).GetNormal()) < 0;

		return b1 == b2 && b2 == b3;
	}
};

#endif