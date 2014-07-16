#pragma once

#include "DataProvider.h"
#include <list>

struct AccountData;
struct CharacterSelect_Struct;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;
class DatabaseConnection;
class Timer;

class MySQLDataProvider : public DataProvider {
public:
	MySQLDataProvider();
	~MySQLDataProvider();
	bool initialise();
	void update();

	bool getAccounts(std::list<AccountData*>& pAccounts);
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
	bool isCharacterNameUnique(std::string pCharacterName);
	bool deleteCharacter(std::string pCharacterName);
	bool createCharacter(uint32 pWorldAccountID, std::string pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
private:
	uint32 _getCharacterID(std::string pCharacterName);
	
	Timer* mKeepAliveTimer;
	DatabaseConnection* mDatabaseConnection;
};