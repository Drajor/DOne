#include "../common/debug.h"

#ifdef _WINDOWS
#define _WINSOCKAPI_
#include <winsock2.h>
#endif

#include <iostream>
#include <mysqld_error.h>
#include <limits.h>
#include "DatabaseConnection.h"
#include <string.h>
#include "../common/MiscFunctions.h"
#include <cstdlib>

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#include <process.h>
#else
#include "unix.h"
#include <pthread.h>
#endif

#ifdef _EQDEBUG
#define DEBUG_MYSQL_QUERIES 0
#else
#define DEBUG_MYSQL_QUERIES 0
#endif

DatabaseConnection::DatabaseConnection() {
	mMYSQL = new MYSQL();
	mysql_init(mMYSQL);
	pHost = 0;
	pUser = 0;
	pPassword = 0;
	pDatabase = 0;
	pCompress = false;
	pSSL = false;
	mStatus = Closed;
}

DatabaseConnection::~DatabaseConnection() {
	mysql_close(mMYSQL);
	safe_delete(mMYSQL);
	safe_delete_array(pHost);
	safe_delete_array(pUser);
	safe_delete_array(pPassword);
	safe_delete_array(pDatabase);
}

bool DatabaseConnection::initialise()
{
	return open("127.0.0.1", "root", "password", "drajordev", 3306, 0, 0, 0, 0); // TODO: Remove hardcode ;)
}

// Sends the MySQL server a keepalive
void DatabaseConnection::ping() {
	mysql_ping(mMYSQL);
}

// Taken from errmsg.h 
#define CR_SERVER_GONE_ERROR 2006
#define CR_SERVER_LOST 2013
bool DatabaseConnection::runQuery(const char* query, uint32 querylen, char* errbuf, MYSQL_RES** result, uint32* affected_rows, uint32* last_insert_id, uint32* errnum, bool retry) {
	if (errnum)
		*errnum = 0;
	if (errbuf)
		errbuf[0] = 0;
	bool ret = false;
	if (mStatus != Connected)
		open();

	if (mysql_real_query(mMYSQL, query, querylen)) {
		if (mysql_errno(mMYSQL) == CR_SERVER_GONE_ERROR)
			mStatus = Error;
		if (mysql_errno(mMYSQL) == CR_SERVER_LOST || mysql_errno(mMYSQL) == CR_SERVER_GONE_ERROR) {
			if (retry) {
				std::cout << "Database Error: Lost connection, attempting to recover...." << std::endl;
				ret = runQuery(query, querylen, errbuf, result, affected_rows, last_insert_id, errnum, false);
				if (ret)
					std::cout << "Reconnection to database successful." << std::endl;
			}
			else {
				mStatus = Error;
				if (errnum)
					*errnum = mysql_errno(mMYSQL);
				if (errbuf)
					snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mMYSQL), mysql_error(mMYSQL));
				std::cout << "DB Query Error #" << mysql_errno(mMYSQL) << ": " << mysql_error(mMYSQL) << std::endl;
				ret = false;
			}
		}
		else {
			if (errnum)
				*errnum = mysql_errno(mMYSQL);
			if (errbuf)
				snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mMYSQL), mysql_error(mMYSQL));
#ifdef _EQDEBUG
			std::cout << "DB Query Error #" << mysql_errno(mMYSQL) << ": " << mysql_error(mMYSQL) << std::endl;
#endif
			ret = false;
		}
	}
	else {
		if (result && mysql_field_count(mMYSQL)) {
			*result = mysql_store_result(mMYSQL);
#ifdef _EQDEBUG
			DBMemLeak::Alloc(*result, query);
#endif
		}
		else if (result)
			*result = 0;
		if (affected_rows)
			*affected_rows = mysql_affected_rows(mMYSQL);
		if (last_insert_id)
			*last_insert_id = mysql_insert_id(mMYSQL);
		if (result) {
			if (*result) {
				ret = true;
			}
			else {
#ifdef _EQDEBUG
				std::cout << "DB Query Error: No Result" << std::endl;
#endif
				if (errnum)
					*errnum = UINT_MAX;
				if (errbuf)
					strcpy(errbuf, "DBcore::RunQuery: No Result");
				ret = false;
			}
		}
		else {
			ret = true;
		}
	}
#if DEBUG_MYSQL_QUERIES >= 1
	if (ret) {
		std::cout << "query successful";
		if (result && (*result))
			std::cout << ", " << (int)mysql_num_rows(*result) << " rows returned";
		if (affected_rows)
			std::cout << ", " << (*affected_rows) << " rows affected";
		std::cout << std::endl;
	}
	else {
		std::cout << "QUERY: query FAILED" << std::endl;
	}
#endif
	return ret;
}

bool DatabaseConnection::open(const char* iHost, const char* iUser, const char* iPassword, const char* iDatabase, uint32 iPort, uint32* errnum, char* errbuf, bool iCompress, bool iSSL) {
	safe_delete(pHost);
	safe_delete(pUser);
	safe_delete(pPassword);
	safe_delete(pDatabase);
	pHost = strcpy(new char[strlen(iHost) + 1], iHost);
	pUser = strcpy(new char[strlen(iUser) + 1], iUser);
	pPassword = strcpy(new char[strlen(iPassword) + 1], iPassword);
	pDatabase = strcpy(new char[strlen(iDatabase) + 1], iDatabase);
	pCompress = iCompress;
	pPort = iPort;
	pSSL = iSSL;
	return open(errnum, errbuf);
}

bool DatabaseConnection::open(uint32* errnum, char* errbuf) {
	if (errbuf)
		errbuf[0] = 0;
	if (mStatus == Connected)
		return true;
	if (mStatus == Error) {
		mysql_close(mMYSQL);
		mysql_init(mMYSQL);		// Initialize structure again
	}
	if (!pHost)
		return false;
	/*
	Added CLIENT_FOUND_ROWS flag to the connect
	otherwise DB update calls would say 0 rows affected when the value already equalled
	what the function was tring to set it to, therefore the function would think it failed
	*/
	uint32 flags = CLIENT_FOUND_ROWS;
	if (pCompress)
		flags |= CLIENT_COMPRESS;
	if (pSSL)
		flags |= CLIENT_SSL;
	if (mysql_real_connect(mMYSQL, pHost, pUser, pPassword, pDatabase, pPort, 0, flags)) {
		mStatus = Connected;
		return true;
	}
	else {
		if (errnum)
			*errnum = mysql_errno(mMYSQL);
		if (errbuf)
			snprintf(errbuf, MYSQL_ERRMSG_SIZE, "#%i: %s", mysql_errno(mMYSQL), mysql_error(mMYSQL));
		mStatus = Error;
		return false;
	}
}