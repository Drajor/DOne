/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/debug.h"
#include "../common/version.h"

#ifdef _WINDOWS
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <pthread.h>
	#include <unistd.h>
	#include <errno.h>

	#include "../common/unix.h"

	#define SOCKET_ERROR -1
	#define INVALID_SOCKET -1
	extern int errno;
#endif

#include "LoginServerConnection.h"
#include "../common/EmuTCPConnection.h"
#include "../common/servertalk.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/StringUtil.h"

#include "worlddb.h"
#include "WorldConfig.h"
#include "Utility.h"
#include "World.h"
#include "AccountManager.h"

extern uint32 numzones;
extern uint32 numplayers;

static const int StatusUpdateInterval = 15000;

LoginServerConnection::LoginServerConnection(World* pWorld, AccountManager* pAccountManager, const char* pAddress, uint16 pPort, const char* pAccountName, const char* pPassword) :
	mStatusUpdateTimer(StatusUpdateInterval),
	mWorld(pWorld),
	mAccountManager(pAccountManager),
	mTCPConnection(0)
{
	strn0cpy(mLoginServerAddress,pAddress,256);
	mLoginServerPort = pPort;
	strn0cpy(mLoginAccount,pAccountName,31);
	strn0cpy(mLoginPassword,pPassword,31);
	mTCPConnection = new EmuTCPConnection(true);
	mTCPConnection->SetPacketMode(EmuTCPConnection::packetModeLogin);
}

LoginServerConnection::~LoginServerConnection() {
	delete mTCPConnection;
}

void LoginServerConnection::update() {
	//const WorldConfig *Config=WorldConfig::get();

	if (mStatusUpdateTimer.Check()) {
		this->sendWorldStatus();
	}

	/************ Get all packets from packet manager out queue and process them ************/
	ServerPacket *pack = 0;
	while((pack = mTCPConnection->PopPacket()))
	{
		_log(WORLD__LS_TRACE,"Recevied ServerPacket from LS OpCode 0x04x",pack->opcode);
		_hex(WORLD__LS_TRACE,pack->pBuffer,pack->size);

		switch(pack->opcode) {
			case 0:
				break;
			case ServerOP_KeepAlive: {
				// Ignored.
				Utility::print("ServerOP_KeepAlive");
				break;
			}
			case ServerOP_UsertoWorldReq: {
				Utility::print("ServerOP_UsertoWorldReq");
				UsertoWorldRequest_Struct* utwr = (UsertoWorldRequest_Struct*) pack->pBuffer;
				// TODO: Below.
				//uint32 id = mAccountManager->getWorldAccountID(utwr->lsaccountid);
				uint32 id = database.GetAccountIDFromLSID(utwr->lsaccountid);
				int16 accountStatus = database.CheckStatus(id);

				ServerPacket* outpack = new ServerPacket;
				outpack->opcode = ServerOP_UsertoWorldResp;
				outpack->size = sizeof(UsertoWorldResponse_Struct);
				outpack->pBuffer = new uchar[outpack->size];
				memset(outpack->pBuffer, 0, outpack->size);
				UsertoWorldResponse_Struct* utwrs = (UsertoWorldResponse_Struct*) outpack->pBuffer;
				utwrs->lsaccountid = utwr->lsaccountid;
				utwrs->ToID = utwr->FromID;
				utwrs->worldid = utwr->worldid;

				//utwrs->response = 1; // Normal, everything is OK.
				//utwrs->response = 0; // 'That server currently unavailable. Please check the EverQuest webpage for current server status and try again later'.
				//utwrs->response = -1; // 'This account is currently suspended. Please contact customer service for more information.'
				//utwrs->response = -2; // 'This account is currently banned. Please contact customer service for more information.'
				//utwrs->response = -3; // 'The world server has denied your login request. Please try again later.'

				static const int16 ACCOUNT_STATUS_SUSPENDED = -1;
				static const int16 ACCOUNT_STATUS_BANNED = -2;

				// Assume everything is fine.
				utwrs->response = 1;

				// Check Suspended.
				if (accountStatus == ACCOUNT_STATUS_SUSPENDED)
					utwrs->response = -1;
				// Check Banned.
				if (accountStatus == ACCOUNT_STATUS_BANNED)
					utwrs->response = -2;

				// Special case: Server is locked.
				if (mWorld->getLocked() && accountStatus >= 0 ) {
					utwrs->response = 0; // unsuspended/unbanned clients can not join locked server.
					if (accountStatus >= 100) utwrs->response = 1; // GM/Admin may enter locked server.
				}
				// NOTE: There was a -3 here previously for 'MaxClients'.

				_sendPacket(outpack);
				delete outpack;
				break;
			}
			case ServerOP_LSClientAuth: {
				Utility::print("ServerOP_LSClientAuth");
				ServerLSClientAuth* slsca = (ServerLSClientAuth*) pack->pBuffer;

				//if (RuleI(World, AccountSessionLimit) >= 0) {
				//	// Enforce the limit on the number of characters on the same account that can be
				//	// online at the same time.
				//	//client_list.EnforceSessionLimit(slsca->lsaccount_id);
				//}

				// TODO LOL
				//client_list.CLEAdd(slsca->lsaccount_id, slsca->name, slsca->key, slsca->worldadmin, slsca->ip, slsca->local);

				// Notify World that a Client is inbound.
				mWorld->notifyIncomingClient(slsca->lsaccount_id, slsca->name, slsca->key, slsca->worldadmin, slsca->ip, slsca->local);
				break;
			}
			case ServerOP_LSFatalError: {
				// Ignored. Public LS may or may not send this, local LS does not.
				Utility::print("ServerOP_LSFatalError");
				break;
			}
			case ServerOP_SystemwideMessage: {
				// Ignored.
				break;
			}
			case ServerOP_LSRemoteAddr: {
				// Ignored. I am unsure if this is required or not. Local LS does not send this but the public LS may.
				break;
			}
			case ServerOP_LSAccountUpdate: {
				// Ignored.
				break;
			}
			default: {
				_log(WORLD__LS_ERR, "Unknown LSOpCode: 0x%04x size=%d",(int)pack->opcode,pack->size);
				DumpPacket(pack->pBuffer, pack->size);
				break;
			}
		}
		delete pack;
	}
}

