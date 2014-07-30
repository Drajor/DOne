#include "Zone.h"
#include "ZoneData.h"
#include "World.h"
#include "ZoneManager.h"
#include "GroupManager.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "Constants.h"
#include "DataStore.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "../common/eq_packet_structs.h"
#include "LogSystem.h"

Zone::Zone(World* pWorld, ZoneManager* pZoneManager, GroupManager* pGroupManager, DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID) :
	mWorld(pWorld),
	mZoneManager(pZoneManager),
	mGroupManager(pGroupManager),
	mDataStore(pDataStore),
	mPort(pPort),
	mID(pZoneID),
	mInstanceID(pInstanceID),
	mInitialised(false),
	mStreamFactory(nullptr),
	mStreamIdentifier(nullptr),
	mNextSpawnID(1),
	mLongName("Unknown Zone"),
	mShortName("Unknown Zone")
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

	mLongNameStringID = ZoneData::getLongNameStringID(mID);
	mLongName = ZoneData::getLongName(mID);
	mShortName = ZoneData::getShortName(mID);

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

void Zone::addCharacter(Character* pCharacter) {
	mCharacters.push_back(pCharacter);
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
void Zone::notifyCharacterGM(Character* pCharacter){ _sendSpawnAppearance(pCharacter, SpawnAppearanceType::GM, pCharacter->getGM(), true); }

void Zone::_sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter, bool pIncludeSender) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* appearance = reinterpret_cast<SpawnAppearance_Struct*>(outPacket->pBuffer);
	appearance->spawn_id = pCharacter->getSpawnID();
	appearance->type = pType;
	appearance->parameter = pParameter;

	if (pIncludeSender) {
		for (auto i : mZoneClientConnections) {
			i->sendPacket(outPacket);
		}
	}
	else {
		for (auto i : mZoneClientConnections) {
			if (i != sender)
				i->sendPacket(outPacket);
		}
	}
	safe_delete(outPacket);
}

void Zone::notifyCharacterChatSay(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, ChannelID::CH_SAY, pMessage);
}

void Zone::notifyCharacterChatShout(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, ChannelID::CH_SHOUT, pMessage);
}

void Zone::notifyCharacterChatAuction(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, ChannelID::CH_AUCTION, pMessage);

	// TODO: Server Auction
}

void Zone::notifyCharacterChatOOC(Character* pCharacter, const std::string pMessage) {
	_sendChat(pCharacter, ChannelID::CH_OOC, pMessage);
	
	// TODO: Server OOC
}

void Zone::notifyCharacterEmote(Character* pCharacter, const std::string pMessage) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_Emote, 4 + pMessage.length() + pCharacter->getName().length() + 2);
	Emote_Struct* payload = reinterpret_cast<Emote_Struct*>(outPacket->pBuffer);
	char* Buffer = (char*)payload;
	Buffer += 4;
	snprintf(Buffer, sizeof(Emote_Struct)-4, "%s %s", pCharacter->getName().c_str(), pMessage.c_str());

	for (auto i : mZoneClientConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

void Zone::_sendChat(Character* pCharacter, ChannelID pChannel, const std::string pMessage) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ChannelMessage, sizeof(ChannelMessage_Struct)+pMessage.length() + 1);
	ChannelMessage_Struct* payload = (ChannelMessage_Struct*)outPacket->pBuffer;
	payload->language = Language::COMMON_TONGUE_LANG;
	payload->skill_in_language = 0;
	payload->chan_num = static_cast<std::uint32_t>(pChannel);
	strcpy(payload->message, pMessage.c_str());
	strcpy(payload->sender, pCharacter->getName().c_str());

	for (auto i : mZoneClientConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

void Zone::notifyCharacterChatTell(Character* pCharacter, const std::string& pTargetName, const std::string& pMessage) {
	mZoneManager->notifyCharacterChatTell(pCharacter, pTargetName, pMessage);
}

bool Zone::trySendTell(const std::string& pSenderName, const std::string& pTargetName, const std::string& pMessage) {
	for (auto i : mCharacters) {
		if (i->getName() == pTargetName) {
			i->getConnection()->sendTell(pSenderName, pMessage);
			return true;
		}
	}

	// Character not in this zone.
	return false;
}

void Zone::notifyCharacterAnimation(Character* pCharacter, uint8 pAction, uint8 pAnimationID, bool pIncludeSender) {
	auto sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_Animation, sizeof(Animation_Struct));
	Animation_Struct* payload = reinterpret_cast<Animation_Struct*>(outPacket->pBuffer);
	payload->spawnid = pCharacter->getSpawnID();
	payload->action = pAction;
	payload->value = pAnimationID;

	if (pIncludeSender) {
		for (auto i : mZoneClientConnections) {
			i->sendPacket(outPacket);
		}
	}
	else {
		for (auto i : mZoneClientConnections) {
			if ( i != sender)
				i->sendPacket(outPacket);
		}
	}
	safe_delete(outPacket);
}

void Zone::notifyCharacterLevelIncrease(Character* pCharacter) {
	// Notify user client.
	_sendCharacterLevel(pCharacter);
	_sendLevelAppearance(pCharacter);
}

void Zone::notifyCharacterLevelDecrease(Character* pCharacter) {
	// Notify user client.
	_sendCharacterLevel(pCharacter);
}

