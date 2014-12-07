#pragma once

#include "Types.h"
#include "Vector3.h"

class Object {
public:
	inline const u32 getType() const { return mType; }
	inline void setType(const u32 pType) { mType = pType; }

	inline const String& getAsset() const { return mAsset; }
	inline void setAsset(const String& pAsset) { mAsset = pAsset; }

	inline const float getSize() const { return mSize; }
	inline void setSize(const float pSize) { mSize = pSize; }

	inline const float getHeading() const { return mHeading; }
	inline void setHeading(const float pHeading) { mHeading = pHeading; }

	inline const Vector3& getPosition() { return mPosition; }
	inline void setPosition(const Vector3& pPosition) { mPosition = pPosition; }
private:
	u32 mType = 0;
	String mAsset;
	float mSize = 1.0f;
	float mHeading = 0.0f;
	Vector3 mPosition;
};