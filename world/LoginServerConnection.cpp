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

LoginServerConnection::~LoginServerConnection() {
	if (mConnection) {
		delete mConnection;
		mConnection = nullptr;
	}

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
	
}

const bool LoginServerConnection::initialise(World* pWorld, ILog* pLog, const String& pAddress, const u16 pPort) {
	if (mInitialised) return false;
	if (!pWorld) return false;
	if (!pLog) return false;

	mWorld = pWorld;
	mLog = pLog;
	mLog->setContext("[LoginServerConnection ID: " + toString(getID()) + "]");
	mLog->status("Initialising.");

	mConnection = new EmuTCPConnection(true);
	mConnection->SetPacketMode(EmuTCPConnection::packetModeLogin);

	if (!connect(pAddress, pPort)) {
		mLog->error("Failed to connect.");
		return false;
	}
	mLog->status("Connected.");
	
	mInitialised = true;
	mLog->status("Finished initialising.");
	return true;
}

const bool LoginServerConnection::connect(const String& pAddress, const u16 pPort) {
	char errbuf[1024];

	// Resolve IP.
	mLoginServerIP = ResolveIP(pAddress.c_str(), errbuf);
	if (mLoginServerIP == 0) {
		mLog->error("Unable to resolve IP");
		return false;
	}

	if (!mConnection->ConnectIP(mLoginServerIP, pPort, errbuf)) {
		return false;
	}

	return true;
}

void LoginServerConnection::update() {
	using namespace Payload::LoginServer;

	// Process any incoming packets.
	auto packet = mConnection->PopPacket();
	while(packet) {
		if (packet->opcode == OpCode::ConnectRequest) {
			// NOTE: This occurs when a client clicks 'X' at the Server Selection Screen.
			_handleConnectRequest(packet);
		}
		else if (packet->opcode == OpCode::Authentication) {
			// NOTE: This occurs when the Login Server receives a positive ConnectResponse.
			_handleClientAuthentication(packet);
		}

		delete packet;
		packet = mConnection->PopPacket();
	}
}

void LoginServerConnection::_handleConnectRequest(ServerPacket* pPacket) {
	using namespace Payload::LoginServer;
	if (!pPacket) return;
	EXPECTED(ConnectRequest::sizeCheck(pPacket));

	auto payload = ConnectRequest::convert(pPacket);

	mLog->info("Connect request from AccountID: " + toString(payload->mAccountID));

	// Notify World.
	mWorld->onConnectRequest(this, payload->mAccountID);
}

void LoginServerConnection::sendConnectResponse(const u32 pAccountID, const u8 pResponse) {
	using namespace Payload::LoginServer;

	auto packet = new ServerPacket(OpCode::ConnectResponse, ConnectResponse::size());
	auto payload = ConnectResponse::convert(packet->pBuffer);

	payload->mAccountID = pAccountID;
	payload->mResponse = pResponse;
	
	sendPacket(packet);
	delete packet;
}

void LoginServerConnection::_handleClientAuthentication(ServerPacket* pPacket) {
	using namespace Payload::LoginServer;
	EXPECTED(Authentication::sizeCheck(pPacket));

	auto payload = Authentication::convert(pPacket);

	// Notify World.
	mWorld->onAuthentication(this, payload->mLoginAccountID, payload->mLoginAccountName, payload->mLoginKey, payload->mIP);
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

	sendPacket(packet);
	delete packet;
}

void LoginServerConnection::sendWorldStatus(const i32 pStatus, const i32 pPlayers, const i32 pZones) {
	using namespace Payload::LoginServer;

	auto packet = new ServerPacket(OpCode::WorldStatus, WorldStatus::size());
	auto payload = WorldStatus::convert(packet);

	payload->mStatus = pStatus;
	payload->mPlayers = pPlayers;
	payload->mZones = pZones;

	sendPacket(packet);
	delete packet;
}

void LoginServerConnection::sendPacket(ServerPacket* pPacket) { mConnection->SendPacket(pPacket); }
