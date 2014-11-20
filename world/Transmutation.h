#pragma once

#include "Constants.h"
#include "Singleton.h"

class Item;
struct TransmutationComponent;

class Transmutation : public Singleton<Transmutation> {
private:
	friend class Singleton<Transmutation>;
	Transmutation() {};
	~Transmutation() {};
	Transmutation(Transmutation const&); // Do not implement.
	void operator=(Transmutation const&); // Do not implement.
public:
	const bool initialise();
	Item* transmute(std::list<Item*> pItems);
private:
	const int32 _roll(Item* pItem, TransmutationComponent* pComponent);
	TransmutationComponent* _get(const uint32 pItemID) const;
	void _bootstrap();
	bool mInitialised = false;
	std::list<TransmutationComponent*> mComponents;
};