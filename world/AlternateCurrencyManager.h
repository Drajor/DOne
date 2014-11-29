#pragma once

#include "Constants.h"
#include "Singleton.h"

struct AlternateCurrency;

class AlternateCurrencyManager : public Singleton<AlternateCurrencyManager>{
private:
	friend class Singleton<AlternateCurrencyManager>;
	AlternateCurrencyManager() {};
	~AlternateCurrencyManager() {};
	AlternateCurrencyManager(AlternateCurrencyManager const&); // Do not implement.
	void operator=(AlternateCurrencyManager const&); // Do not implement.
public:
	const bool initialise();
	inline std::list<AlternateCurrency*>& getCurrencies() { return mCurrencies; }
	const uint32 getItemID(const uint32 pCurrencyID) const;
	const uint32 getCurrencyID(const uint32 pItemID) const;
private:
	bool mInitialised = false;
	std::list<AlternateCurrency*> mCurrencies;
};