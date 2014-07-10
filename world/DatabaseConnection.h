#pragma once

// Ripped from dbcore.h

#ifdef _WINDOWS
#define _WINSOCKAPI_
// mysql depends on winsock therefore needs to be included first.
#include <winsock2.h>
#endif

#include <mysql.h>
#include "../common/types.h"

class DatabaseConnection {
public:
	

	DatabaseConnection();
	~DatabaseConnection();
	bool initialise();
	bool runQuery(const char* query, uint32 querylen, char* errbuf = 0, MYSQL_RES** result = 0, uint32* affected_rows = 0, uint32* last_insert_id = 0, uint32* errnum = 0, bool retry = true);
	void ping();

protected:
	bool open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, uint32 iPort, uint32* errnum = 0, char* errbuf = 0, bool iCompress = false, bool iSSL = false);
private:
	enum ConnectionStatus { Closed, Connected, Error };
	ConnectionStatus mStatus;
	MYSQL* mMYSQL;

	bool open(uint32* errnum = 0, char* errbuf = 0);

	


	char*	pHost;
	char*	pUser;
	char*	pPassword;
	char*	pDatabase;
	bool	pCompress;
	uint32	pPort;
	bool	pSSL;

};