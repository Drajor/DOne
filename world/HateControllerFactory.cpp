#include "HateControllerFactory.h"

void HateControllerFactory::set(const String pID, ProductMethod pMethod) {
	mMethods[pID] = pMethod;
}

HateControllerFactory::Product HateControllerFactory::make(const String pID) {
	// TODO: Error Checking.
	return mMethods[pID]();
}

