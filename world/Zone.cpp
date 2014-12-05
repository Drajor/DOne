#include "Zone.h"
#include "ZoneData.h"
#include "World.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "GroupManager.h"
#include "GuildManager.h"
#include "RaidManager.h"
#include "Character.h"
#include "NPC.h"
#include "ZoneClientConnection.h"
#include "Constants.h"
#include "DataStore.h"
#include "Data.h"
#include "SpellDataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/Underfoot.h"
#include "../common/eq_packet_structs.h"
#include "LogSystem.h"
#include "Scene.h"
#include "SpawnPoint.h"
#include "SpawnPointManager.h"
#include "NPCFactory.h"
#include "Random.h"
#include "LootAllocator.h"
#include "Item.h"
#include "ItemFactory.h"
#include "Inventory.h"
#include "EventDispatcher.h"

Zone::Zone(const uint32 pPort, const uint16 pZoneID, const uint16 pInstanceID) :
	mPort(pPort),
	mID(pZoneID),
	mInstanceID(pInstanceID)
{
}

Zone::~Zone() {
	if (mStreamFactory) mStreamFactory->Close();
	// NOTE: mStreamFactory is intentionally not deleted here.
	safe_delete(mStreamIdentifier);
	safe_delete(mScene);
	safe_delete(mLootAllocator);
	safe_delete(mSpawnPointManager);
}

const bool Zone::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(ZoneStream);
	EXPECTED_BOOL(mStreamFactory->Open(mPort));

	mStreamIdentifier = new EQStreamIdentifier;
	Underfoot::Register(*mStreamIdentifier);

	mScene = new Scene(this);

	mLootAllocator = new LootAllocator();

	EXPECTED_BOOL(ZoneDataManager::getInstance().getLongNameStringID(mID, mLongNameStringID));
	EXPECTED_BOOL(ZoneDataManager::getInstance().getLongName(mID, mLongName));
	EXPECTED_BOOL(ZoneDataManager::getInstance().getShortName(mID, mShortName));
	EXPECTED_BOOL(ZoneDataManager::getInstance().getSafePoint(mID, mSafePoint));

	EXPECTED_BOOL(loadZonePoints());
	EXPECTED_BOOL(populate());

	// Load SpawnPoint Data for Zone.
	std::list<Data::SpawnPoint*> spawnPointData;
	EXPECTED_BOOL(ZoneDataManager::getInstance().getSpawnPoints(getID(), spawnPointData));

	// Load SpawnGroup Data for Zone.
	std::list<Data::SpawnGroup*> spawnGroupData;
	EXPECTED_BOOL(ZoneDataManager::getInstance().getSpawnGroups(getID(), spawnGroupData));
	
	// Pass to SpawnPointManager.
	mSpawnPointManager = new SpawnPointManager();
	EXPECTED_BOOL(mSpawnPointManager->initialise(this, spawnGroupData, spawnPointData));

	mInitialised = true;
	return true;
}

const bool Zone::loadZonePoints() {
	std::list<Data::ZonePoint*> zonePointData;
	EXPECTED_BOOL(ZoneDataManager::getInstance().getZonePoints(getID(), zonePointData));

	// Create ZonePoints.
	for (auto i : zonePointData) {
		auto zonePoint = new ZonePoint();
		mZonePoints.push_back(zonePoint);

		zonePoint->mID = i->mID;
		zonePoint->mPosition = i->mPosition;
		zonePoint->mDestinationPosition = i->mDestinationPosition;
		zonePoint->mDestinationHeading = i->mDestinationHeading;
		zonePoint->mDestinationZoneID = i->mDestinationZoneID;
		zonePoint->mDestinationInstanceID = i->mDestinationInstanceID;
	}

	return true;
}

const bool Zone::populate() {
	//EXPECTED_BOOL(mPopulated == false);

	//for (auto i : mSpawnPoints)
	//	_populate(i);

	//mPopulated = true;
	return true;
}

const bool Zone::depopulate() {
	//EXPECTED_BOOL(mPopulated);

	//for (auto i : mSpawnPoints) {
	//	auto npc = i->getNPC();
	//	if (npc) {
	//		// Unlink NPC and SpawnPoint
	//		i->setNPC(nullptr);
	//		npc->setSpawnPoint(nullptr);

	//		// Flag for destruction next update.
	//		npc->destroy();
	//	}
	//}

	//mPopulated = false;
	return true;
}

void Zone::update() {
	// Check if any new clients are connecting to this Zone.
	_handleIncomingConnections();

	_updatePreConnections();
	_updateConnections();
	_updateCharacters();
	_updateNPCs();

	mSpawnPointManager->update();

	// Check: LD Characters for removal.
	for (auto i = mLinkDeadCharacters.begin(); i != mLinkDeadCharacters.end();) {
		// Check: LD timer has finished.
		if (i->mTimer->Check()) {
			Log::info("[Zone] Removing LD Character. " + Utility::zoneLogDetails(this) + Utility::characterLogDetails(i->mCharacter));

			// Remove World Authentication - allowing them to log back in.
			World::getInstance().removeAuthentication(i->mCharacter->getAuthentication());

			// Save
			i->mCharacter->_updateForSave();
			requestSave(i->mCharacter);

			mScene->remove(i->mCharacter);
			delete i->mTimer;
			delete i->mCharacter;
			i = mLinkDeadCharacters.erase(i);
			continue;
		}
		i++;
	}
}

void Zone::_updatePreConnections() {
	// Update our pre-connections (zoning in or coming from character select).
	for (auto i = mPreConnections.begin(); i != mPreConnections.end();) {
		auto connection = *i;
		// Connection is fine, proceed as normal.
		if (connection->isConnected()) {
			connection->update();
			// 
			if (connection->isReadyForZoneIn()) {
				Character* character = connection->getCharacter();
				if (character) {
					Log::info("[Zone] Adding new Character. " + Utility::zoneLogDetails(this) + Utility::characterLogDetails(character));

					// Remove from pre-connection list.
					i = mPreConnections.erase(i);
					// Add to the main connection list.
					mConnections.push_back(connection);
					// Add Character to zone.
					mCharacters.push_back(character);
					mActors.push_back(character);

					// Tell everyone else.
					onEnterZone(character);
					// Let Character do what it needs to.
					character->onEnterZone();
				}
				else {
					// This should never occur.
					Log::info("[Zone] Connection was ready for zone in but no Character.");
					delete connection;
					i = mPreConnections.erase(i);
				}
				continue;
			}
			i++;
		}
		// Connection has been lost.
		else {
			Log::info("[Zone] Connection lost while zoning in.");
			// Disconnect while zoning or logging in.
			Character* character = connection->getCharacter();
			if (character) {
				// TODO: Group/Raid + anything else that could be interested IF this was a dc during zoning.
				delete character;
			}
			delete connection;
			i = mPreConnections.erase(i);
		}
	}
}