void Zone::_sendLevelAppearance(Character* pCharacter) {
	auto outPacket = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));
	auto payload = reinterpret_cast<LevelAppearance_Struct*>(outPacket->pBuffer);
	payload->parm1 = 0x4D;
	payload->parm2 = payload->parm1 + 1;
	payload->parm3 = payload->parm2 + 1;
	payload->parm4 = payload->parm3 + 1;
	payload->parm5 = payload->parm4 + 1;
	payload->spawn_id = pCharacter->getSpawnID();
	payload->value1a = 1;
	payload->value2a = 2;
	payload->value3a = 1;
	payload->value3b = 1;
	payload->value4a = 1;
	payload->value4b = 1;
	payload->value5a = 2;

	for (auto i : mZoneClientConnections) {
		i->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}

void Zone::_sendCharacterLevel(Character* pCharacter) {
	_sendSpawnAppearance(pCharacter, SpawnAppearanceType::WhoLevel, pCharacter->getLevel());
}

void Zone::requestSave(Character*pCharacter) {
	// Save taking 7ms - 90ms ... I will have to do something about that eventually.
	// http://dev.mysql.com/doc/refman/5.5/en/too-many-connections.html
	// Considering a DB connection per user and just copy data to another thread.
	if (!mDataStore->saveCharacter(pCharacter->getID(), pCharacter->getProfile(), pCharacter->getExtendedProfile())) {
		pCharacter->getConnection()->sendMessage(MessageType::Red, "[ERROR] There was an error saving your character. I suggest you log out.");
		Log::error("[Zone] Failed to save character");
	}
}

void Zone::whoRequest(Character* pCharacter, WhoFilter& pFilter) {
	// /who all
	if (pFilter.mType == WHO_WORLD) {
		mZoneManager->whoAllRequest(pCharacter, pFilter);
	}
	// /who
	else if (pFilter.mType == WHO_ZONE) {
		_handleWhoRequest(pCharacter, pFilter);
	}
}

void Zone::_handleWhoRequest(Character* pCharacter, WhoFilter& pFilter) {
	std::list<Character*> matches;
	getWhoMatches(matches, pFilter);
	pCharacter->getConnection()->sendWhoResults(matches);
}

void Zone::getWhoMatches(std::list<Character*>& pMatches, WhoFilter& pFilter) {
	// Search zone for matches to pFilter.
	pMatches.insert(pMatches.begin(), mCharacters.begin(), mCharacters.end());
}

void Zone::notifyCharacterGroupInvite(Character* pCharacter, const std::string pToCharacterName) {
	// Search our Zone first.
	Character* toCharacter = findCharacter(pToCharacterName);
	if (toCharacter) {
		// Character already grouped.
		if (toCharacter->hasGroup()) {

		}
		else {
			toCharacter->getConnection()->sendGroupInvite(pCharacter->getName());
		}
		return;
	}

	// Search all zones.
	toCharacter = mZoneManager->findCharacter(pToCharacterName, false, this);
	if (toCharacter) {
		return;
	}
	
	// Player was not found (as per Live).
	std::stringstream ss;
	ss << "Player " << pToCharacterName << " was not found.";
	pCharacter->getConnection()->sendMessage(MessageType::Red, ss.str());
}

Character* Zone::findCharacter(const std::string pCharacterName) {
	for (auto i : mCharacters) {
		if (i->getName() == pCharacterName)
			return i;
	}

	return nullptr;
}

Character* Zone::_findCharacter(const std::string& pCharacterName, bool pIncludeZoning) {
	// Search locally first.
	Character* character = findCharacter(pCharacterName);
	if (character) return character;

	// Proceed to global search.
	return mZoneManager->findCharacter(pCharacterName, pIncludeZoning, this);
}


void Zone::notifyCharacterAcceptGroupInvite(Character* pCharacter, std::string pToCharacterName) {
	// Search our Zone first.
	Character* toCharacter = findCharacter(pToCharacterName);
	if (toCharacter) {
		// Joining existing group.
		if (toCharacter->hasGroup()) {

		}
		// Starting a new group.
		else {
			mGroupManager->makeGroup(toCharacter, pCharacter);
		}
	}
}

void Zone::notifyCharacterDeclineGroupInvite(Character* pCharacter, std::string pToCharacterName)
{
	
}

void Zone::notifyCharacterGroupDisband(Character* pCharacter, const std::string& pRemoveCharacterName) {
	Character* removeCharacter = _findCharacter(pRemoveCharacterName); // TODO: Disbanding zoning characters?
	if (!removeCharacter) {
		Log::error("[Zone] Attempting to remove Character from group that does not exist.");
		return;
	}

	mGroupManager->removeMemberRequest(pCharacter, removeCharacter);
	//// Check: Same group.
	//if (pCharacter->getGroup() != removeCharacter->getGroup()) {
	//	Log::error("[Zone] Attempting to remove Character ");
	//	return;
	//}
	//Group* group = pCharacter->getGroup();
}

void Zone::notifyCharacterChatGroup(Character* pCharacter, const std::string pMessage) {
	// Check: Character has a group.
	if (pCharacter->hasGroup()) {
		mGroupManager->chatSent(pCharacter, pMessage);
		return;
	}

	// Log: De-sync or hacker
	std::stringstream ss; ss << "[Zone] Character(" << pCharacter->getName() << ") sent group message while not grouped.";
	Log::error(ss.str());
}
