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
#include "Payload.h"

static const int StatusUpdateInterval = 15000;

LoginServerConnection::LoginServerConnection(World* pWorld, String pAddress, uint16 pPort, String pAccountName, String pPassword) :
	mStatusUpdateTimer(StatusUpdateInterval),
	mWorld(pWorld),
	mTCPConnection(0),
	mLoginServerAddress(pAddress),
	mLoginAccount(pAccountName),
	mLoginPassword(pPassword),
	mLoginServerPort(pPort)
{
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

	ServerPacket* packet = 0;
	while((packet = mTCPConnection->PopPacket())) {
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
				Log::error("[Login Server Connection] Got unexpected packet, ignoring.");
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

	mLoginServerIP = ResolveIP(mLoginServerAddress.c_str(), errbuf);
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
	using namespace Payload::LoginServer;
	PACKET_SIZE_CHECK(ConnectRequest::sizeCheck(pPacket->size));

	auto payload = ConnectRequest::convert(pPacket->pBuffer);

	auto outPacket = new ServerPacket(ServerOP_UsertoWorldResp, sizeof(ConnectResponse));
	auto outPayload = ConnectResponse::convert(outPacket->pBuffer);
	outPayload->mAccountID = payload->mAccountID;
	outPayload->mToID = payload->mFromID;
	outPayload->mWorldID = payload->mWorldID;
	//outPayload->mResponse = mWorld->getConnectResponse(payload->mLoginServerAccountID); // Get response from World.
	outPayload->mResponse = ResponseID::ALLOWED;
	// NOTE: Currently just allow everyone. If the LS sent the LS account name I would do a proper look up

	_sendPacket(outPacket);
	safe_delete(outPacket);
}

void LoginServerConnection::_handleLoginServerClientAuth(ServerPacket* pPacket) {
	//using namespace Payload::LoginServer;
	auto expectedSize = sizeof(Payload::LoginServer::ClientAuthentication);
	auto actualSize = pPacket->size;
	PACKET_SIZE_CHECK(Payload::LoginServer::ClientAuthentication::sizeCheck(pPacket->size));

	auto payload = Payload::LoginServer::ClientAuthentication::convert(pPacket->pBuffer);

	// Add authentication for the incoming client.
	ClientAuthentication authentication;
	authentication.mLoginServerAccountID = payload->mAccountID;
	authentication.mLoginServerAccountName = Utility::safeString(payload->mAccountName, Limits::LoginServer::MAX_ACCOUNT_NAME_LENGTH);
	authentication.mKey = Utility::safeString(payload->mKey, Limits::LoginServer::MAX_KEY_LENGTH);
	authentication.mWorldAdmin = payload->mWorldAdmin;
	authentication.mIP = payload->mIP;
	authentication.mLocal = payload->mLocal;
	mWorld->addAuthentication(authentication);

	// Check if client does not yet have an account.
	if (!mWorld->ensureAccountExists(payload->mAccountID, payload->mAccountName)) {
		Log::error("[Login Server Connection] accountCheck failed.");
	}
}

void LoginServerConnection::_sendWorldInformation() {
	auto outPacket = new ServerPacket(ServerOP_NewLSInfo, sizeof(ServerNewLSInfo_Struct));
	auto payload = reinterpret_cast<ServerNewLSInfo_Struct*>(outPacket->pBuffer);
	strcpy(payload->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(payload->serverversion, LOGIN_VERSION);
	strcpy(payload->name, "DrajorTest"); // TODO: (Configuration)
	strcpy(payload->shortname, "[TEST] Drajor");
	strcpy(payload->account, mLoginAccount.c_str());
	strcpy(payload->password, mLoginPassword.c_str());

	_sendPacket(outPacket);
	safe_delete(outPacket);
}

void LoginServerConnection::sendWorldStatus() {
	auto outPacket = new ServerPacket(ServerOP_LSStatus, sizeof(ServerLSStatus_Struct));
	auto payload = reinterpret_cast<ServerLSStatus_Struct*>(outPacket->pBuffer);

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