void Zone::_updateConnections() {
	// Update our connections.
	for (auto i = mConnections.begin(); i != mConnections.end();) {
		auto connection = *i;
		// Connection is fine, proceed as normal.
		if (connection->isConnected()) {
			connection->update();
			i++;
			continue;
		}
		// Connection has been lost.
		else {
			Character* character = connection->getCharacter();

			// Check: Character camped out.
			if (character->getCampComplete()) {
				Log::info("[Zone] Character camped. " + Utility::zoneLogDetails(this) + Utility::characterLogDetails(character));

				mScene->remove(character);

				// Save
				character->_updateForSave();
				requestSave(character);

				i = mConnections.erase(i); // Correct iterator.
				mCharacters.remove(character);
				mActors.remove(character);

				_onCamp(character);
				delete connection;
				delete character;
				continue;
			}
			// Expected: Player zoning out.
			else if (character->isZoningOut()) {
				Log::info("[Zone] Character zoning out. " + Utility::zoneLogDetails(this) + Utility::characterLogDetails(character));

				mScene->remove(character);

				// TODO: Save .. not sure yet how to handle this.
				i = mConnections.erase(i); // Correct iterator.
				character->onZoneOut();
				mCharacters.remove(character);
				mActors.remove(character);
				
				_onLeaveZone(character);
				delete connection;
				continue;
			}
			// Unexpected: Link Dead.
			else {
				Log::info("[Zone] Character LD. " + Utility::zoneLogDetails(this) + Utility::characterLogDetails(character));
				delete connection; // Free.
				i = mConnections.erase(i); // Correct iterator.

				_onLinkdead(character);
				continue;
			}
		}

	}
}

void Zone::_updateCharacters() {
	for (auto i : mCharacters)
		i->update();
}

void Zone::_updateNPCs() {
	for (auto i = mNPCs.begin(); i != mNPCs.end();) {
		NPC* npc = *i;
		if (npc->update()) {
			i++;
			continue;
		}

		i = mNPCs.erase(i);
		
		npc->onDestroy();
		removeActor(npc);
		delete npc;
	}
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
		Log::info("[Zone] New Zone Client Connection. " + Utility::zoneLogDetails(this));
		mPreConnections.push_back(new ZoneClientConnection(incomingStreamInterface, this));
	}
}

void Zone::moveCharacter(Character* pCharacter, float pX, float pY, float pZ) {
	pCharacter->setPosition(Vector3(pX, pY, pZ));
	handleActorPositionChange(pCharacter);

	// This is not ideal. Need to decide whether handleActorPositionChange should check for Character/have a parameter to send to self.
	pCharacter->getConnection()->sendPosition();
}

void Zone::onEnterZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(mScene);

	// Add Character to Scene.
	mScene->add(pCharacter);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::EnterZone, pCharacter);

	Log::info("Character " + pCharacter->getName() + " entered Zone");
}

void Zone::handleActorPositionChange(Actor* pActor) {

	// Update Scene with Actor movement
	mScene->update(pActor);

	pActor->_syncPosition();
	// Update any Character visible to pActor.
	auto position = pActor->getPositionData();
	auto packet = Payload::ActorPosition::create(position);
	for (auto i : pActor->getVisibleTo()) {
		if (i->isLinkDead() == false)
			i->getConnection()->sendPacket(packet);
	}
	packet->pBuffer = nullptr;
	safe_delete(packet);
}


void Zone::handleAFK(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::AFK, pCharacter->isAFK()); }
void Zone::handleShowHelm(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::ShowHelm, pCharacter->getShowHelm()); }
void Zone::handleAnonymous(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::Anonymous, pCharacter->getAnonymous()); }
void Zone::handleStanding(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_Animation, SpawnAppearanceAnimation::Standing); }
void Zone::handleSitting(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_Animation, SpawnAppearanceAnimation::Sitting); }
void Zone::handleCrouching(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_Animation, SpawnAppearanceAnimation::Crouch); }
void Zone::notifyCharacterGM(Character* pCharacter){ _sendSpawnAppearance(pCharacter, SpawnAppearanceType::GM, pCharacter->isGM() ? 1 : 0, true); }
void Zone::handleLinkDead(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::LinkDead, 1, false); }

void Zone::_sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter, bool pIncludeSender) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	SpawnAppearance payload;
	payload.mSpawnID = pCharacter->getSpawnID();
	payload.mType = pType;
	payload.mParameter = pParameter;

	auto outPacket = SpawnAppearance::create(payload);
	sendToVisible(pCharacter, outPacket, pIncludeSender);
	safe_delete(outPacket);
}

void Zone::handleSay(Character* pCharacter, const String pMessage) {
	EXPECTED(pCharacter);

	// Send to other Characters.
	_sendChat(pCharacter, ChannelID::CH_SAY, pMessage);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Say, pCharacter);
}

void Zone::handleShout(Character* pCharacter, const String pMessage) {
	EXPECTED(pCharacter);

	// Send to other Characters.
	_sendChat(pCharacter, ChannelID::CH_SHOUT, pMessage);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Shout, pCharacter);
}

void Zone::handleAuction(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_AUCTION, pMessage);

	// TODO: Server Auction
}

void Zone::handleOOC(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_OOC, pMessage);
	
	// TODO: Server OOC
}

void Zone::handleEmote(Character* pCharacter, const String pMessage) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_Emote, 4 + pMessage.length() + pCharacter->getName().length() + 2);
	Emote_Struct* payload = reinterpret_cast<Emote_Struct*>(outPacket->pBuffer);
	char* Buffer = (char*)payload;
	Buffer += 4;
	snprintf(Buffer, sizeof(Emote_Struct)-4, "%s %s", pCharacter->getName().c_str(), pMessage.c_str());

	for (auto i : mConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

void Zone::_sendDespawn(const uint16 pSpawnID, const bool pDecay) {
	auto outPacket = new EQApplicationPacket(OP_DeleteSpawn, sizeof(DeleteSpawn_Struct));
	auto payload = reinterpret_cast<DeleteSpawn_Struct*>(outPacket->pBuffer);
	payload->spawn_id = pSpawnID;
	payload->Decay = pDecay ? 1 : 0;

	for (auto i : mConnections) {
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

void Zone::_sendChat(Character* pCharacter, ChannelID pChannel, const String pMessage) {
	const ZoneClientConnection* sender = pCharacter->getConnection();
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ChannelMessage, sizeof(ChannelMessage_Struct)+pMessage.length() + 1);
	ChannelMessage_Struct* payload = (ChannelMessage_Struct*)outPacket->pBuffer;
	payload->language = Languages::COMMON_TONGUE_LANG;
	payload->skill_in_language = 0;
	payload->chan_num = static_cast<uint32>(pChannel);
	strcpy(payload->message, pMessage.c_str());
	strcpy(payload->sender, pCharacter->getName().c_str());

	for (auto i : mConnections) {
		if (i != sender)
			i->sendPacket(outPacket);
	}
	safe_delete(outPacket);
}

void Zone::handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage) {
	ZoneManager::getInstance().handleTell(pCharacter, pTargetName, pMessage);
}

bool Zone::trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage) {
	for (auto i : mCharacters) {
		if (i->getName() == pTargetName) {
			i->getConnection()->sendTell(pSenderName, pMessage);
			return true;
		}
	}

	// Character not in this zone.
	return false;
}