bool LoginServerConnection::initialise() {
	if(!isConnected()) {
		if(_isConnectReady()) {
			_log(WORLD__LS, "Connecting to login server: %s:%d",mLoginServerAddress,mLoginServerPort);
			connect();
		} else {
			_log(WORLD__LS_ERR, "Not connected but not ready to connect, this is bad: %s:%d", mLoginServerAddress,mLoginServerPort);
		}
	}
	return true;
}

bool LoginServerConnection::connect() {
	char errbuf[TCPConnection_ErrorBufferSize];
	if ((mLoginServerIP = ResolveIP(mLoginServerAddress, errbuf)) == 0) {
		_log(WORLD__LS_ERR, "Unable to resolve '%s' to an IP.",mLoginServerAddress);
		return false;
	}

	if (mLoginServerIP == 0 || mLoginServerPort == 0) {
		_log(WORLD__LS_ERR, "Connect info incomplete, cannot connect: %s:%d",mLoginServerAddress,mLoginServerPort);
		return false;
	}

	if (mTCPConnection->ConnectIP(mLoginServerIP, mLoginServerPort, errbuf)) {
		_log(WORLD__LS, "Connected to Loginserver: %s:%d",mLoginServerAddress,mLoginServerPort);
		_sendWorldInformation();
		sendWorldStatus();
		return true;
	}
	else {
		_log(WORLD__LS_ERR, "Could not connect to login server: %s:%d %s",mLoginServerAddress,mLoginServerPort,errbuf);
		return false;
	}
}

void LoginServerConnection::_sendWorldInformation() {
	const WorldConfig* config = WorldConfig::get();

	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_NewLSInfo;
	pack->size = sizeof(ServerNewLSInfo_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerNewLSInfo_Struct* lsi = (ServerNewLSInfo_Struct*) pack->pBuffer;
	strcpy(lsi->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(lsi->serverversion, LOGIN_VERSION);
	strcpy(lsi->name, config->LongName.c_str());
	strcpy(lsi->shortname, config->ShortName.c_str());
	strcpy(lsi->account, mLoginAccount);
	strcpy(lsi->password, mLoginPassword);
	if (config->WorldAddress.length())
		strcpy(lsi->remote_address, config->WorldAddress.c_str());
	if (config->LocalAddress.length())
		strcpy(lsi->local_address, config->LocalAddress.c_str());
	else {
		uint16 port;
		mTCPConnection->GetSockName(lsi->local_address,&port);
		WorldConfig::SetLocalAddress(lsi->local_address);
	}
	_sendPacket(pack);
	delete pack;
}

void LoginServerConnection::sendWorldStatus() {
	mStatusUpdateTimer.Start();
	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_LSStatus;
	pack->size = sizeof(ServerLSStatus_Struct);
	pack->pBuffer = new uchar[pack->size];
	memset(pack->pBuffer, 0, pack->size);
	ServerLSStatus_Struct* lss = (ServerLSStatus_Struct*) pack->pBuffer;

	if (WorldConfig::get()->Locked)
		lss->status = -2;
	else if (numzones <= 0)
		lss->status = -2;
	else
		lss->status = numplayers;

	lss->num_zones = numzones;
	lss->num_players = numplayers;
	_sendPacket(pack);
	delete pack;
}

void LoginServerConnection::_sendPacket(ServerPacket* pPacket) { mTCPConnection->SendPacket(pPacket); }
bool LoginServerConnection::_isConnectReady() { return mTCPConnection->ConnectReady(); }
bool LoginServerConnection::isConnected() { return mTCPConnection->Connected(); }
