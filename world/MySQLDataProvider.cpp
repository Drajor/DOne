#include "MySQLDataProvider.h"
#include "Data.h"
#include "DatabaseConnection.h"
#include "../common/timer.h"
#include "../common/eq_packet_structs.h"
#include "../common/StringUtil.h"

#define KEEP_ALIVE_TIMER 10000

#define USE_FIELD_MACROS int j = 0
#define NEXT_FIELD row[j++]
#define NEXT_FIELD_INT atoi(NEXT_FIELD) 
#define NEXT_FIELD_BOOL atoi(NEXT_FIELD) > 0 ? true : false

MySQLDataProvider::MySQLDataProvider() : mDatabaseConnection(0), mKeepAliveTimer(0) { }

MySQLDataProvider::~MySQLDataProvider() {
	safe_delete(mKeepAliveTimer);
	safe_delete(mDatabaseConnection);
}

bool MySQLDataProvider::initialise() {
	mKeepAliveTimer = new Timer(KEEP_ALIVE_TIMER);
	mKeepAliveTimer->Trigger();

	mDatabaseConnection = new DatabaseConnection();
	return mDatabaseConnection->initialise();
}

void MySQLDataProvider::update() {
	// Is it time to ping mysql?
	if (mKeepAliveTimer->Check()) {
		mKeepAliveTimer->Start();
		mDatabaseConnection->ping();
	}
}

bool MySQLDataProvider::getAccounts(std::list<AccountData*>& pAccounts) {
	static const std::string GET_ACCOUNTS_QUERY = "SELECT id, name, charname, sharedplat, status, lsaccount_id, gmspeed, hideme, suspendeduntil FROM account";
	static const uint32 GET_ACCOUNTS_QUERY_LENGTH = GET_ACCOUNTS_QUERY.length();
	char errorBuffer[MYSQL_ERRMSG_SIZE];
	MYSQL_RES* result;
	if (mDatabaseConnection->runQuery(GET_ACCOUNTS_QUERY.c_str(), GET_ACCOUNTS_QUERY_LENGTH, errorBuffer, &result)) {
		for (int i = 0; i < mysql_num_rows(result); i++) {
			USE_FIELD_MACROS;
			MYSQL_ROW row = mysql_fetch_row(result);
			AccountData* accountData = new AccountData();
			accountData->mWorldAccountID = NEXT_FIELD_INT;
			accountData->mName = NEXT_FIELD;
			accountData->mCharacterName = NEXT_FIELD;
			accountData->mSharedPlatinum = NEXT_FIELD_INT;
			accountData->mStatus = NEXT_FIELD_INT;
			accountData->mLoginServerAccountID = NEXT_FIELD_INT;
			accountData->mGMSpeed = NEXT_FIELD_BOOL;
			accountData->mHidden = NEXT_FIELD_BOOL;
			pAccounts.push_back(accountData);
		}
	} else {
		return false;
	}

	mysql_free_result(result);
	return true;
}

bool MySQLDataProvider::getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData) {
	static const int NUM_CHARACTERS = 10;
	static const std::string GET_CHARACTERS_QUERY = "SELECT name, profile, zonename, class, level FROM character_ WHERE account_id = %i order by name limit 10";
	char errorBuffer[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 queryLength = MakeAnyLenString(&query, GET_CHARACTERS_QUERY.c_str(), pWorldAccountID);
	MYSQL_RES* result;
	if (mDatabaseConnection->runQuery(query, queryLength, errorBuffer, &result)) {
		safe_delete_array(query);

		// TODO: Why are only part of this data initialised here?
		for (int i = 0; i < NUM_CHARACTERS; i++) {
			strcpy(pCharacterSelectData->name[i], "<none>");
			pCharacterSelectData->zone[i] = 0;
			pCharacterSelectData->level[i] = 0;
			pCharacterSelectData->tutorial[i] = 0;
			pCharacterSelectData->gohome[i] = 0;
		}

		for (int i = 0; i < mysql_num_rows(result); i++) {
			MYSQL_ROW row = mysql_fetch_row(result);
			strcpy(pCharacterSelectData->name[i], row[0]);

			// Copy data from Player Profile.
			PlayerProfile_Struct* playerProfile = (PlayerProfile_Struct*)row[1];
			pCharacterSelectData->class_[i] = playerProfile->class_;
			pCharacterSelectData->level[i] = playerProfile->level;
			pCharacterSelectData->race[i] = playerProfile->race;
			pCharacterSelectData->gender[i] = playerProfile->gender;
			pCharacterSelectData->deity[i] = playerProfile->deity;
			//pCharacterSelectData->zone[i] = GetZoneID(row[2]);
			pCharacterSelectData->zone[i] = 1;
			pCharacterSelectData->face[i] = playerProfile->face;
			pCharacterSelectData->haircolor[i] = playerProfile->haircolor;
			pCharacterSelectData->beardcolor[i] = playerProfile->beardcolor;
			pCharacterSelectData->eyecolor2[i] = playerProfile->eyecolor2;
			pCharacterSelectData->eyecolor1[i] = playerProfile->eyecolor1;
			pCharacterSelectData->hairstyle[i] = playerProfile->hairstyle;
			pCharacterSelectData->beard[i] = playerProfile->beard;
			pCharacterSelectData->drakkin_heritage[i] = playerProfile->drakkin_heritage;
			pCharacterSelectData->drakkin_tattoo[i] = playerProfile->drakkin_tattoo;
			pCharacterSelectData->drakkin_details[i] = playerProfile->drakkin_details;
			pCharacterSelectData->tutorial[i] = 0; // Disabled.
			pCharacterSelectData->gohome[i] = 0; // Disabled.

			// No equipment yet. NUDES!
			for (int j = 0; j < 9; j++) {
				pCharacterSelectData->equip[i][j] = 2;
				pCharacterSelectData->cs_colors[i][j].color = 0;
			}

			pCharacterSelectData->primary[i] = 146;
			pCharacterSelectData->secondary[i] = 146;

		}
	} else {
		return false;
	}
	mysql_free_result(result);
	return true;
}

bool MySQLDataProvider::isCharacterNameUnique(std::string pCharacterName)
{
	static const std::string CHECK_NAME_QUERY = "SELECT name FROM character_ WHERE BINARY name = '%s'"; // BINARY makes it case sensitive.
	char errorBuffer[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	uint32 queryLength = MakeAnyLenString(&query, CHECK_NAME_QUERY.c_str(), pCharacterName.c_str());
	MYSQL_RES* result;
	if (mDatabaseConnection->runQuery(query, queryLength, errorBuffer, &result)) {
		// Expect zero rows if name is not in use.
		if (mysql_num_rows(result) != 0) {
			mysql_free_result(result);
			return false;
		}
	}
	else {
		mysql_free_result(result);
		return false;
	}
	mysql_free_result(result);
	return true;
}
