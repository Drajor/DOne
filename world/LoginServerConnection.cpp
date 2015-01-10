#include "LoginServerConnection.h"

#include "../common/debug.h"
#include "../common/version.h"


#include "../common/EmuTCPConnection.h"
#include "../common/servertalk.h"

#include "Utility.h"
#include "World.h"
#include "AccountManager.h"
#include "LogSystem.h"
#include "Payload.h"

LoginServerConnection::LoginServerConnection(World* pWorld) :
	mWorld(pWorld)
{
	mTCPConnection = new EmuTCPConnection(true);
	mTCPConnection->SetPacketMode(EmuTCPConnection::packetModeLogin);
}

LoginServerConnection::~LoginServerConnection() {
	delete mTCPConnection;
}

bool LoginServerConnection::initialise(const String& pAddress, const u16 pPort) {
	Log::status("[Login Server Connection] Initialising.");
	if (isConnected()) return true;

	if (_isConnectReady()) {
		Log::status("[Login Server Connection] Connecting");
		connect(pAddress, pPort);
	}
	else {
		Log::error("[Login Server Connection] Not ready to connect.");
		return false;
	}

	Log::status("[Login Server Connection] Initialised.");
	return true;
}

bool LoginServerConnection::connect(const String& pAddress, const u16 pPort) {
	EXPECTED_BOOL(pPort != 0);

	char errbuf[TCPConnection_ErrorBufferSize];

	mLoginServerIP = ResolveIP(pAddress.c_str(), errbuf);
	if (mLoginServerIP == 0) {
		Log::error("[Login Server Connection] Unable to resolve Login Server IP");
		return false;
	}

	if (mTCPConnection->ConnectIP(mLoginServerIP, pPort, errbuf)) {
		Log::status("[Login Server Connection] Connected to Login Server");
		return true;
	}

	Log::error("[Login Server Connection] Failed to connect.");
	return false;
}

void LoginServerConnection::update() {
	using namespace Payload::LoginServer;

	// Process any incoming packets.
	auto packet = mTCPConnection->PopPacket();
	while(packet) {
		if (packet->opcode == OpCode::ConnectRequest) {
			// NOTE: This occurs when a client clicks 'X' at the Server Selection Screen.
			_handleConnectRequest(packet);
		}
		else if (packet->opcode == OpCode::ClientAuthentication) {
			// NOTE: This occurs when the Login Server receives a positive ConnectResponse.
			_handleClientAuthentication(packet);
		}

		delete packet;
		packet = mTCPConnection->PopPacket();
	}
}

void LoginServerConnection::_handleConnectRequest(ServerPacket* pPacket) {
	using namespace Payload::LoginServer;
	EXPECTED(pPacket);
	EXPECTED(ConnectRequest::sizeCheck(pPacket));

	auto payload = ConnectRequest::convert(pPacket);

	// Notify World.
	mWorld->handleConnectRequest(payload->mAccountID);
}

void LoginServerConnection::sendConnectResponse(const u32 pAccountID, const u8 pResponse) {
	using namespace Payload::LoginServer;

	auto packet = new ServerPacket(OpCode::ConnectResponse, ConnectResponse::size());
	auto payload = ConnectResponse::convert(packet->pBuffer);

	payload->mAccountID = pAccountID;
	payload->mResponse = pResponse;
	
	_sendPacket(packet);
	delete packet;
}

void LoginServerConnection::_handleClientAuthentication(ServerPacket* pPacket) {
	//using namespace Payload::LoginServer;
	EXPECTED(Payload::LoginServer::ClientAuthentication::sizeCheck(pPacket));

	auto payload = Payload::LoginServer::ClientAuthentication::convert(pPacket);

	// Notify World.
	mWorld->handleClientAuthentication(payload->mAccountID, payload->mAccountName, payload->mKey, payload->mIP);
}

void LoginServerConnection::sendWorldInformation(const String& pAccount, const String& pPassword, const String& pLongName, const String& pShortName) {
	using namespace Payload::LoginServer;

	auto packet = new ServerPacket(OpCode::WorldInformation, WorldInformation::size());
	auto payload = WorldInformation::convert(packet);

	strcpy(payload->mAccount, pAccount.c_str());
	strcpy(payload->mPassword, pPassword.c_str());
	strcpy(payload->mLongName, pLongName.c_str());
	strcpy(payload->mShortName, pShortName.c_str());
	strcpy(payload->protocolversion, EQEMU_PROTOCOL_VERSION);
	strcpy(payload->serverversion, LOGIN_VERSION);

	_sendPacket(packet);
	delete packet;
}

void LoginServerConnection::sendWorldStatus(const i32 pStatus, const i32 pPlayers, const i32 pZones) {
	using namespace Payload::LoginServer;

	auto packet = new ServerPacket(OpCode::WorldStatus, WorldStatus::size());
	auto payload = WorldStatus::convert(packet);

	payload->mStatus = pStatus;
	payload->mPlayers = pPlayers;
	payload->mZones = pZones;

	_sendPacket(packet);
	delete packet;
}

void LoginServerConnection::_sendPacket(ServerPacket* pPacket) { mTCPConnection->SendPacket(pPacket); }
bool LoginServerConnection::_isConnectReady() { return mTCPConnection->ConnectReady(); }
bool LoginServerConnection::isConnected() { return mTCPConnection->Connected(); }
