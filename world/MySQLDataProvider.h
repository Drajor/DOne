#pragma once

#include "Constants.h"
#include "DataProvider.h"

//struct AccountData;
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

	//bool getAccounts(std::list<AccountData*>& pAccounts);
	//bool createAccount(uint32 pLoginServerAccountID, String pLoginServerAccountName);
	//bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
	//bool isCharacterNameUnique(String pCharacterName);
	//bool deleteCharacter(String pCharacterName);
	//bool createCharacter(uint32 pWorldAccountID, String pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	//bool checkOwnership(uint32 pWorldAccountID, String pCharacterName);

	bool loadCharacter(String pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	bool saveCharacter(uint32 pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
private:
	uint32 _getCharacterID(String pCharacterName);
	void copyProfile(PlayerProfile_Struct* pProfileTo, PlayerProfile_Struct* pProfileFrom);
	
	Timer* mKeepAliveTimer;
	DatabaseConnection* mDatabaseConnection;
	StringStream mStringStream;
};