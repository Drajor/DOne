#pragma once

#include <list>
#include "../common/types.h"

struct AccountData;
struct CharacterSelect_Struct;

class DataProvider {
public:
	virtual ~DataProvider() { };
	virtual bool initialise() = 0;
	virtual void update() { };
	virtual bool getAccounts(std::list<AccountData*>& pAccounts) = 0;
	virtual bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData) = 0;
	virtual bool isCharacterNameUnique(std::string pCharacterName) = 0;

private:
};