#pragma once

#include "Types.h"
#include "Vector3.h"

struct BindLocation {
public:
	inline const u16 getZoneID() const { return mZoneID; }
	inline const Vector3& getPosition() const { return mPosition; }
	inline const float getHeading() const { return mHeading; }

	inline void setZoneID(u16 pValue) { mZoneID = pValue; }
	inline void setPosition(const Vector3& pValue) { mPosition = pValue; }
	inline void setHeading(const float pValue) { mHeading = pValue; }
private:
	u16 mZoneID = 0;
	Vector3 mPosition;
	float mHeading = 0.0f;
};