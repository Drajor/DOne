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
	//virtual bool getAccounts(std::list<AccountData*>& pAccounts) = 0;
	//virtual bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData) = 0;
	//virtual bool isCharacterNameUnique(String pCharacterName) = 0;
	//virtual bool deleteCharacter(String pCharacterName) = 0;
	//virtual bool createCharacter(uint32 pWorldAccountID, String pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) = 0;
	//virtual bool checkOwnership(uint32 pWorldAccountID, String pCharacterName) = 0;
	virtual bool loadCharacter(String pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) = 0;
	virtual bool createAccount(uint32 pLoginServerAccountID, String pLoginServerAccountName) = 0;
	virtual bool saveCharacter(uint32 pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) = 0;
};