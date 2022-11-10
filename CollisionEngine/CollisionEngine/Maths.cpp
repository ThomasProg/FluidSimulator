#include "Maths.h"

#include <stdlib.h>

float Sign(float a)
{
	return Select(a >= 0.0f, 1.0f, -1.0f);
}

float Random(float from, float to)
{
	return from + (to - from) * (((float)rand()) / ((float)RAND_MAX));
}

Circle::Circle(Vec2 _point, const std::vector<Vec2>& _polygonPoints) : point(_point)
{
	float maxSqr = 0.f;
	for (Vec2 p : _polygonPoints)
	{
		float sqrLength = p.GetSqrLength();
		if (sqrLength > maxSqr)
		{
			maxSqr = sqrLength;
		}
	}

	this->radius = sqrt(maxSqr);
}