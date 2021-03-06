#pragma once

#include "Types.h"
#include <math.h>

struct Vector3 {
	Vector3(float pX = 0.0f, float pY = 0.0f, float pZ = 0.0f) : x(pX), y(pY), z(pZ) {}
	Vector3(const Vector3& pVector){
		x = pVector.x;
		y = pVector.y;
		z = pVector.z;
	}
	
	inline Vector3& operator = (const Vector3& pVector) {
		x = pVector.x;
		y = pVector.y;
		z = pVector.z;
		return *this;
	}

	inline void swap(Vector3& pVector) {
		std::swap(x, pVector.x);
		std::swap(y, pVector.y);
		std::swap(z, pVector.z);
	}

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

	inline const float distance(const Vector3& pVector) const {
		return sqrt(squareDistance(pVector));
	}
	inline const float squareDistance(const Vector3& pVector) const {
		return ((x - pVector.x) * (x - pVector.x) + (y - pVector.y) * (y - pVector.y) + (z - pVector.z) * (z - pVector.z));
	}
	String toString(uint32 pPrecision = 1) const {
		StringStream ss;
		ss.precision(pPrecision);
		ss << std::fixed << "(" << x << ", " << y << ", " << z << ")";
		return ss.str();
	}
};