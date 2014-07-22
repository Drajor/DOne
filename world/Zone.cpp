#include "Zone.h"
#include "World.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "Constants.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "../common/eq_packet_structs.h"
#include "LogSystem.h"

Zone::Zone(World* pWorld, DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID) :
	mWorld(pWorld),
	mDataStore(pDataStore),
	mPort(pPort),
	mID(pZoneID),
	mInstanceID(pInstanceID),
	mInitialised(false),
	mStreamFactory(nullptr),
	mStreamIdentifier(nullptr),
	mNextSpawnID(1)
{
}

Zone::~Zone() {
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
}

bool Zone::initialise() {
	// Prevent multiple initialisation.
	if (mInitialised) return false;

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(ZoneStream);
	if (!mStreamFactory->Open(mPort)) {
		return false;
	}

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	mInitialised = true;
	return true;
}

void Zone::addAuthentication(ClientAuthentication& pAuthentication, std::string pCharacterName) {
	mAuthenticatedCharacters.insert(std::make_pair(pCharacterName, pAuthentication));
}

void Zone::removeAuthentication(std::string pCharacterName) {
	mAuthenticatedCharacters.erase(pCharacterName);
}

bool Zone::checkAuthentication(std::string pCharacterName) {
	for (auto i : mAuthenticatedCharacters) {
		if (i.first == pCharacterName) {
			Log::info("[Zone] Authentication Passed");
			return true;
		}
	}

	Log::error("[Zone] checkAuthentication is returning false. This is unexpected.");
	return false;
}

void Zone::update() {
	// Check if any new clients are connecting to this Zone.
	_handleIncomingConnections();

	for (auto i : mZoneClientConnections)
		i->update();

	for (auto i = mCharacters.begin(); i != mCharacters.end();) {
		// Remove Characters that logged out.
		if ((*i)->getLoggedOut()) {
			ZoneClientConnection* connection = (*i)->getConnection();
			mZoneClientConnections.remove(connection);
			//mZoneClientConnections.erase(connection);
			delete connection;
			delete *i;
			i = mCharacters.erase(i);
		}
		else {
			(*i)->update();
			i++;
		}
	}

	//for (auto i = mClientConnections.begin(); i != mClientConnections.end();) {
	//	if ((*i)->update()){
	//		i++;
	//	}
	//	else {
	//		delete *i;
	//		i = mClientConnections.erase(i);
	//	}
	//}
}

void Zone::shutdown()
{

}

void Zone::_handleIncomingConnections() {
	// Check for incoming connections.
	EQStream* incomingStream = nullptr;
	while (incomingStream = mStreamFactory->Pop()) {
		// Hand over to the EQStreamIdentifier. (Determine which client to user has)
		mStreamIdentifier->AddStream(incomingStream);
	}

	mStreamIdentifier->Process();

	// Check for identified streams.
	EQStreamInterface* incomingStreamInterface = nullptr;
	while (incomingStreamInterface = mStreamIdentifier->PopIdentified()) {
		Log::info("[Zone] New Zone Client Connection");
		mZoneClientConnections.push_back(new ZoneClientConnection(incomingStreamInterface, mDataStore, this));
	}
}

void Zone::moveCharacter(Character* pCharacter, float pX, float pY, float pZ) {
	pCharacter->setPosition(pX, pY, pZ, 0);
	pCharacter->getConnection()->sendPosition();
}

void Zone::notifyCharacterLogOut(Character* pCharacter)
{
	// TODO: Tell Everyone!
}

void Zone::notifyCharacterZoneOut(Character* pCharacter)
{
	// TODO: Tell Everyone!
}

void Zone::notifyCharacterZoneIn(Character* pCharacter) {
	// Notify players in zone.
	ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = sender->makeCharacterSpawnPacket();
	for (auto i : mZoneClientConnections) {
		if(i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);

	// Notify character zoning in of zone spawns.
	const unsigned int numCharacters = mZoneClientConnections.size();
	if (numCharacters > 1) {
		EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ZoneSpawns, sizeof(NewSpawn_Struct)* numCharacters);
		NewSpawn_Struct* spawns = reinterpret_cast<NewSpawn_Struct*>(outPacket->pBuffer);
		int index = 0;
		for (auto i : mZoneClientConnections) {
			i->populateSpawnStruct(&spawns[index]);
			index++;
		}

		sender->sendPacket(outPacket);
		safe_delete(outPacket);
	}
	
}


void Zone::notifyCharacterPositionChanged(Character* pCharacter) {
	// Notify players in zone.
	ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = pCharacter->getConnection()->makeCharacterPositionUpdate();
	for (auto i : mZoneClientConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}


void Zone::notifyCharacterLinkDead(Character* pCharacter)
{
	// TODO: Tell Everyone!
}

void Zone::notifyCharacterAFK(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::AFK, pCharacter->getAFK()); }
void Zone::notifyCharacterShowHelm(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::ShowHelm, pCharacter->getShowHelm()); }
void Zone::notifyCharacterAnonymous(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::Anonymous, pCharacter->getAnonymous()); }
void Zone::notifyCharacterStanding(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::Animation, SpawnAppearanceAnimation::Standing); }
void Zone::notifyCharacterSitting(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::Animation, SpawnAppearanceAnimation::Sitting); }
void Zone::notifyCharacterCrouching(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::Animation, SpawnAppearanceAnimation::Crouch); }

void Zone::_sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* appearance = reinterpret_cast<SpawnAppearance_Struct*>(outPacket->pBuffer);
	appearance->spawn_id = pCharacter->getSpawnID();
	appearance->type = pType;
	appearance->parameter = pParameter;

	for (auto i : mZoneClientConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

void Zone::notifyCharacterChatSay(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, CH_SAY, pMessage);
}

void Zone::notifyCharacterChatShout(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, CH_SHOUT, pMessage);
}

void Zone::notifyCharacterChatAuction(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, CH_AUCTION, pMessage);

	// TODO: Server Auction
}

void Zone::notifyCharacterChatOOC(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, CH_OOC, pMessage);
	
	// TODO: Server OOC
}

void Zone::_sendChat(Character* pCharacter, ChannelID pChannel, const std::string pMessage) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ChannelMessage, sizeof(ChannelMessage_Struct)+pMessage.length() + 1);
	ChannelMessage_Struct* payload = (ChannelMessage_Struct*)outPacket->pBuffer;
	payload->language = Language::COMMON_TONGUE_LANG;
	payload->skill_in_language = 0;
	payload->chan_num = pChannel;
	strcpy(payload->message, pMessage.c_str());
	strcpy(payload->sender, pCharacter->getName().c_str());

	for (auto i : mZoneClientConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

