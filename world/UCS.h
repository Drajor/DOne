#pragma once

#include "Constants.h"
#include "Singleton.h"

class EQStreamFactory;

class UCS : public Singleton<UCS> {
private:
	friend class Singleton<UCS>;
	UCS() {};
	~UCS() {};
	UCS(UCS const&); // Do not implement.
	void operator=(UCS const&); // Do not implement.
public:
	bool initialise();
	void update();
private:

	EQStreamFactory* mStreamFactory = nullptr;
};