void Zone::handleAnimation(Actor* pActor, const uint8 pAnimation, const uint8 pSpeed, const bool pIncludeSender) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = Payload::Zone::ActorAnimation::construct(pActor->getSpawnID(), pAnimation, pSpeed);
	// Character animation.
	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), packet, pIncludeSender);
	}
	// NPC animation.
	else {
		sendToVisible(pActor, packet);
	}

	safe_delete(packet);
}

void Zone::handleLevelIncrease(Character* pCharacter) {
	// Notify user client.
	_sendCharacterLevel(pCharacter);
	_sendLevelAppearance(pCharacter);
}

void Zone::handleLevelDecrease(Character* pCharacter) {
	// Notify user client.
	_sendCharacterLevel(pCharacter);
}

void Zone::_sendLevelAppearance(Character* pCharacter) {
	EXPECTED(pCharacter);

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

	sendToVisible(pCharacter, outPacket, true);
	safe_delete(outPacket);
}

void Zone::_sendCharacterLevel(Character* pCharacter) {
	_sendSpawnAppearance(pCharacter, SpawnAppearanceType::WhoLevel, pCharacter->getLevel());
}

void Zone::requestSave(Character*pCharacter) {
	if (!DataStore::getInstance().saveCharacter(pCharacter->getName(), pCharacter->getData())) {
		pCharacter->getConnection()->sendMessage(MessageType::Red, "[ERROR] There was an error saving your character. I suggest you log out.");
		Log::error("[Zone] Failed to save character");
		return;
	}

	// Update the Account
	AccountManager::getInstance().updateCharacter(pCharacter->getAccountID(), pCharacter);
}

void Zone::whoRequest(Character* pCharacter, WhoFilter& pFilter) {
	// /who all
	if (pFilter.mType == WHO_WORLD) {
		ZoneManager::getInstance().whoAllRequest(pCharacter, pFilter);
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

Character* Zone::findCharacter(const String pCharacterName) {
	for (auto i : mCharacters) {
		if (i->getName() == pCharacterName)
			return i;
	}

	return nullptr;
}

Character* Zone::_findCharacter(const String& pCharacterName, bool pIncludeZoning) {
	// Search locally first.
	Character* character = findCharacter(pCharacterName);
	if (character) return character;

	// Proceed to global search.
	return ZoneManager::getInstance().findCharacter(pCharacterName, pIncludeZoning, this);
}

void Zone::handleZoneChange(Character* pCharacter, const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition) {
	EXPECTED(pCharacter);
	Log::info("handleZoneChange: " + pPosition.toString());
	// Check: Are we expecting a zone change?
	if (pCharacter->checkZoneChange(pZoneID, pInstanceID)) {
		pCharacter->setZoneAuthentication(pZoneID, pInstanceID);
		pCharacter->getConnection()->sendZoneChange(pZoneID, mInstanceID, pPosition, 1);

		// TODO: Check Position to prevent zone/warp. The position the client sends here should be what we sent them previously.

		return;
	}
	else {
		auto zp = _getClosestZonePoint(pCharacter->getPosition());
		EXPECTED(zp);
		EXPECTED(zp->mDestinationZoneID == pZoneID);
		EXPECTED(zp->mDestinationInstanceID == pInstanceID);

		float distance = zp->mPosition.distance(pCharacter->getPosition());
		Log::info("[Zone] Character is " + std::to_string(distance) + " units from ZonePoint: " + std::to_string(zp->mID));

		//if (distance <= 20.0f) {
		if (true) {

			pCharacter->setZoneChange(pZoneID, pInstanceID);
			pCharacter->setZoneAuthentication(pZoneID, pInstanceID);
			pCharacter->getConnection()->sendZoneChange(pZoneID, mInstanceID, zp->mDestinationPosition, 1);
			pCharacter->setPosition(zp->mDestinationPosition);
			pCharacter->setHeading(zp->mDestinationHeading);
			return;
		}
	}
	
	// Deny zone change.
	pCharacter->getConnection()->sendZoneChange(pZoneID, mInstanceID,  pCharacter->getPosition(), 0);
}

void Zone::notifyGuildsChanged() {
	auto outPacket = new EQApplicationPacket(OP_GuildsList);
	outPacket->size = Limits::Guild::MAX_NAME_LENGTH + (Limits::Guild::MAX_NAME_LENGTH * Limits::Guild::MAX_GUILDS); // TODO: Work out the minimum sized packet UF will accept.
	outPacket->pBuffer = reinterpret_cast<unsigned char*>(GuildManager::getInstance()._getGuildNames());
	
	for (auto i : mConnections) {
		i->sendPacket(outPacket);
	}
	outPacket->pBuffer = nullptr;
	safe_delete(outPacket);
}

void Zone::notifyCharacterGuildChange(Character* pCharacter) {
	EXPECTED(pCharacter);

	_sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_GuildID, pCharacter->getGuildID(), true);
	_sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_GuildRank, pCharacter->getGuildRank(), true);
}

void Zone::_onLeaveZone(Character* pCharacter) {
	EXPECTED(pCharacter);

	// Handle: Character leaving zone while looting.
	// (UNTESTED)
	if (pCharacter->isLooting()) {
		Actor* corpse = pCharacter->getLootingCorpse();
		corpse->setLooter(nullptr);
		pCharacter->setLootingCorpse(nullptr);
	}

	// Store Character during zoning.
	ZoneManager::getInstance().addZoningCharacter(pCharacter);

	if (pCharacter->hasGuild())
		GuildManager::getInstance().onLeaveZone(pCharacter);

	if (pCharacter->hasGroup())
		GroupManager::getInstance().onLeaveZone(pCharacter);

	if (pCharacter->hasRaid())
		RaidManager::getInstance().onLeaveZone(pCharacter);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::LeaveZone, pCharacter);
}

