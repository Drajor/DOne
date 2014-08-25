#pragma once

#include <math.h>

struct Vector3 {
	Vector3(float pX = 0.0f, float pY = 0.0f, float pZ = 0.0f) : x(pX), y(pY), z(pZ) {}
	float x;
	float y;
	float z;

	inline const float distance(const Vector3& pVector) {
		return sqrt(squareDistance(pVector));
	}
	inline const float squareDistance(const Vector3& pVector) {
		return ((x - pVector.x) * (x - pVector.x) + (y - pVector.y) * (y - pVector.y) + (z - pVector.z) * (z - pVector.z));
	}
};