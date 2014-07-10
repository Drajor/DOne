#include "MySQLDataProvider.h"
#include "Data.h"
#include "DatabaseConnection.h"
#include "../common/StringUtil.h"

#define USE_FIELD_MACROS int j = 0
#define NEXT_FIELD row[j++]
#define NEXT_FIELD_INT atoi(NEXT_FIELD) 
#define NEXT_FIELD_BOOL atoi(NEXT_FIELD) > 0 ? true : false

MySQLDataProvider::MySQLDataProvider() : mDatabaseConnection(0) { }

MySQLDataProvider::~MySQLDataProvider()
{
	safe_delete(mDatabaseConnection);
}

bool MySQLDataProvider::initialise()
{
	mDatabaseConnection = new DatabaseConnection();
	return mDatabaseConnection->initialise();
}

void MySQLDataProvider::update()
{
	mDatabaseConnection->ping();
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