void Zone::_onCamp(Character* pCharacter) {
	EXPECTED(pCharacter);

	if (pCharacter->hasGuild())
		GuildManager::getInstance().onCamp(pCharacter);

	if (pCharacter->hasGroup())
		GroupManager::getInstance().onCamp(pCharacter);

	if (pCharacter->hasRaid())
		RaidManager::getInstance().onCamp(pCharacter);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Camped, pCharacter);
}

void Zone::_onLinkdead(Character* pCharacter) {
	EXPECTED(pCharacter);

	pCharacter->setLinkDead();

	// Tidy up Character
	mCharacters.remove(pCharacter); // Remove from active Character list.
	mActors.remove(pCharacter);
	pCharacter->setConnection(nullptr); // Update Character(ZCC) pointer.

	// Handle: Character going LD while looting.
	// (UNTESTED)
	if (pCharacter->isLooting()) {
		Actor* corpse = pCharacter->getLootingCorpse();
		corpse->setLooter(nullptr);
		pCharacter->setLootingCorpse(nullptr);
	}

	LinkDeadCharacter linkdeadCharacter;
	linkdeadCharacter.mTimer = new Timer(5000);
	linkdeadCharacter.mCharacter = pCharacter;
	mLinkDeadCharacters.push_back(linkdeadCharacter);

	if (pCharacter->hasGuild())
		GuildManager::getInstance().onLinkdead(pCharacter);

	if (pCharacter->hasGroup())
		GroupManager::getInstance().onLinkdead(pCharacter);

	if (pCharacter->hasRaid())
		RaidManager::getInstance().onLinkdead(pCharacter);

	handleLinkDead(pCharacter);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::ELinkDead, pCharacter);
}

void Zone::handleTarget(Character* pCharacter, const uint16 pSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	// Character is clearing their target.
	if (pSpawnID == 0) {
		pCharacter->clearTarget();
		return;
	}

	Actor* actor = findActor(pSpawnID);
	EXPECTED(actor);
	pCharacter->setTarget(actor);

	// Send HP of new target.
	auto packet = ActorHPUpdate::construct(actor->getHPPercent(), actor->getHPPercent());
	pCharacter->getConnection()->sendPacket(packet);
	safe_delete(packet);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Target, pCharacter);
}

Actor* Zone::findActor(const uint32 pSpawnID) const {
	for (auto i : mActors) {
		if (i->getSpawnID() == pSpawnID)
			return i;
	}

	return nullptr;
}

void Zone::handleFaceChange(Character* pCharacter) {
	// TODO: Notify others in zone.
}

void Zone::handleVisibilityAdd(Character* pCharacter, Actor* pAddActor) {
	EXPECTED(pCharacter);
	EXPECTED(pAddActor);

	pAddActor->_syncPosition();

	uint32 size = pAddActor->getDataSize();
	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);
	EXPECTED(pAddActor->copyData(ds));

	auto outPacket = new EQApplicationPacket(OP_ZoneEntry, data, size);
	pCharacter->getConnection()->sendPacket(outPacket);
	safe_delete(outPacket);

	// Send nimbuses
	auto nimbuses = pAddActor->getNimbuses();
	for (auto i : nimbuses) {
		pCharacter->getConnection()->sendAddNimbus(pAddActor->getSpawnID(), i);
	}
}

void Zone::handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor) {
	EXPECTED(pCharacter);
	EXPECTED(pRemoveActor);

	//Log::info(pCharacter->getName() + " can no longer see " + pRemoveActor->getName());

	// 
	auto outPacket = new EQApplicationPacket(OP_DeleteSpawn, sizeof(DeleteSpawn_Struct));
	auto payload = reinterpret_cast<DeleteSpawn_Struct*>(outPacket->pBuffer);
	payload->spawn_id = pRemoveActor->getSpawnID();
	payload->Decay = 1;
	pCharacter->getConnection()->sendPacket(outPacket);
	safe_delete(outPacket);
}

void Zone::addActor(Actor* pActor) {
	EXPECTED(pActor);

	mScene->add(pActor);
	mActors.push_back(pActor);

	if (pActor->isNPC()) {
		mNPCs.push_back(Actor::cast<NPC*>(pActor));
	}
}

void Zone::removeActor(Actor* pActor) {
	EXPECTED(pActor);
	mScene->remove(pActor);
	mActors.remove(pActor);
}

void Zone::handleSurnameChange(Actor* pActor) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto outPacket = new EQApplicationPacket(OP_GMLastName, SurnameUpdate::size());
	auto payload = SurnameUpdate::convert(outPacket->pBuffer);

	strcpy(payload->mCharaterName, pActor->getName().c_str());
	strcpy(payload->mGMName, pActor->getName().c_str());
	strcpy(payload->mLastName, pActor->getLastName().c_str());

	payload->mUnknown0[0] = 1;
	payload->mUnknown0[1] = 1;
	payload->mUnknown0[2] = 1;
	payload->mUnknown0[3] = 1;

	// Character surname changed.
	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), outPacket, true);
	}
	// NPC surname changed.
	else {
		sendToVisible(pActor, outPacket);
	}

	safe_delete(outPacket);
}

void Zone::handleTitleChanged(Character* pCharacter, const uint32 pOption) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	EQApplicationPacket* packet = nullptr;

	// Updating Title.
	if (pOption == TitleOption::Title)
		packet = TitleUpdate::construct(TitleUpdate::UPDATE_TITLE, pCharacter->getSpawnID(), pCharacter->getTitle());
	// Updating Suffix.
	else
		packet = TitleUpdate::construct(TitleUpdate::UPDATE_SUFFIX, pCharacter->getSpawnID(), pCharacter->getSuffix());

	// Update Character + those visible to
	sendToVisible(pCharacter, packet, true);
	safe_delete(packet);
}

void Zone::handleCastingBegin(Character* pCharacter, const uint16 pSlot, const uint32 pSpellID) {
	using namespace Payload::Zone;
	EXPECTED(Limits::SpellBar::slotValid(pSlot));
	EXPECTED(Limits::SpellBar::spellIDValid(pSpellID));
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->isCaster());
	EXPECTED(pCharacter->isCasting() == false);
	EXPECTED(pCharacter->hasSpell(pSlot, pSpellID)); // Check: pCharacter has the spell on the Spell Bar.
	EXPECTED(pCharacter->canCast(pSpellID)); // Check: pCharacter can cast the spell.

	auto spellData = Spell::get(pSpellID);
	EXPECTED(spellData);

	// Check: Pre-Conditions for casting
	if (pCharacter->preCastingChecks(spellData) == false) {

		//return; TODO: Once we have things sorted out more.
	}

	// Update Character state.
	EXPECTED(pCharacter->beginCasting(pSlot, pSpellID));

	//spellData->mID = 2;

	auto outPacket = new EQApplicationPacket(OP_BeginCast, BeginCast::size());
	auto payload = BeginCast::convert(outPacket->pBuffer);
	payload->mSpawnID = pCharacter->getSpawnID();
	payload->mSpellID = pSpellID;
	payload->mCastTime = 500; // TODO:

	// Update Character + those visible to
	sendToVisible(pCharacter, outPacket, true);
	safe_delete(outPacket);
}

