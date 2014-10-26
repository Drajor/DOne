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
#include "NPCFactory.h"
#include "Random.h"
#include "LootAllocator.h"
#include "Item.h"
#include "Inventory.h"

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

	mSpawnPoints.remove_if(Utility::containerEntryDelete<SpawnPoint*>);
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
	EXPECTED_BOOL(loadSpawnPoints());
	EXPECTED_BOOL(populate());

	mInitialised = true;
	return true;
}

const bool Zone::loadZonePoints() {
	std::list<ZonePointData*>* zonePointData = nullptr;
	EXPECTED_BOOL(ZoneDataManager::getInstance().getZonePoints(getID(), &zonePointData));
	EXPECTED_BOOL(zonePointData);

	// Create ZonePoints.
	for (auto i : *zonePointData) {
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

const bool Zone::loadSpawnPoints() {
	std::list<SpawnPointData*>* spawnPointData = nullptr;
	EXPECTED_BOOL(ZoneDataManager::getInstance().getSpawnPoints(getID(), &spawnPointData));
	EXPECTED_BOOL(spawnPointData);

	// Create SpawnPoints.
	for (auto i : *spawnPointData) {
		auto spawnPoint = new SpawnPoint();
		mSpawnPoints.push_back(spawnPoint);

		spawnPoint->setPosition(i->mPosition);
		spawnPoint->setHeading(i->mHeading);
		spawnPoint->setRespawnTime(i->mRespawnTime);
		spawnPoint->setType(i->mType);
		spawnPoint->setNPCType(i->mNPCType);
		spawnPoint->setSpawnGroup(i->mSpawnGroupID);
	}

	return true;
}

const bool Zone::populate() {
	EXPECTED_BOOL(mPopulated == false);

	for (auto i : mSpawnPoints)
		_populate(i);

	mPopulated = true;
	return true;
}

const bool Zone::depopulate() {
	EXPECTED_BOOL(mPopulated);

	for (auto i : mSpawnPoints) {
		auto npc = i->getNPC();
		if (npc) {
			// Unlink NPC and SpawnPoint
			i->setNPC(nullptr);
			npc->setSpawnPoint(nullptr);

			// Flag for destruction next update.
			npc->destroy();
		}
	}

	mPopulated = false;
	return true;
}

//void Zone::addAuthentication(ClientAuthentication& pAuthentication, String pCharacterName) {
//	mAuthenticatedCharacters.insert(std::make_pair(pCharacterName, pAuthentication));
//}
//
//void Zone::removeAuthentication(String pCharacterName) {
//	mAuthenticatedCharacters.erase(pCharacterName);
//}
//
//bool Zone::checkAuthentication(String pCharacterName) {
//	for (auto i : mAuthenticatedCharacters) {
//		if (i.first == pCharacterName) {
//			Log::info("[Zone] Authentication Passed");
//			return true;
//		}
//	}
//
//	Log::error("[Zone] checkAuthentication is returning false. This is unexpected.");
//	return false;
//}
//
//bool Zone::getAuthentication(String pCharacterName, ClientAuthentication& pAuthentication) {
//	for (auto i : mAuthenticatedCharacters) {
//		if (i.first == pCharacterName) {
//			pAuthentication = i.second;
//			return true;
//		}
//	}
//
//	return false;
//}


void Zone::update() {
	// Check if any new clients are connecting to this Zone.
	_handleIncomingConnections();

	_updatePreConnections();
	_updateConnections();
	_updateCharacters();
	_updateNPCs();
	_updateSpawnPoints();

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
					notifyCharacterZoneIn(character);
					// Let Character do what it needs to.
					character->onZoneIn();
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

void Zone::notifyCharacterZoneIn(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(mScene);

	// Add Character to Scene.
	mScene->add(pCharacter);
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
	_sendChat(pCharacter, ChannelID::CH_SAY, pMessage);
}

void Zone::handleShout(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_SHOUT, pMessage);
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
}

void Zone::_onCamp(Character* pCharacter) {
	EXPECTED(pCharacter);

	if (pCharacter->hasGuild())
		GuildManager::getInstance().onCamp(pCharacter);

	if (pCharacter->hasGroup())
		GroupManager::getInstance().onCamp(pCharacter);

	if (pCharacter->hasRaid())
		RaidManager::getInstance().onCamp(pCharacter);
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
}

void Zone::handleTarget(Character* pCharacter, uint16 pSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	// Character is clearing their target.
	if (pSpawnID == NO_TARGET) {
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
}

Actor* Zone::findActor(const SpawnID pSpawnID) {
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

	//Log::info(pCharacter->getName() + " can now see " + pAddActor->getName());

	pAddActor->_syncPosition();

	uint32 size = pAddActor->getDataSize();
	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);
	EXPECTED(pAddActor->copyData(ds));

	auto outPacket = new EQApplicationPacket(OP_ZoneEntry, data, size);
	pCharacter->getConnection()->sendPacket(outPacket);
	safe_delete(outPacket);
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

	//if (pActor->isNPC()) {
	//	mNPCs.remove(Actor::cast<NPC*>(pActor));
	//}
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
		SpawnPoint* spawnPoint = npc->getSpawnPoint();
		if (spawnPoint) {
			spawnPoint->setNPC(nullptr);
			npc->setSpawnPoint(nullptr);
			// Add SpawnPoint to the respawn list.
			_addRespawn(spawnPoint);
		}

		// Check: Empty corpse.
		if (!npc->hasCurrency() && !npc->hasItems()) {
			pActor->destroy();
			return;
		}
	}
	// Character Dead.
	else {

	}

	pActor->onDeath();

	//Death_Struct* d = (Death_Struct*)app->pBuffer;
	//d->spawn_id = getID();
	//d->killer_id = killerMob ? killerMob->getID() : 0;
	//d->bindzoneid = 0;
	//d->spell_id = spell == SPELL_UNKNOWN ? 0xffffffff : spell;
	//d->attack_skill = SkillDamageTypes[attack_skill];
	//d->damage = damage;
	//app->priority = 6;
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
			EXPECTED(pLooter->addCurrency(MoneySlotID::PERSONAL, platinum, gold, silver, copper));
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


void Zone::_addRespawn(SpawnPoint* pSpawnPoint) {
	EXPECTED(pSpawnPoint);
	EXPECTED(pSpawnPoint->getNPC() == nullptr);

	pSpawnPoint->start();
	mRespawns.push_back(pSpawnPoint);
}

void Zone::_updateSpawnPoints() {
	for (auto i = mRespawns.begin(); i != mRespawns.end();) {
		SpawnPoint* spawnPoint = *i;
		if (spawnPoint->update()) {
			i++;
			continue;
		}
		i = mRespawns.erase(i);
		_populate(spawnPoint);
	}
}

void Zone::_populate(SpawnPoint* pSpawnPoint) {
	EXPECTED(pSpawnPoint);
	EXPECTED(pSpawnPoint->getNPC() == nullptr);

	auto npc = NPCFactory::getInstance().create(1);
	npc->setZone(this);
	npc->initialise();
	npc->setPosition(pSpawnPoint->getPosition());
	npc->setHeading(pSpawnPoint->getHeading());

	// Link NPC / SpawnPoint
	pSpawnPoint->setNPC(npc);
	npc->setSpawnPoint(pSpawnPoint);

	// Allocate Loot
	mLootAllocator->allocate(npc);

	addActor(npc);
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

	pCharacter->getConnection()->sendSimpleMessage(MessageType::Aqua, StringID::CORPSE_DECAY1, "1", "1");
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