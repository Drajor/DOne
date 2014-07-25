#pragma once

#include "DataProvider.h"
#include <list>
#include <sstream>

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
	bool createAccount(uint32 pLoginServerAccountID, std::string pLoginServerAccountName);
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
	bool isCharacterNameUnique(std::string pCharacterName);
	bool deleteCharacter(std::string pCharacterName);
	bool createCharacter(uint32 pWorldAccountID, std::string pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	bool checkOwnership(uint32 pWorldAccountID, std::string pCharacterName);

	bool loadCharacter(std::string pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	bool saveCharacter(uint32 pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
private:
	uint32 _getCharacterID(std::string pCharacterName);
	void copyProfile(PlayerProfile_Struct* pProfileTo, PlayerProfile_Struct* pProfileFrom);
	
	Timer* mKeepAliveTimer;
	DatabaseConnection* mDatabaseConnection;
	std::stringstream mStringStream;
};