void Zone::handleCastingFinished(Actor* pActor) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto outPacket = new EQApplicationPacket(OP_Action, Action::size());
	auto payload = Action::convert(outPacket->pBuffer);

	payload->mTargetSpawnID = pActor->getSpawnID();
	payload->mSourceSpawnID = payload->mTargetSpawnID; // TODO:
	payload->mSpellID = 1000; // TODO:
	payload->mType = 231; // Spell = 231
	payload->buff_unknown = 0;

	// Character has finished casting.
	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), outPacket, true);
	}
	// NPC has finished casting.
	else {
		sendToVisible(pActor, outPacket);
	}

	safe_delete(outPacket);
}


void Zone::sendToVisible(Character* pCharacter, EQApplicationPacket* pPacket, bool pIncludeSender) {
	EXPECTED(pCharacter);
	EXPECTED(pPacket);

	// Update Character.
	if (pIncludeSender)
		pCharacter->getConnection()->sendPacket(pPacket);

	// Update anyone who can see pCharacter.
	sendToVisible(pCharacter, pPacket);
}

void Zone::sendToVisible(Actor* pActor, EQApplicationPacket* pPacket) {
	EXPECTED(pActor);
	EXPECTED(pPacket);

	// Update anyone who can see pActor.
	for (auto i : pActor->getVisibleTo())
		i->getConnection()->sendPacket(pPacket);
}

void Zone::sendToTargeters(Actor* pActor, EQApplicationPacket* pPacket) {
	EXPECTED(pActor);
	EXPECTED(pPacket);

	// Update any Character targeting pActor.
	for (auto i : pActor->getTargeters()) {
		if (i->isCharacter()) {
			Actor::cast<Character*>(pActor)->getConnection()->sendPacket(pPacket);
		}
	}
}

void Zone::handleDeath(Actor* pActor, Actor* pKiller, const uint32 pDamage, const uint32 pSkill) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = Death::construct(pActor->getSpawnID(), pKiller ? pKiller->getSpawnID() : 0, pDamage, pSkill);
	sendToVisible(pActor, packet);
	safe_delete(packet);

	// NPC Dead.
	if (pActor->isNPC()) {
		NPC* npc = pActor->cast<NPC*>(pActor);
		
		// Check: Associated with a SpawnPoint.
		if (npc->hasSpawnPoint()) {
			// Notify SpawnPointManager
			mSpawnPointManager->onDeath(npc);
		}

		// Dispatch Event.
		EventDispatcher::getInstance().event(Event::Dead, npc);

		// Check: Empty corpse.
		if (!npc->hasCurrency() && !npc->hasItems()) {
			pActor->destroy();
			return;
		}
	}
	// Character Dead.
	else {
		Character* character = pActor->cast<Character*>(pActor);
		// Dispatch Event.
		EventDispatcher::getInstance().event(Event::Dead, character);
	}

	pActor->onDeath();
}

void Zone::handleBeginLootRequest(Character* pLooter, const uint32 pCorpseSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(pLooter);
	EXPECTED(pLooter->isLooting() == false);

	// Check: Actor exists.
	Actor* actor = findActor(pCorpseSpawnID);
	if (!actor) {
		pLooter->notify("Corpse could not be found.");
		pLooter->getConnection()->sendLootResponse(LootResponse::DENY);
		return;
	}

	// Check: Actor is a corpse.
	if (!actor->isCorpse()) {
		pLooter->notify("You can not loot that.");
		pLooter->getConnection()->sendLootResponse(LootResponse::DENY);
		return;
	}

	// Handle: Looting an NPC corpse.
	if (actor->isNPCCorpse()) {
		NPC* npcCorpse = Actor::cast<NPC*>(actor);
		// Check: Is pCharacter close enough to loot.
		if (pLooter->squareDistanceTo(npcCorpse) > 625) { // TODO: Magic.
			pLooter->getConnection()->sendLootResponse(LootResponse::TOO_FAR);
			return;
		}
		// Check: Is pCharacter allowed to loot this corpse?
		if (false) {
			pLooter->getConnection()->sendLootResponse(LootResponse::DENY);
			return;
		}
		// Check: Is someone already looting this corpse?
		if (actor->hasLooter()) {
			pLooter->getConnection()->sendLootResponse(LootResponse::ALREADY);
			return;
		}

		pLooter->setLootingCorpse(npcCorpse);
		npcCorpse->setLooter(pLooter);

		int32 platinum = 0;
		int32 gold = 0;
		int32 silver = 0;
		int32 copper = 0;
		bool currencyLooted = false;

		// Check: Does the corpse have currency on it?
		if (npcCorpse->hasCurrency()) {
			currencyLooted = true;

			// Remove currency from corpse.
			npcCorpse->getCurrency(platinum, gold, silver, copper);
			npcCorpse->removeCurrency();

			// Add currency to looter.
			EXPECTED(pLooter->getInventory()->addCurrency(CurrencySlot::Personal, platinum, gold, silver, copper));
		}

		pLooter->getConnection()->sendLootResponse(LootResponse::LOOT, platinum, gold, silver, copper);

		if (currencyLooted) {
			// TODO: Currency save.
		}

		// Send items.
		npcCorpse->onLootBegin();
		int count = 0;
		for (auto i : npcCorpse->getLootItems()) {
			i->setSlot(23 + count);
			uint32 payloadSize = 0;
			const unsigned char* data = i->copyData(payloadSize, Payload::ItemPacketLoot);

			auto outPacket = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
			pLooter->getConnection()->sendPacket(outPacket);
			safe_delete(outPacket);
			count++;
		}

		return;
	}

	// Handle: Looting a Character corpse.
	if (actor->isCharacterCorpse()) {

		return;
	}
}

void Zone::handleEndLootRequest(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->isLooting());
	Actor* corpse = pCharacter->getLootingCorpse();
	EXPECTED(corpse);
	EXPECTED(corpse->getLooter() == pCharacter);

	corpse->setLooter(nullptr);
	pCharacter->setLootingCorpse(nullptr);
	pCharacter->getConnection()->sendLootComplete();

	// Finished looting an NPC corpse.
	if (corpse->isNPCCorpse()) {
		NPC* npcCorpse = Actor::cast<NPC*>(corpse);

		// Check: Empty corpse
		if (!npcCorpse->hasCurrency() && !npcCorpse->hasItems()) {
			npcCorpse->destroy();
			return;
		}
	}
	// Finished looting a Character corpse.
	else if (corpse->isCharacterCorpse()) {
		// TODO:
	}
}

