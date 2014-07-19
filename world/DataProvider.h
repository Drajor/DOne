#pragma once

#include <list>
#include "../common/types.h"

struct AccountData;
struct CharacterSelect_Struct;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

class DataProvider {
public:
	virtual ~DataProvider() { };
	virtual bool initialise() = 0;
	virtual void update() { };
	virtual bool getAccounts(std::list<AccountData*>& pAccounts) = 0;
	virtual bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData) = 0;
	virtual bool isCharacterNameUnique(std::string pCharacterName) = 0;
	virtual bool deleteCharacter(std::string pCharacterName) = 0;
	virtual bool createCharacter(uint32 pWorldAccountID, std::string pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) = 0;
	virtual bool checkOwnership(uint32 pWorldAccountID, std::string pCharacterName) = 0;
	virtual bool loadCharacter(std::string pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) = 0;
};