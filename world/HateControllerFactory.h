#pragma once

#include "Types.h"
#include <map>
#include <functional>

class HateController;
class Actor;

class HateControllerFactory {
public:
	typedef HateController* Product;
	typedef std::function<Product()> ProductMethod;

	void set(const String pID, ProductMethod pMethod);
	Product make(const String pID);
private:
	std::map<String, ProductMethod> mMethods;
};