void Zone::handleLootItem(Character* pCharacter, Actor* pCorpse, const uint32 pSlotID) {
	EXPECTED(pCharacter);
	EXPECTED(pCorpse);
	EXPECTED(pCharacter->isLooting());
	EXPECTED(pCharacter->getLootingCorpse() == pCorpse);
	EXPECTED(pCorpse->getLooter() == pCharacter);
	//EXPECTED(Limits::Corpse::slotIsValid(pSlotID)); // TODO!

	// Looting Item from NPC corpse.
	if (pCorpse->isNPCCorpse()) {
		NPC* npcCorpse = Actor::cast<NPC*>(pCorpse);
		Item* item = npcCorpse->getLootItem(pSlotID - 23);

		// Update Character Inventory.
		pCharacter->getInventory()->pushCursor(item);

		// Update NPC loot items.
		npcCorpse->removeLootItem(pSlotID - 23);

		uint32 payloadSize = 0;
		const unsigned char* data = item->copyData(payloadSize, Payload::ItemPacketTrade);

		auto outPacket = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
		pCharacter->getConnection()->sendPacket(outPacket);
		safe_delete(outPacket);
	}
	
}

void Zone::handleConsider(Character* pCharacter, const uint32 pSpawnID) {
	EXPECTED(pCharacter);

	if (pSpawnID == pCharacter->getSpawnID()){
		// TODO: Client sends self consider, need to double check how this should be handled.
		return;
	}
	
	Actor* actor = pCharacter->findVisible(pSpawnID);
	EXPECTED(actor);

	pCharacter->getConnection()->sendConsiderResponse(pSpawnID, 1); // TODO: Message
}

void Zone::handleConsiderCorpse(Character* pCharacter, const uint32 pSpawnID) {
	EXPECTED(pCharacter);

	Actor* actor = pCharacter->findVisible(pSpawnID);
	EXPECTED(actor);
	EXPECTED(actor->isCorpse());

	if (actor->isNPCCorpse()) {
		auto decayTimer = actor->getDecayTimer();
		auto minutes = decayTimer.minutesRemainingX();
		auto seconds = decayTimer.secondsRemainingX();
		pCharacter->getConnection()->sendSimpleMessage(MessageType::Aqua, StringID::CORPSE_DECAY1, std::to_string(minutes), std::to_string(seconds));
	}

	
	/*
	491 This corpse's resurrection time will expire in %1 hour(s) %2 minute(s) %3 seconds.
	492 This corpse's resurrection time will expire in %1 minute(s) %2 seconds.
	493 This corpse will decay in %1 day(s) %2 hour(s) %3 minute(s) %4 seconds.
	494 This corpse will decay in %1 hour(s) %2 minute(s) %3 seconds.
	495 This corpse will decay in %1 minute(s) %2 seconds.
	*/
}

const bool Zone::checkAuthentication(Character* pCharacter) {
	EXPECTED_BOOL(pCharacter);
	return pCharacter->checkZoneAuthentication(mID, mInstanceID);
}

ZonePoint* Zone::_getClosestZonePoint(const Vector3& pPosition) {
	ZonePoint* result = nullptr;
	float closestDistance = 9999999.0f;

	for (auto i : mZonePoints) {
		float d = pPosition.squareDistance(i->mPosition);
		if (d < closestDistance) {
			closestDistance = d;
			result = i;
		}
	}

	return result;
}

const bool Zone::canBank(Character* pCharacter) {
	EXPECTED_BOOL(pCharacter);
	static const float MaxBankingDistance = 5.0f; // TODO! Work out the max distance.

	float closestDistance = 9999999.0f;
	for (auto i : pCharacter->getVisibleNPCs()) {
		if (i->isBanker()) {
			float d = pCharacter->squareDistanceTo(i);
			if (d < closestDistance)
				closestDistance = d;
		}
	}

	return closestDistance < MaxBankingDistance;
}

const bool Zone::canShop(Character* pCharacter, NPC* pMerchant) {
	EXPECTED_BOOL(pCharacter);
	EXPECTED_BOOL(pMerchant);

	static const float MaxShoppingDistance = 405.0f; // This is fairly close. Have seen 401.

	return pCharacter->squareDistanceTo(pMerchant) <= MaxShoppingDistance;
}

void Zone::handleDamage(Actor* pAttacker, Actor* pDefender, const int32 pAmount, const uint8 pType, const uint16 pSpellID) {
	using namespace Payload::Zone;

	uint32 sequence = Random::make(0, 20304843);
	auto packet = Damage::construct(pDefender->getSpawnID(), pAttacker->getSpawnID(), pAmount, pType, sequence, pSpellID);
	sendToVisible(pDefender, packet);
	safe_delete(packet);
}

void Zone::handleCriticalHit(Character* pCharacter, int32 pDamage) {
	pCharacter->getConnection()->sendSimpleMessage(MessageType::CritMelee, StringID::CRITICAL_HIT, pCharacter->getName(), std::to_string(pDamage));
}

void Zone::handleHPChange(Actor* pActor) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = ActorHPUpdate::construct(pActor->getSpawnID(), pActor->getHPPercent());
	sendToVisible(pActor, packet);
	safe_delete(packet);
}

void Zone::handleTradeRequest(Character* pCharacter, const uint32 pToSpawnID) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getSpawnID() != pToSpawnID);
	EXPECTED(pCharacter->isTrading() == false); // Check: Character is not already trading.

	auto actor = findActor(pToSpawnID);
	EXPECTED(actor);

	// TODO: Check distance for trading.

	// Requesting trade with an NPC.
	if (actor->isNPC()) {
		auto npc = Actor::cast<NPC*>(actor);
		// Check: Is this NPC accepting trading requests?
		if (npc->willTrade()) {
			// Notify: NPC will accept trading.
			pCharacter->getConnection()->sendTradeRequestAcknowledge(npc->getSpawnID());
			pCharacter->setTradingWith(npc);
			return;
		}
		else {
			pCharacter->message(MessageType::White, "This NPC will not trade with you.");
			return;
		}
	}
	// Requesting trade with a Character.
	else if (actor->isCharacter()) {
		auto character = Actor::cast<Character*>(actor);
		character->getConnection()->sendTradeRequest(pCharacter->getSpawnID());
		return;
	}
}

