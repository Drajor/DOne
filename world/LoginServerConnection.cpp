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

#include "Utility.h"
#include "World.h"
#include "AccountManager.h"
#include "LogSystem.h"

static const int StatusUpdateInterval = 15000;

LoginServerConnection::LoginServerConnection(World* pWorld, const char* pAddress, uint16 pPort, const char* pAccountName, const char* pPassword) :
	mStatusUpdateTimer(StatusUpdateInterval),
	mWorld(pWorld),
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
	if (mStatusUpdateTimer.Check()) {
		sendWorldStatus();
		mStatusUpdateTimer.Start();
	}

	/************ Get all packets from packet manager out queue and process them ************/
	ServerPacket* packet = 0;
	while((packet = mTCPConnection->PopPacket()))
	{
		_log(WORLD__LS_TRACE,"Recevied ServerPacket from LS OpCode 0x04x",packet->opcode);
		_hex(WORLD__LS_TRACE,packet->pBuffer,packet->size);

		switch(packet->opcode) {
			case 0:
				break;
			case ServerOP_KeepAlive: {
				break;
			}
			case ServerOP_UsertoWorldReq: {
				_handleUserToWorldRequest(packet);
				break;
			}
			case ServerOP_LSClientAuth: {
				_handleLoginServerClientAuth(packet);
				break;
			}
			case ServerOP_LSFatalError: {
				// Ignored. Public LS may or may not send this, local LS does not.
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
				_log(WORLD__LS_ERR, "Unknown LSOpCode: 0x%04x size=%d",(int)packet->opcode,packet->size);
				DumpPacket(packet->pBuffer, packet->size);
				break;
			}
		}
		delete packet;
	}
}

bool LoginServerConnection::initialise() {
	if (isConnected()) return true;

	if (_isConnectReady()) {
		Log::status("[Login Server Connection] Connecting");
		connect();
	} else {
		Log::error("[Login Server Connection] Not ready to connect.");
		return false;
	}

	return true;
}

bool LoginServerConnection::connect() {
	char errbuf[TCPConnection_ErrorBufferSize];

	mLoginServerIP = ResolveIP(mLoginServerAddress, errbuf);
	if (mLoginServerIP == 0) {
		Log::error("[Login Server Connection] Unable to resolve Login Server IP");
		return false;
	}

	if (mLoginServerPort == 0) {
		Log::error("[Login Server Connection] Login Server port not set.");
		return false;
	}

	if (mTCPConnection->ConnectIP(mLoginServerIP, mLoginServerPort, errbuf)) {
		_sendWorldInformation();
		sendWorldStatus();
		Log::status("[Login Server Connection] Connected to Login Server");
		return true;
	}

	Log::error("[Login Server Connection] Failed to connect.");
	return false;
}

void LoginServerConnection::_handleUserToWorldRequest(ServerPacket* pPacket) {
	if (pPacket->size != sizeof(UsertoWorldRequest_Struct)) {
		Log::error("[Login Server Connection] Wrong size of UsertoWorldRequest_Struct");
		return;
	}
	UsertoWorldRequest_Struct* inPayload = reinterpret_cast<UsertoWorldRequest_Struct*>(pPacket->pBuffer);
	ServerPacket* outPacket = new ServerPacket(ServerOP_UsertoWorldResp, sizeof(UsertoWorldResponse_Struct));
	UsertoWorldResponse_Struct* outPayload = reinterpret_cast<UsertoWorldResponse_Struct*>(outPacket->pBuffer);
	outPayload->lsaccountid = inPayload->lsaccountid;
	outPayload->ToID = inPayload->FromID;
	outPayload->worldid = inPayload->worldid;
	// Ask World if this Client can join World.
	outPayload->response = mWorld->getUserToWorldResponse(inPayload->lsaccountid);
	_sendPacket(outPacket);
	safe_delete(outPacket);
}

void LoginServerConnection::_handleLoginServerClientAuth(ServerPacket* pPacket) {
	if (pPacket->size != sizeof(ServerLSClientAuth)) {
		Log::error("[Login Server Connection] Wrong size of ServerLSClientAuth");
		return;
	}
	ServerLSClientAuth* payload = reinterpret_cast<ServerLSClientAuth*>(pPacket->pBuffer);
	// Tell World that a Client is inbound.
	mWorld->notifyIncomingClient(payload->lsaccount_id, payload->name, payload->key, payload->worldadmin, payload->ip, payload->local);
}

void LoginServerConnection::_sendWorldInformation() {
	ServerPacket* outPacket = new ServerPacket(ServerOP_NewLSInfo, sizeof(ServerNewLSInfo_Struct));
	ServerNewLSInfo_Struct* payload = reinterpret_cast<ServerNewLSInfo_Struct*>(outPacket->pBuffer);
	strcpy(payload->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(payload->serverversion, LOGIN_VERSION);
	strcpy(payload->name, "DrajorTest"); // TODO: (Configuration)
	strcpy(payload->shortname, "[TEST] Drajor");
	strcpy(payload->account, mLoginAccount);
	strcpy(payload->password, mLoginPassword);

	_sendPacket(outPacket);
	safe_delete(outPacket);
}

void LoginServerConnection::sendWorldStatus() {
	ServerPacket* outPacket = new ServerPacket(ServerOP_LSStatus, sizeof(ServerLSStatus_Struct));
	ServerLSStatus_Struct* payload = reinterpret_cast<ServerLSStatus_Struct*>(outPacket->pBuffer);

	if (mWorld->getLocked())
		payload->status = -2;
	else payload->status = 100;

	payload->num_zones = 100; // TODO:
	payload->num_players = 100;
	_sendPacket(outPacket);
	safe_delete(outPacket);
}

void LoginServerConnection::_sendPacket(ServerPacket* pPacket) { mTCPConnection->SendPacket(pPacket); }
bool LoginServerConnection::_isConnectReady() { return mTCPConnection->ConnectReady(); }
bool LoginServerConnection::isConnected() { return mTCPConnection->Connected(); }