void Zone::handleTradeAccept(Character* pCharacter, const uint32 pSpawnID) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getSpawnID() != pSpawnID);
	EXPECTED(pCharacter->isTrading()); // Check: Character is trading.
	EXPECTED(pCharacter->getTradingWith()->getSpawnID() == pSpawnID); // Sanity.

	pCharacter->getConnection()->sendTradeFinished();
	//pCharacter->getConnection()->sendCancelTrade(pSpawnID);
	//pCharacter->getConnection()->sendFinishWindow();
	//pCharacter->getConnection()->sendFinishWindow2();
	pCharacter->setTradingWith(nullptr);

	const int64 tradeCurrency = pCharacter->getInventory()->getTotalTradeCurrency();
	pCharacter->getInventory()->onTradeAccept();
}

void Zone::handleTradeCancel(Character* pCharacter, const uint32 pSpawnID) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getSpawnID() != pSpawnID);
	EXPECTED(pCharacter->isTrading()); // Check: Character is trading.
	EXPECTED(pCharacter->getTradingWith()->getSpawnID() == pSpawnID); // Sanity.

	// Make a list of Items that were in the trade window.
	std::list<Item*> unordered;
	pCharacter->getInventory()->getTradeItems(unordered);
	
	// Adjust order of Items so that containers are sent first.
	std::list<Item*> ordered;
	for (auto i = unordered.begin(); i != unordered.end();) {
		if ((*i)->isContainer()) {
			ordered.push_back(*i);
			i = unordered.erase(i);
			continue;
		}
		i++;
	}
	// Add remaining Items.
	for (auto i : unordered)
		ordered.push_back(i);

	// Clear all trade Items.
	EXPECTED(pCharacter->getInventory()->clearTradeItems());

	// Place Items back into Character Inventory.
	for (auto i : ordered) {
		if (i->isStackable()) {
			// When an Item is being return to the Character, try stacking first.
			auto existing = pCharacter->getInventory()->findStackable(i->getID());
			if (existing) {
				existing->addStacks(1);
				// This currently only works for primary slot stacking.. when I try to send into a container I get a broken item in the charm slot.
				pCharacter->getConnection()->sendItemTrade(existing);
				delete i;
			}
			// If there is no existing Item to stack on to, find a free slot.
			else {
				const uint32 slotID = pCharacter->getInventory()->findEmptySlot(i);
				pCharacter->getInventory()->put(i, slotID);
				pCharacter->getConnection()->sendItemTrade(i);
			}
		}
		else {
			const uint32 slotID = pCharacter->getInventory()->findEmptySlot(i);

			// No slot was found, try cursor.
			if (SlotID::isNone(slotID)) {
				//if (pCharacter->getInventory()->)
			}
			else {
				pCharacter->getInventory()->put(i, slotID);
				
				// Update container contents slots
				if (i->isContainer()) {
					// Check: Containers can only be moved into main slots.
					EXPECTED(SlotID::isMainInventory(slotID));
					i->updateContentsSlots();
				}
					

				pCharacter->getConnection()->sendItemTrade(i);
			}
		}
	}

	// Update Inventory.
	EXPECTED(pCharacter->getInventory()->onTradeCancel());

	pCharacter->getConnection()->sendTradeCancel(pSpawnID);
	pCharacter->getConnection()->sendFinishWindow();
	pCharacter->getConnection()->sendFinishWindow2();
	pCharacter->setTradingWith(nullptr);

	// Update client.
	pCharacter->getConnection()->sendCurrencyUpdate();

	
}

void Zone::handleShopRequest(Character* pCharacter, const uint32 pSpawnID) {
	EXPECTED(pCharacter);

	// Check: Character is in a state that allows for shopping.
	EXPECTED(pCharacter->canShop());

	// Find Actor by pSpawnID.
	auto actor = findActor(pSpawnID);
	EXPECTED(actor);
	EXPECTED(actor->isNPC());

	auto npc = Actor::cast<NPC*>(actor);

	// Check: NPC is a merchant.
	EXPECTED(npc->isMerchant());

	// Check: Distance to merchant.
	EXPECTED(canShop(pCharacter, npc));

	// Check: Character has Items on cursor.
	// NOTE: UF does not prevent this, this is just to keep things more simple.
	if (pCharacter->getInventory()->isCursorEmpty() == false) {
		pCharacter->notify("Please clear your cursor and try again.");
		pCharacter->getConnection()->sendShopRequestReply(pSpawnID, 0);
		return;
	}

	// Merchant is open for business.
	if (npc->isShopOpen()) {
		// Associate Character and NPC.
		pCharacter->setShoppingWith(npc);
		npc->addShopper(pCharacter);

		pCharacter->getConnection()->sendShopRequestReply(pSpawnID, 1, npc->_getSellRate());

		// Send shop Items.
		std::list<Item*> shopItems = npc->getShopItems();
		for (auto i : shopItems) {
			pCharacter->getConnection()->sendItemShop(i);
		}
	}
	// Merchant is busy.
	else {
		pCharacter->getConnection()->sendSimpleMessage(MessageType::Yellow, StringID::MERCHANT_BUSY);
		pCharacter->getConnection()->sendShopRequestReply(pSpawnID, 0);
	}
}

void Zone::handleShopEnd(Character* pCharacter, const uint32 pSpawnID) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->isShopping());

	NPC* npc = pCharacter->getShoppingWith();
	EXPECTED(npc->getSpawnID() == pSpawnID); // Sanity.

	// Disassociate Character and NPC.
	EXPECTED(npc->removeShopper(pCharacter));
	pCharacter->setShoppingWith(nullptr);
	
	pCharacter->getConnection()->sendShopEndReply();
}

void Zone::handleShopSell(Character* pCharacter, const uint32 pSpawnID, const uint32 pSlotID, const uint32 pStacks) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->isShopping());

	NPC* npc = pCharacter->getShoppingWith();
	EXPECTED(npc->getSpawnID() == pSpawnID); // Sanity.

	// Check: Distance to merchant.
	EXPECTED(canShop(pCharacter, npc));

	// Check: Items can be sold from pSlotID
	EXPECTED(SlotID::isValidShopSellSlot(pSlotID));

	auto item = pCharacter->getInventory()->getItem(pSlotID);
	EXPECTED(item);

	// Check: Item can be sold.
	EXPECTED(item->isSellable());

	// Check: Item has enough stacks.
	EXPECTED(item->getStacks() >= pStacks);

	// Calculate sale price.
	const uint32 price = item->getSellPrice(pStacks, npc->getSellRate());

	const int32 copper = price % 10;
	const int32 silver = (price % 100) / 10;
	const int32 gold = (price % 1000) / 100;
	const int32 platinum = (price / 1000);
	
	// Add currency to Character.
	EXPECTED(pCharacter->getInventory()->addCurrency(platinum, gold, silver, copper));

	// Detect when the Character's auto food / drink is being sold.
	const bool updateConsumables = pCharacter->getInventory()->isAutoFood(item) || pCharacter->getInventory()->isAutoDrink(item);
	
	// Consume Item/stacks sold.
	EXPECTED(pCharacter->getInventory()->consume(pSlotID, pStacks));
	item = nullptr;

	// Update consumables.
	if (updateConsumables)
		pCharacter->getInventory()->updateConsumables();

	// Item being sold from a worn slot.
	if (SlotID::isWorn(pSlotID)) {

	}

	pCharacter->getConnection()->sendShopSellReply(pSpawnID, pSlotID, pStacks, price);

	// TODO: Dynamic merchant Items.
}

void Zone::handleShopBuy(Character* pCharacter, const uint32 pSpawnID, const uint32 pItemInstanceID, const uint32 pStacks) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->isShopping());

	// Check: Cursor is empty. Underfoot checks this.
	EXPECTED(pCharacter->getInventory()->isCursorEmpty());

	NPC* npc = pCharacter->getShoppingWith();
	EXPECTED(npc->getSpawnID() == pSpawnID); // Sanity.

	// Check: Distance to merchant.
	EXPECTED(canShop(pCharacter, npc));

	// Find Item.
	auto item = npc->getShopItem(pItemInstanceID);
	if (!item) {
		pCharacter->notify("I seem to have misplaced that. Sorry!");
		// Send failure reply to prevent UI locking up.
		pCharacter->getConnection()->sendShopBuyReply(pSpawnID, pItemInstanceID, pStacks, 0, -1);
		return;
	}

	//uint64 price = 0;
	const bool success = _handleShopBuy(pCharacter, npc, item, pStacks);

	if (success) {
		// Calculate cost.
		const uint32 price = item->getShopPrice() * pStacks;
		// Convert currency from single number to EQ currency.
		int32 platinum = 0, gold = 0, silver = 0, copper = 0;
		Utility::convertFromCopper(price, platinum, gold, silver, copper);
		// Remove currency from Character.
		EXPECTED(pCharacter->getInventory()->removeCurrency(platinum, gold, silver, copper));

		// Update client.
		pCharacter->getConnection()->sendCurrencyUpdate();

		pCharacter->getConnection()->sendShopBuyReply(pSpawnID, pItemInstanceID, pStacks, price);
	}
	else {
		pCharacter->getConnection()->sendShopBuyReply(pSpawnID, pItemInstanceID, pStacks, 0, -1);
	}
		
}

const bool Zone::_handleShopBuy(Character* pCharacter, NPC* pNPC, Item* pItem, const uint32 pStacks) {
	EXPECTED_BOOL(pCharacter);
	EXPECTED_BOOL(pNPC);
	EXPECTED_BOOL(pItem);

	// Unlimited Quantity.
	if (pItem->getShopQuantity() == -1) {
		// Non-stackable
		if (pItem->isStackable() == false) {
			EXPECTED_BOOL(pStacks == 1);

			// Try to find an empty slot for the Item.
			const uint32 slotID = pCharacter->getInventory()->findEmptySlot(pItem->isContainer(), pItem->getSize());

			// No empty slot found.
			if (SlotID::isNone(slotID)) {
				// NOTE: UF still sends the packet when it knows the Inventory is full. Go figure.
				// X tells you, 'Your inventory appears full! How can you buy more?'
				return false;
			}
			
			// Make a copy of the shop Item.
			Item* purchasedItem = ItemFactory::copy(pItem);

			// Put Item into Inventory.
			EXPECTED_BOOL(pCharacter->getInventory()->put(purchasedItem, slotID));

			// Update client.
			pCharacter->getConnection()->sendItemTrade(purchasedItem);

			// Update currency.
			//purchasedItem->getB

			return true;
		}
		// Stackable
		else {

			// Try to find an existing stack.
			//pCharacter->getInventory()->findStackable(pItem->getID());
		}
	}
	// Limited Quantity.
	else {
		// TODO: Dynamic shop Items.
		// ItemPacketMerchant
	}

	return false;

	//return true;

	//// Check: Shop has enough stacks.
	//if (pStacks > 1) {
	//	EXPECTED_BOOL(pItem->isStackable());

	//	// Limited quantity.
	//	if (pItem->getShopQuantity() != -1)
	//		EXPECTED_BOOL(pItem->getShopQuantity() >= pStacks);
	//}

	//Item* purchasedItem = nullptr;

	//// Unlimited quantity.
	//if (pItem->getShopQuantity() == -1) {
	//	purchasedItem = ItemFactory::copy(pItem);

	//	if (pItem->isStackable())
	//		purchasedItem->setStacks(pStacks);
	//}
	//else {

	//}

	//if (purchasedItem->isStackable()) {
	//	purchasedItem->setStacks(pStacks);
	//}
	//else {
	//	const uint32 slotID = pCharacter->getInventory()->findSlot(purchasedItem);

	//	// No slot was found
	//	if (SlotID::isNone(slotID)) {
	//		// Push to cursor.
	//		EXPECTED_BOOL(pCharacter->getInventory()->pushCursor(purchasedItem));
	//	}
	//	else {
	//		// Put in free slot.
	//		EXPECTED_BOOL(pCharacter->getInventory()->put(purchasedItem, slotID));
	//	}

	//	pCharacter->getConnection()->sendItemTrade(purchasedItem);
	//}

	//return true;
}

void Zone::handleNimbusAdded(Actor* pActor, const uint32 pNimbusID) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = AddNimbus::create();
	auto payload = AddNimbus::convert(packet);

	payload->mNimbusID = pNimbusID;
	payload->mSpawnID = pActor->getSpawnID();
	payload->mSpawnID2 = pActor->getSpawnID();

	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), packet, true);
	}
	else {
		sendToVisible(pActor, packet);
	}
	
	delete packet;
}

void Zone::handleNimbusRemoved(Actor* pActor, const uint32 pNimbusID) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = RemoveNimbus::construct(pActor->getSpawnID(), pNimbusID);

	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), packet, true);
	}
	else {
		sendToVisible(pActor, packet);
	}

	delete packet;
}

void Zone::handleRandomRequest(Character* pCharacter, const uint32 pLow, const uint32 pHigh) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	u32 low = pLow;
	u32 high = pHigh;

	if (low == 0 && high == 0) { high = 100; }

	EXPECTED(low <= high);

	const uint32 result = Random::make(low, high);
	auto packet = RandomReply::construct(pCharacter->getName(), low, high, result);
	sendToVisible(pCharacter, packet, true);
	delete packet;
}
