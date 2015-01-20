#include "Zone.h"
#include "ZoneConnectionManager.h"
#include "ServiceLocator.h"
#include "World.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "GroupManager.h"
#include "GuildManager.h"
#include "Guild.h"
#include "RaidManager.h"
#include "Character.h"
#include "NPC.h"
#include "ZoneConnection.h"
#include "Constants.h"
#include "IDataStore.h"
#include "Transmutation.h"
#include "Data.h"
#include "SpellDataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "../common/types.h"

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
#include "Object.h"
#include "LootController.h"
#include "RespawnOptions.h"
#include "ExperienceController.h"
#include "ExperienceCalculator.h"
#include "HateController.h"
#include "CommandHandler.h"

Zone::Zone(const u16 pPort, const u16 pZoneID, const u16 pInstanceID) :
	mPort(pPort),
	mID(pZoneID),
	mInstanceID(pInstanceID)
{
}

Zone::~Zone() {

	safe_delete(mScene);
	safe_delete(mLootAllocator);
	safe_delete(mSpawnPointManager);
	safe_delete(mZoneConnectionManager);
	safe_delete(mTransmutation);

	for (auto i : mObjects) delete i;
	mObjects.clear();

	//for (auto i : mDoors) delete i;
	//mDoors.clear();

	for (auto i : mZonePoints) delete i;
	mZonePoints.clear();

	safe_delete(mLog);
}

const bool Zone::initialise(ZoneManager* pZoneManager, ILogFactory* pLogFactory, Data::Zone* pZoneData, Experience::Calculator* pExperienceCalculator, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory, NPCFactory* pNPCFactory) {
	if (mInitialised) return false;
	if (!pZoneManager) return false;
	if (!pLogFactory) return false;
	if (!pZoneData) return false;
	if (!pExperienceCalculator) return false;
	if (!pGroupManager) return false;
	if (!pRaidManager) return false;
	if (!pGuildManager) return false;
	if (!pCommandHandler) return false;
	if (!pItemFactory) return false;
	if (!pNPCFactory) return false;

	mZoneManager = pZoneManager;
	mLogFactory = pLogFactory;
	mExperienceCalculator = pExperienceCalculator;
	mGroupManager = pGroupManager;
	mRaidManager = pRaidManager;
	mGuildManager = pGuildManager;
	mCommandHandler = pCommandHandler;
	mItemFactory = pItemFactory;
	mNPCFactory = pNPCFactory;

	// Create and configure Zone log.
	mLog = mLogFactory->make();
	StringStream ss;
	ss << "[Zone (ID: " << getID() << " Instance: " << getInstanceID() << ")]";
	mLog->setContext(ss.str());
	mLog->status("Initialising");

	// Create ZoneConnectionManager.
	mZoneConnectionManager = new ZoneConnectionManager();
	if (!mZoneConnectionManager->initialise(mPort, this, mLogFactory, mZoneManager, mGroupManager, mRaidManager, mGuildManager)) {
		mLog->error("ZoneConnectionManager failed to initialise.");
		return false;
	}

	// Create Scene.
	mScene = new Scene();
	if (!mScene->initialise(this, mLogFactory->make())) {
		mLog->error("Scene failed to initialise.");
		return false;
	}

	// Create SpawnPointManager.
	mSpawnPointManager = new SpawnPointManager();
	if (!mSpawnPointManager->initialise(this, mNPCFactory, mLogFactory->make(), pZoneData->mSpawnGroups, pZoneData->mSpawnPoints)) {
		mLog->error("SpawnPointManager failed to initialise.");
		return false;
	}

	mLootAllocator = new LootAllocator();
	mTransmutation = new Transmutation();
	mExperienceModifer = new Experience::Modifier();

	mZoneType = pZoneData->mZoneType;
	mTimeType = pZoneData->mTimeType;
	mSkyType = pZoneData->mSkyType;
	mFogDensity = pZoneData->mFogDensity;

	mLongName = pZoneData->mLongName;
	mShortName = pZoneData->mShortName;
	mLongNameStringID = pZoneData->mLongNameStringID;
	mSafePoint = pZoneData->mSafePosition;

	EXPECTED_BOOL(loadZonePoints(pZoneData->mZonePoints));

	// Objects.
	EXPECTED_BOOL(loadObjects(pZoneData->mObjects));

	// Doors.
	EXPECTED_BOOL(loadDoors(pZoneData->mDoors));

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

const bool Zone::canShutdown() const {
	// Can not shut down while there are still active connections to the Zone.
	return mZoneConnectionManager->numConnections() == 0;
}

const bool Zone::shutdown() {
	EXPECTED_BOOL(mShuttingDown == false);

	// TODO: Save any temp data.
	
	mShuttingDown = true;

	return true;
}

const bool Zone::loadZonePoints(Data::ZonePointList pZonePoints) {
	EXPECTED_BOOL(mZonePoints.empty());

	// Create ZonePoints.
	for (auto i : pZonePoints) {
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
	return mSpawnPointManager->populate();
}

const bool Zone::depopulate() {
	return mSpawnPointManager->depopulate();
}

const bool Zone::update() {
	if (isShuttingDown()) { return false; }

	mZoneConnectionManager->update();
	_updateCharacters();
	_updateNPCs();

	mSpawnPointManager->update();

	return true;
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

void Zone::moveCharacter(Character* pCharacter, float pX, float pY, float pZ) {
	pCharacter->setPosition(Vector3(pX, pY, pZ));
	handleActorPositionChange(pCharacter);

	// This is not ideal. Need to decide whether handleActorPositionChange should check for Character/have a parameter to send to self.
	pCharacter->getConnection()->sendPosition();
}

void Zone::onEnterZone(Character* pCharacter) {
	if (!pCharacter) return;

	mLog->info("Character (" + pCharacter->getName() + ") entering zone.");
	pCharacter->onEnterZone();

	// Add Character.
	mScene->add(pCharacter);
	mCharacters.push_back(pCharacter);
	mActors.push_back(pCharacter);

	if (pCharacter->hasGroup())
		mGroupManager->onEnterZone(pCharacter);

	if (pCharacter->hasRaid())
		mRaidManager->onEnterZone(pCharacter);

	if (pCharacter->hasGuild())
		mGuildManager->onEnterZone(pCharacter);

	// Notify ZoneManager.
	mZoneManager->onEnterZone(pCharacter);
}

void Zone::onLeaveZone(Character* pCharacter) {
	if (!pCharacter) return;

	mLog->info("Character (" + pCharacter->getName() + ") leaving zone.");
	pCharacter->onLeaveZone();

	// Remove Character.
	mScene->remove(pCharacter);
	mCharacters.remove(pCharacter);
	mActors.remove(pCharacter);

	// Handle: Character leaving zone while looting.
	// (UNTESTED)
	if (pCharacter->isLooting()) {
		auto corpse = pCharacter->getLootingCorpse();
		// Clear Character reference from corpse.
		corpse->getLootController()->clearLooter();
		// Clear corpse reference from Character.
		pCharacter->setLootingCorpse(nullptr);
	}

	// Handle: Character leaving zone while shopping.
	// (UNTESTED)
	if (pCharacter->isShopping()) {
		auto npc = pCharacter->getShoppingWith();
		EXPECTED(npc);
		// Disassociate Character and NPC.
		EXPECTED(npc->removeShopper(pCharacter));
		pCharacter->setShoppingWith(nullptr);
	}

	// Targetters.

	// Trading.

	// Aggro.

	if (pCharacter->hasGuild())
		mGuildManager->onLeaveZone(pCharacter);

	if (pCharacter->hasGroup())
		mGroupManager->onLeaveZone(pCharacter);

	if (pCharacter->hasRaid())
		mRaidManager->onLeaveZone(pCharacter);

	// Notify ZoneManager.
	mZoneManager->onLeaveZone(pCharacter);
}

void Zone::onCampComplete(Character* pCharacter) {
	if (!pCharacter) return;

	mLog->info("Character (" + pCharacter->getName() + ") camped.");

	// Save
	pCharacter->_updateForSave();
	requestSave(pCharacter);

	// Clean up.
	mScene->remove(pCharacter);
	mCharacters.remove(pCharacter);
	mActors.remove(pCharacter);

	if (pCharacter->hasGuild())
		mGuildManager->onCamp(pCharacter);

	if (pCharacter->hasGroup())
		mGroupManager->onCamp(pCharacter);

	if (pCharacter->hasRaid())
		mRaidManager->onCamp(pCharacter);

	// Notify ZoneManager.
	mZoneManager->onLeaveWorld(pCharacter);
}

void Zone::onLinkdeadBegin(Character* pCharacter) {
	if (!pCharacter) return;

	mLog->info("Character (" + pCharacter->getName() + ") LD begin.");
	pCharacter->setLinkDead();
	handleLinkDead(pCharacter);

	// Handle: Character going LD while looting.
	// (UNTESTED)
	if (pCharacter->isLooting()) {
		Actor* corpse = pCharacter->getLootingCorpse();
		// Clear Character reference from corpse.
		corpse->getLootController()->clearLooter();
		// Clear corpse reference from Character.
		pCharacter->setLootingCorpse(nullptr);
	}

	if (pCharacter->hasGuild())
		mGuildManager->onLinkdead(pCharacter);

	if (pCharacter->hasGroup())
		mGroupManager->onLinkdead(pCharacter);

	if (pCharacter->hasRaid())
		mRaidManager->onLinkdead(pCharacter);
}


void Zone::onLinkdeadEnd(Character* pCharacter) {
	if (!pCharacter) return;

	mLog->info("Character (" + pCharacter->getName() + ") LD end.");
	
	// Save
	pCharacter->_updateForSave();
	requestSave(pCharacter);

	// Clean up.
	mScene->remove(pCharacter);
	mCharacters.remove(pCharacter);
	mActors.remove(pCharacter);

	// Notify ZoneManager.
	mZoneManager->onLeaveWorld(pCharacter);
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


void Zone::handleAFK(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::AFK, pCharacter->isAFK()); }
void Zone::handleShowHelm(Actor* pActor) { _sendSpawnAppearance(pActor, SpawnAppearanceTypeID::ShowHelm, pActor->getShowHelm()); }
void Zone::handleAnonymous(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::Anonymous, pCharacter->getAnonymous()); }
void Zone::handleStanding(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::Animation, SpawnAppearanceAnimation::Standing); }
void Zone::handleSitting(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::Animation, SpawnAppearanceAnimation::Sitting); }
void Zone::handleCrouching(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::Animation, SpawnAppearanceAnimation::Crouch); }
void Zone::notifyCharacterGM(Character* pCharacter){ _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::GM, pCharacter->isGM() ? 1 : 0, true); }
void Zone::handleLinkDead(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::LinkDead, 1, false); }

void Zone::_sendSpawnAppearance(Actor* pActor, const u16 pType, const uint32 pParameter, const bool pIncludeSender) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	SpawnAppearance payload;
	payload.mSpawnID = pActor->getSpawnID();
	payload.mType = pType;
	payload.mParameter = pParameter;

	auto packet = SpawnAppearance::create(payload);
	// Character
	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), packet, pIncludeSender);
	}
	// Actor
	else {
		sendToVisible(pActor, packet);
	}
	delete packet;
}

void Zone::handleChannelMessage(Character* pCharacter, const u32 pChannelID, const String& pSenderName, const String& pTargetName, const String& pMessage) {
	EXPECTED(pCharacter);

	switch (pChannelID) {
	case ChannelID::Guild:
		EXPECTED(pCharacter->hasGuild());
		mGuildManager->onMessage(pCharacter, pMessage);
		break;
	case ChannelID::Group:
		EXPECTED(pCharacter->hasGroup());
		mGroupManager->handleMessage(pCharacter, pMessage);
		break;
	case ChannelID::Shout:
		break;
	case ChannelID::Auction:
		break;
	case ChannelID::OOC:
		break;
	case ChannelID::Broadcast:
		break;
	case ChannelID::Tell:
		break;
	case ChannelID::Say:
		// Check whether user has entered a command.
		if (pMessage[0] == COMMAND_TOKEN) {
			mCommandHandler->command(pCharacter, pMessage);
			break;
		}
		break;
	case ChannelID::GMSay:
		break;
	case ChannelID::Raid:
		EXPECTED(pCharacter->hasRaid());
		mRaidManager->handleMessage(pCharacter, pMessage);
		break;
	case ChannelID::UCS:
		break;
	case ChannelID::Emote:
		handleEmote(pCharacter, pMessage);
		break;
	default:
		break;
	}
}

void Zone::handleSay(Character* pCharacter, const String pMessage) {
	EXPECTED(pCharacter);

	// Send to other Characters.
	_sendChat(pCharacter, ChannelID::Say, pMessage);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Say, pCharacter);
}

void Zone::handleShout(Character* pCharacter, const String pMessage) {
	EXPECTED(pCharacter);

	// Send to other Characters.
	_sendChat(pCharacter, ChannelID::Shout, pMessage);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Shout, pCharacter);
}

void Zone::handleAuction(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::Auction, pMessage);

	// TODO: Server Auction
}

void Zone::handleOOC(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::OOC, pMessage);
	
	// TODO: Server OOC
}

void Zone::handleEmote(Character* pCharacter, const String pMessage) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	String message = pCharacter->getName() + " " + pMessage;
	auto packet = Emote::construct(message);
	sendToVisible(pCharacter, packet, false);
	delete packet;
}

void Zone::_sendChat(Character* pCharacter, const u32 pChannel, const String pMessage) {
	EXPECTED(pCharacter);

	const auto sender = pCharacter->getConnection();
	auto packet = ZoneConnection::makeChannelMessage(pChannel, pCharacter->getName(), pMessage);

	for (auto i : mCharacters) {
		if (i != pCharacter)
			i->getConnection()->sendPacket(packet);
	}

	delete packet;
}

void Zone::handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage) {
	ServiceLocator::getZoneManager()->handleTell(pCharacter, pTargetName, pMessage);
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
	EXPECTED(pCharacter);

	// Notify other clients.
	_sendActorLevel(pCharacter);
	// Send particle effects.
	_sendLevelAppearance(pCharacter);

	// Notify GuildManager.
	if (pCharacter->hasGuild())
		mGuildManager->onLevelChange(pCharacter);
}

void Zone::handleLevelDecrease(Character* pCharacter) {
	EXPECTED(pCharacter);

	// Notify user client.
	_sendActorLevel(pCharacter);

	// Notify GuildManager.
	if (pCharacter->hasGuild())
		mGuildManager->onLevelChange(pCharacter);
}

void Zone::_sendLevelAppearance(Character* pCharacter) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	//auto outPacket = new EQApplicationPacket(OP_LevelAppearance, sizeof(LevelAppearance_Struct));
	//auto payload = reinterpret_cast<LevelAppearance_Struct*>(outPacket->pBuffer);
	//payload->parm1 = 0x4D;
	//payload->parm2 = payload->parm1 + 1;
	//payload->parm3 = payload->parm2 + 1;
	//payload->parm4 = payload->parm3 + 1;
	//payload->parm5 = payload->parm4 + 1;
	//payload->spawn_id = pCharacter->getSpawnID();
	//payload->value1a = 1;
	//payload->value2a = 2;
	//payload->value3a = 1;
	//payload->value3b = 1;
	//payload->value4a = 1;
	//payload->value4b = 1;
	//payload->value5a = 2;

	auto packet = LevelAppearance::create();
	auto payload = LevelAppearance::convert(packet);
	payload->mSpawnID = pCharacter->getSpawnID();
	payload->mParameter1 = 0x4D;

	sendToVisible(pCharacter, packet, true);
	delete packet;
}

void Zone::_sendActorLevel(Actor* pActor) {
	_sendSpawnAppearance(pActor, SpawnAppearanceTypeID::WhoLevel, pActor->getLevel());
}

void Zone::requestSave(Character*pCharacter) {
	if (!ServiceLocator::getDataStore()->saveCharacter(pCharacter->getName(), pCharacter->getData())) {
		pCharacter->getConnection()->sendMessage(MessageType::Red, "[ERROR] There was an error saving your character. I suggest you log out.");
		Log::error("[Zone] Failed to save character");
		return;
	}

	// Update the Account
	ServiceLocator::getAccountManager()->updateCharacter(pCharacter->getAccount(), pCharacter);
}

void Zone::getWhoMatches(std::list<Character*>& pResults, const WhoFilter& pFilter) {
	for (auto i : mCharacters) {
		if (i->isHidden()) continue;
		if (pFilter.checkRace() && i->getRace() != pFilter.mRace) continue;
		if (pFilter.checkClass() && i->getClass() != pFilter.mClass) continue;
		if (pFilter.checkMinimumLevel() && i->getLevel() < pFilter.mLevelMinimum) continue;
		if (pFilter.checkMaximumLevel() && i->getLevel() > pFilter.mLevelMaximum) continue;

		// Check: Flags
		if (pFilter.checkFlag()) {
			if (pFilter.mFlag == WhoFlag::LFG && !i->isLFG()) continue;
			if (pFilter.mFlag == WhoFlag::Trader && !i->isTrader()) continue;
			if (pFilter.mFlag == WhoFlag::Buyer && !i->isBuyer()) continue;

			// Check: Guild ID.
			if (pFilter.mFlag != i->getGuildID()) continue;
		}

		// Check: Text.
		if (pFilter.checkText()) {
			const bool characterName = Utility::findCI(i->getName(), pFilter.mText);
			const bool zoneName = Utility::findCI(getShortName(), pFilter.mText);
			if (!characterName && !zoneName) continue;
		}

		pResults.push_back(i);
	}
}

Character* Zone::findCharacter(const String pCharacterName) const {
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
	return ServiceLocator::getZoneManager()->findCharacter(pCharacterName, pIncludeZoning, this);
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

void Zone::onGuildsChanged() {
	auto packet = new EQApplicationPacket(OP_GuildsList);
	packet->size = Limits::Guild::MAX_NAME_LENGTH + (Limits::Guild::MAX_NAME_LENGTH * Limits::Guild::MAX_GUILDS); // TODO: Work out the minimum sized packet UF will accept.
	packet->pBuffer = reinterpret_cast<unsigned char*>(mGuildManager->_getGuildNames());
	
	for (auto i : mCharacters) {
		i->getConnection()->sendPacket(packet);
	}

	packet->pBuffer = nullptr;
	delete packet;
}

void Zone::onChangeGuild(Character* pCharacter) {
	if (!pCharacter) return;

	_sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::GuildID, pCharacter->getGuildID(), true);
	_sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::GuildRank, pCharacter->getGuildRank(), true);
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
	EXPECTED(pCharacter);
	_sendAppearanceUpdate(pCharacter);
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

	auto spellData = ServiceLocator::getSpellDataStore()->getData(pSpellID);
	EXPECTED(spellData);

	// Check: Pre-Conditions for casting
	if (pCharacter->preCastingChecks(spellData) == false) {

		//return; TODO: Once we have things sorted out more.
	}

	// Update Character state.
	EXPECTED(pCharacter->beginCasting(pSlot, pSpellID));

	// Update Character + those visible to
	auto packet = BeginCast::construct(pCharacter->getSpawnID(), pSpellID, 500);
	sendToVisible(pCharacter, packet, true);
	delete packet;
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
	if (pActor->isCharacter())
		sendToVisible(Actor::cast<Character*>(pActor), packet, true);
	else
		sendToVisible(pActor, packet);

	delete packet;

	// NPC Dead.
	if (pActor->isNPC())
		_handleDeath(pActor->cast<NPC*>(pActor), pKiller);
	else
		_handleDeath(pActor->cast<Character*>(pActor), pKiller);		
}

u32 getExperienceForKill(const u8 pKillerLevel, const u8 pNPCLevel, const float pZoneModifier, const float pNPCModifier) {
	return 3;
}

void Zone::_handleDeath(NPC* pNPC, Actor* pKiller) {
	EXPECTED(pNPC);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Dead, pNPC);

	// Check: Associated with a SpawnPoint.
	if (pNPC->hasSpawnPoint()) {
		// Notify SpawnPointManager
		mSpawnPointManager->onDeath(pNPC);
	}

	// Determine who gets credit for dying NPC.
	Character* killer = nullptr;
	auto controller = pNPC->getHateController();
	auto firstAttacker = controller->first();
	if (firstAttacker) {
		if (firstAttacker->isCharacter()) {
			killer = Actor::cast<Character*>(firstAttacker);
		}
		else if (firstAttacker->isPet()) {
			// TODO: Get owner.
		}
	}

	// Allocate experience / looting rights for kill.
	if (killer) {
		// Solo Character Kill.
		if (killer->isSolo()) {
			// Set looting rights.
			pNPC->getLootController()->set(killer);

			// Award experience.
			allocateSoloExperience(killer, pNPC);
		}
		// Raid Kill.
		else if (killer->hasRaid()) {
			// Set looting rights.
			pNPC->getLootController()->set(killer->getRaid());

			// Award raid experience.
			allocateRaidExperience(killer->getRaid(), pNPC);
		}
		// Group Kill
		else if (killer->hasGroup()) {
			// Set looting rights.
			pNPC->getLootController()->set(killer->getGroup());

			// Award group experience.
			allocateGroupExperience(killer->getGroup(), pNPC);
		}
	}

	// Check: Empty corpse.
	if (!pNPC->hasCurrency() && !pNPC->hasItems()) {
		pNPC->destroy();
		return;
	}

	pNPC->onDeath();
}

void Zone::_handleDeath(Character* pCharacter, Actor* pKiller) {
	EXPECTED(pCharacter);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Dead, pCharacter);

	pCharacter->getRespawnOptions()->setActive(true);
	pCharacter->getConnection()->sendRespawnWindow();

	pCharacter->onDeath();
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
		auto lootController = actor->getLootController();
		auto npcCorpse = Actor::cast<NPC*>(actor);
		// Check: Is pCharacter close enough to loot.
		if (pLooter->squareDistanceTo(npcCorpse) > 625) { // TODO: Magic.
			pLooter->getConnection()->sendLootResponse(LootResponse::TOO_FAR);
			return;
		}
		// Check: Is corpse currently closed?
		if (lootController->isOpen() == false) {
			// Check: Is it time to open the corpse?
			// TODO: This is probably the most efficient place to check this however it is odd!
			if (npcCorpse->getOpenTimer().check()) {
				// Open corpse.
				lootController->setOpen(true);
			}
		}
		// Check: Is Character allowed to loot this corpse?
		if (lootController->canLoot(pLooter) == false) {
			pLooter->getConnection()->sendLootResponse(LootResponse::DENY);
			return;
		}
		// Check: Is someone already looting this corpse?
		if (lootController->hasLooter()) {
			pLooter->getConnection()->sendLootResponse(LootResponse::ALREADY);
			return;
		}

		// Associate Character and Corpse.
		pLooter->setLootingCorpse(npcCorpse);
		lootController->setLooter(pLooter);

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
	auto lootController = corpse->getLootController();
	EXPECTED(lootController->getLooter() == pCharacter);

	// Disassociate Character and corpse.
	lootController->clearLooter();
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
	auto lootController = pCorpse->getLootController();
	EXPECTED(lootController->getLooter() == pCharacter);
	EXPECTED(SlotID::isCorpseSlot(pSlotID));

	// Looting Item from NPC corpse.
	if (pCorpse->isNPCCorpse()) {
		NPC* npcCorpse = Actor::cast<NPC*>(pCorpse);
		Item* item = npcCorpse->getLootItem(pSlotID - 23);

		// TODO: Cursor empty check.

		// Update Character Inventory.
		pCharacter->getInventory()->pushCursor(item);

		// Update NPC loot items.
		npcCorpse->removeLootItem(pSlotID - 23);

		pCharacter->getConnection()->sendItemTrade(item);
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
	if (!pCharacter) return false;
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

	if (pDefender->isCharacter())
		sendToVisible(Actor::cast<Character*>(pDefender), packet, true);
	else
		sendToVisible(pDefender, packet);

	delete packet;
}

void Zone::handleCriticalHit(Actor* pActor, const int32 pDamage) {
	EXPECTED(pActor);

	auto packet = ZoneConnection::makeSimpleMessage(MessageType::CritMelee, StringID::CRITICAL_HIT, pActor->getName(), std::to_string(pDamage));

	if (pActor->isCharacter())
		sendToVisible(Actor::cast<Character*>(pActor), packet, true);
	else
		sendToVisible(pActor, packet);

	delete packet;
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
			Item* purchasedItem = mItemFactory->copy(pItem);

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

void Zone::handleDropItem(Character* pCharacter) {
	EXPECTED(pCharacter);

	auto item = pCharacter->getInventory()->peekCursor();
	EXPECTED(item);
	EXPECTED(item->isTradeable());

	// Consume 
	EXPECTED(pCharacter->getInventory()->consume(SlotID::CURSOR, item->getStacks()));
}

const bool Zone::loadObjects(Data::ObjectList pObjects) {
	EXPECTED_BOOL(mObjects.empty());

	for (auto i : pObjects) {
		auto o = new Object();
		mObjects.push_back(o);

		o->setType(i->mType);
		o->setAsset(i->mAsset);
		o->setSize(i->mSize);
		o->setHeading(i->mHeading);
		o->setPosition(i->mPosition);
	}

	return true;
}

const bool Zone::loadDoors(Data::DoorList pDoors) {
	return true;
}

void Zone::_sendAppearanceUpdate(Actor* pActor) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = AppearanceUpdate::create();
	auto payload = AppearanceUpdate::convert(packet);

	payload->mSpawnID = pActor->getSpawnID();
	strcpy(payload->mActorName, pActor->getName().c_str());

	payload->mRace = pActor->getRace();
	payload->mGender = pActor->getGender();
	payload->mTexture = pActor->getTexture();
	//payload->mHelmTexture = pActor->getHelmTexture(); // Not sure how this works yet.
	payload->mFaceStyle = pActor->getFaceStyle();
	payload->mHairStyle = pActor->getHairStyle();
	payload->mHairColour = pActor->getHairColour();
	payload->mBeardStyle = pActor->getBeardStyle();
	payload->mBeardColour = pActor->getBeardColour();
	payload->mSize = pActor->getSize();
	payload->mDrakkinHeritage = pActor->getDrakkinHeritage();
	payload->mDrakkinTattoo = pActor->getDrakkinTattoo();
	payload->mDrakkinDetails = pActor->getDrakkinDetails();

	// Character
	if (pActor->isCharacter()) {
		sendToVisible(Actor::cast<Character*>(pActor), packet, true);
	}
	// Actor
	else {
		sendToVisible(pActor, packet);
	}
}

void Zone::handleAppearanceChange(Actor* pActor) {
	_sendAppearanceUpdate(pActor);
}

void Zone::handleRespawnSelection(Character* pCharacter, const u32 pSelection) {
	EXPECTED(pCharacter);
	auto respawnOptions = pCharacter->getRespawnOptions();
	EXPECTED(respawnOptions);
	EXPECTED(respawnOptions->isActive()); // Sanity.

	respawnOptions->setActive(false);

	// Find option.

	// TODO!
}

void Zone::handleWhoRequest(Character* pCharacter, WhoFilter& pFilter) {
	std::list<Character*> results;

	// /who all
	if (pFilter.mType == WhoType::All) {
		ServiceLocator::getZoneManager()->handleWhoRequest(pCharacter, pFilter, results);
	}
	// /who
	else if (pFilter.mType == WhoType::Zone) {
		getWhoMatches(results, pFilter);
	}

	pCharacter->getConnection()->sendWhoResponse(pFilter.mType, results);
}

void Zone::handleSetLevel(Actor* pActor, const u8 pLevel) {
	EXPECTED(pActor);

	switch (pActor->getActorType()) {
	case ActorType::AT_NPC:
		handleSetLevel(Actor::cast<NPC*>(pActor), pLevel);
		break;
	case ActorType::AT_PLAYER:
		handleSetLevel(Actor::cast<Character*>(pActor), pLevel);
		break;
	default:
		Log::error("Unknown ActorType in " + String(__FUNCTION__));
		break;
	}
}


void Zone::handleSetLevel(Character* pCharacter, const u8 pLevel) {
	EXPECTED(pCharacter);
	if (pCharacter->getLevel() == pLevel) return;

	auto controller = pCharacter->getExperienceController();
	auto connection = pCharacter->getConnection();

	EXPECTED(controller && connection);

	const auto preLevel = controller->getLevel();

	// Add experience to controller.
	controller->setLevel(pLevel);

	const auto postLevel = controller->getLevel();

	_handleLevelChange(pCharacter, preLevel, postLevel);
}

void Zone::handleSetLevel(NPC* pNPC, const u8 pLevel) {
	EXPECTED(pNPC);

	pNPC->setLevel(pLevel);

	// Notify other clients.
	_sendActorLevel(pNPC);
}

void Zone::_handleLevelChange(Character* pCharacter, const u8 pPreviousLevel, const u8 pCurrentLevel) {
	EXPECTED(pCharacter);
	auto controller = pCharacter->getExperienceController();
	auto connection = pCharacter->getConnection();

	EXPECTED(controller && connection);

	// Update experience bar and level.
	connection->sendLevelUpdate(pPreviousLevel, pCurrentLevel, controller->getExperienceRatio());

	// Gaining.
	if (pCurrentLevel > pPreviousLevel) {
		// Send correct message for level gain.
		const u8 numLevels = pCurrentLevel - pPreviousLevel;
		if (numLevels > 1) {
			// You have gained X levels! Welcome to level Y!
			connection->sendLevelsGainMessage(numLevels);
		}
		else {
			// "You gained a level! Welcome to level X!"
			connection->sendLevelGainMessage();
		}

		// Update zone.
		handleLevelIncrease(pCharacter);
	}
	// Losing.
	else if (pCurrentLevel < pPreviousLevel) {
		// NOTE: [UF] Handles the level loss message by itself.

		// Update zone.
		handleLevelDecrease(pCharacter);
	}
}


void Zone::giveExperience(Character* pCharacter, const u32 pExperience) {
	EXPECTED(pCharacter);
	auto controller = pCharacter->getExperienceController();
	EXPECTED(controller);
	if (controller->canGainExperience() == false) return;

	Experience::CalculationResult result;
	Experience::Context context;

	// Configure context.
	context.mSoloKill = true;

	// Set normal experience.
	result.mNormal = pExperience;

	// Process result.
	processExperienceResult(pCharacter, result, context);
}

void Zone::giveAAExperience(Character* pCharacter, const u32 pExperience) {
	EXPECTED(pCharacter);
	auto controller = pCharacter->getExperienceController();
	EXPECTED(controller);
	if (controller->canGainAAExperience() == false) return;

	Experience::CalculationResult result;
	Experience::Context context;

	// Configure context.
	context.mSoloKill = true;

	// Set AA experience.
	result.mAA = pExperience;

	// Process result.
	processExperienceResult(pCharacter, result, context);
}

void Zone::giveGroupLeadershipExperience(Character* pCharacter, const u32 pExperience) {
	EXPECTED(pCharacter);
	auto controller = pCharacter->getExperienceController();
	EXPECTED(controller);
	if (controller->canGainGroupExperience() == false) return;

	Experience::CalculationResult result;
	Experience::Context context;

	// Set Group Leadership experience.
	result.mGroupLeadership = pExperience;

	// Process result.
	processExperienceResult(pCharacter, result, context);
}

void Zone::giveRaidLeadershipExperience(Character* pCharacter, const u32 pExperience) {
	EXPECTED(pCharacter);
	auto controller = pCharacter->getExperienceController();
	EXPECTED(controller);
	if (controller->canGainRaidExperience() == false) return;

	Experience::CalculationResult result;
	Experience::Context context;

	// Set Raid Leadership experience.
	result.mRaidLeadership = pExperience;

	// Process result.
	processExperienceResult(pCharacter, result, context);
}

void Zone::allocateSoloExperience(Character* pCharacter, NPC* pNPC) {
	EXPECTED(pCharacter);
	EXPECTED(pNPC);

	static Experience::CalculationResult result;
	result.reset();
	static Experience::Context context;
	context.reset();

	// Configure context.
	context.mSoloKill = true;
	context.mNPCLevel = pNPC->getLevel();
	context.mNPCMod = pNPC->getExperienceModifier();
	context.mController = pCharacter->getExperienceController();
	context.mZoneMod = getExperienceModifier();

	// Calculate solo experience for kill.
	mExperienceCalculator->calculate(result, context);

	// Process result.
	processExperienceResult(pCharacter, result, context);
}

void Zone::allocateGroupExperience(Group* pGroup, NPC* pNPC) {
	EXPECTED(pGroup);
	EXPECTED(pNPC);

	static Experience::CalculationResult result;
	result.reset();
	static Experience::Context context;
	context.reset();

	// Configure context.
	context.mGroupKill = true;
	context.mNPCLevel = pNPC->getLevel();
	context.mNPCMod = pNPC->getExperienceModifier();
	context.mZoneMod = getExperienceModifier();
	// TODO: Group level.
	// TODO: Group members in Zone.

	std::list<Character*> members;
	for (auto i : members) {
		context.mController = i->getExperienceController();

		// Calculate group experience for kill.
		mExperienceCalculator->calculate(result, context);

		// Process result.
		processExperienceResult(i, result, context);
	}
}

void Zone::allocateRaidExperience(Raid* pRaid, NPC* pNPC)
{

}

void Zone::processExperienceResult(Character* pCharacter, Experience::CalculationResult& pCalculationResult, Experience::Context& pContent) {
	static Experience::GainResult gainResult;
	gainResult.reset();

	auto controller = pCharacter->getExperienceController();
	auto connection = pCharacter->getConnection();

	// Apply result.
	controller->add(gainResult, pCalculationResult.mNormal, pCalculationResult.mAA, pCalculationResult.mGroupLeadership, pCalculationResult.mRaidLeadership);

	// Notify Client.

	// Gain normal experience.
	if (pCalculationResult.mNormal > 0) {
		// Update experience bars.
		connection->sendExperienceUpdate(controller->getExperienceRatio(), controller->getAAExperienceRatio());

		if (pContent.mSoloKill) {
			// "You gain experience!!"
			connection->sendExperienceMessage();
		}
		else if (pContent.mGroupKill) {
			// "You gain party experience!!"
			connection->sendGroupExperienceMessage();
		}
		else if (pContent.mRaidKill) {
			// "You gained raid experience!"
			connection->sendRaidExperienceMessage();
		}

		// Gain level(s).
		if (gainResult.mLevels > 0) {
			// "You gained a level! Welcome to level X!"
			if (gainResult.mLevels == 1) connection->sendLevelGainMessage();
			// You have gained X levels! Welcome to level Y!
			else connection->sendLevelsGainMessage(gainResult.mLevels);

			// Update Zone.
			handleLevelIncrease(pCharacter);
		}
	}
	// Gain Alternate Advanced experience.
	if (pCalculationResult.mAA > 0) {
		// Update experience bars.
		connection->sendAAExperienceUpdate(controller->getAAExperienceRatio(), controller->getUnspentAAPoints(), controller->getExperienceToAA());

		// Gain AA point(s).
		if (gainResult.mAAPoints > 0) {
			// "You have gained an ability point!  You now have %1 ability points."
			connection->sendAAPointGainMessage(controller->getUnspentAAPoints());
		}
	}
	// Gain Group Leadership experience.
	if (pCalculationResult.mGroupLeadership > 0) {
		// Update experience bars.
		connection->sendLeadershipExperienceUpdate(controller->getGroupRatio(), controller->getGroupPoints(), controller->getRaidRatio(), controller->getRaidPoints());

		// "You gain group leadership experience! (% 1)"
		connection->sendGainGroupLeadershipExperienceMessage();

		// Gain Leadership point(s).
		if (gainResult.mGroupPoints > 0) {
			// "You have gained a group leadership point!"
			connection->sendGainGroupLeadershipPointMessage();
		}

	}
	// Gain Raid Leadership experience.
	if (pCalculationResult.mRaidLeadership > 0) {
		// Update experience bars.
		connection->sendLeadershipExperienceUpdate(controller->getGroupRatio(), controller->getGroupPoints(), controller->getRaidRatio(), controller->getRaidPoints());

		// "You gain raid leadership experience! (% 1)"
		connection->sendGainRaidLeadershipExperienceMessage();

		// Gain Raid Leadership point(s).
		if (gainResult.mRaidPoints > 0) {
			// "You have gained a raid leadership point!"
			connection->sendGainRaidLeadershipPointMessage();
		}
	}

	// 8584 You have reached the maximum number of unused group leadership points.  You must spend some points before you can receive any more experience.
	// 8591 You have reached the maximum number of unused raid leadership points.  You must spend some points before you can receive any more experience.
}

void Zone::onCombine(Character* pCharacter, const u32 pSlot) {
	EXPECTED(pCharacter);
	auto connection = pCharacter->getConnection();

	// Check: Character is in a state that allows for combining.
	EXPECTED(pCharacter->canCombine());

	// Check: The combine is occurring from a valid slot id. (UF Client Checked)
	EXPECTED(SlotID::isMainInventory(pSlot));

	// Check: Cursor is empty. (UF Client Checked)
	EXPECTED(pCharacter->getInventory()->isCursorEmpty() == true);

	// Check: Valid Item at slot id.
	auto container = pCharacter->getInventory()->getItem(pSlot);
	EXPECTED(container);

	// Check: Item is a container.
	EXPECTED(container->isContainer());

	// Check: Container has 'Combine' option.
	EXPECTED(container->isCombineContainer());

	// Check: Container is not empty. (UF Client Checked)
	EXPECTED(container->isEmpty() == false);

	// Check: None of the Items in the container have > 1 stacks. (UF Client Checked)
	auto f = [](Item* pItem) { return pItem->getStacks() == ((pItem->isStackable()) ? 1 : 0); }; // This is annoying. Non-stackable Items should still have getStacks == 1.
	EXPECTED(container->forEachContents(f));

	// Test.
	std::list<Item*> results;
	std::list<Item*> items;
	container->getContents(items);

	if (container->getID() == ItemID::TransmuterTen) {
		Item* item = mTransmutation->transmute(items);
		if (item) results.push_back(item);
	}

	// This is what I want.
	//mCraftingSystem->combine(items, results);

	for (auto i : items) {
		// Update Client.
		connection->sendDeleteItem(i->getSlot());
		// Update Inventory.
		EXPECTED(container->clearContents(i->getSubIndex()));
		// Free memory.
		delete i;
	}
	items.clear();

	EXPECTED(container->isEmpty()); // We expect the container to be empty at this point.

	// Note: Underfoot locks the UI until a reply is received.
	connection->sendCombineReply();

	for (auto i : results) {
		pCharacter->getInventory()->pushCursor(i);
		connection->sendItemSummon(i);
	}
}

const bool Zone::onGuildCreate(Character* pCharacter, const String& pGuildName) {
	if (!pCharacter) return false;

	const bool success = mGuildManager->onCreate(pCharacter, pGuildName);

	// Handle: Failure.
	if (!success) {

		// Check: Guild name already used.
		if (mGuildManager->exists(pGuildName)) {
			pCharacter->getConnection()->sendSimpleMessage(MessageType::Red, StringID::GUILD_NAME_IN_USE);
			return true;
		}

		// TODO: Log
		return false;
	}

	// Notify ZoneManager.
	mZoneManager->onCreateGuild();

	// Update Zone.
	onChangeGuild(pCharacter);

	return true;
}

const bool Zone::onGuildDelete(Character* pCharacter) {
	if (!pCharacter) return false;

	const bool success = mGuildManager->onDelete(pCharacter);

	return true;
}

const bool Zone::onGuildInvite(Character* pInviter, const String& pInviteeName) {
	if (!pInviter) return false;
	
	// Try to find Character to invite.
	auto character = mZoneManager->findCharacter(pInviteeName, false, nullptr);

	// Handle: Character not found, either offline, zoning or does not exist.
	if (!character) {
		pInviter->message(MessageType::Yellow, pInviteeName + " could not be found.");
		return true;
	}

	const bool success = mGuildManager->onInvite(pInviter, character);

	// Handle: Failure.
	if (!success) {

		// Handle: Character was found but is already in another guild.
		if (character->hasGuild()) {
			pInviter->message(MessageType::Yellow, pInviteeName + " already has a guild.");
			return true;
		}
		// Handle: Character has a pending guild invite.
		else if (character->hasPendingGuildInvite()) {
			pInviter->message(MessageType::Yellow, pInviteeName + " is considering joining another guild.");
			return true;
		}

		// TODO: Log
		return false;
	}

	// Record invitation.
	character->setPendingGuildInviteID(pInviter->getGuildID());
	character->setPendingGuildInviteName(pInviter->getName());

	// Notify invitee.
	character->getConnection()->sendGuildInvite(pInviter->getName(), pInviter->getGuild()->getID());
	return true;
}

const bool Zone::onGuildInviteAccept(Character* pCharacter) {
	if (!pCharacter) return false;

	const bool success = mGuildManager->onInviteAccept(pCharacter);

	if (!success) {
		// TODO: Log
		return true;
	}
	
	// Let the inviter know the invite was accepted.
	auto inviter = mZoneManager->findCharacter(pCharacter->getPendingGuildInviteName());
	if (inviter)
		inviter->message(MessageType::Yellow, pCharacter->getName() + " has accepted your invitation to join the guild.");

	pCharacter->clearPendingGuildInvite();

	// Update Zone.
	onChangeGuild(pCharacter);

	return true;
}

const bool Zone::onGuildInviteDecline(Character* pCharacter) {
	if (!pCharacter) return false;
	if (!pCharacter->hasPendingGuildInvite()) return false;

	// Let the inviter know the invite was accepted.
	auto inviter = mZoneManager->findCharacter(pCharacter->getPendingGuildInviteName());
	if (inviter)
		inviter->message(MessageType::Yellow, pCharacter->getName() + " has declined your invitation to join the guild.");

	pCharacter->clearPendingGuildInvite();
	return true;
}

const bool Zone::onGuildRemove(Character* pRemover, const String& pRemoveeName) {
	if (!pRemover) return false;

	// Try to find Character to remove.
	auto character = mZoneManager->findCharacter(pRemoveeName, false, nullptr);

	//// Handle: Character not found, either offline, zoning or does not exist.
	//if (!character) {
	//	pRemover->message(MessageType::Yellow, pRemoveeName + " could not be found.");
	//	return true;
	//}

	if (pRemover->getName() == pRemoveeName) {
		mGuildManager->onLeave(pRemover);
	}

	const bool success = mGuildManager->onRemove(pRemover, pRemoveeName);

	return true;
}

const bool Zone::onGuildLeave(Character* pCharacter) {
	if (!pCharacter) return false;

	const bool success = mGuildManager->onLeave(pCharacter);
	return true;
}

const bool Zone::onGuildPromote(Character* pPromoter, const String& pPromoteeName) {
	if (!pPromoter) return false;

	// Check: Character has a valid target.
	if (!pPromoter->hasTarget()) return false;
	if (!pPromoter->targetIsCharacter()) return false;

	auto promotee = Actor::cast<Character*>(pPromoter->getTarget());

	const auto success = mGuildManager->onPromote(pPromoter, promotee);

	// Handle: Failure.
	if (!success) {
		// TODO: Log
		return false;
	}

	return true;
}

const bool Zone::onGuildDemote(Character* pDemoter, const String& pDemoteeName) {
	return true;
}

const bool Zone::onGuildSetMOTD(Character* pCharacter, const String& pMOTD) {
	if (pCharacter) return false;

	const bool success = mGuildManager->onSetMOTD(pCharacter, pMOTD);
	return true;
}

const bool Zone::onGuildGetMOTD(Character* pCharacter) {
	if (pCharacter) return false;

	const bool success = mGuildManager->onGetMOTD(pCharacter);
	return true;
}

const bool Zone::onGuildMakeLeader(Character* pCharacter, const String& pLeaderName) {
	if (pCharacter) return false;

	const bool success = mGuildManager->onMakeLeader(pCharacter, pLeaderName);
	return true;
}

const bool Zone::onGuildSetURL(Character* pSetter, const String& pURL) {
	if (pSetter) return false;

	const bool success = mGuildManager->onSetURL(pSetter, pURL);
	return true;
}

const bool Zone::onGuildSetChannel(Character* pSetter, const String& pChannel) {
	if (pSetter) return false;

	const bool success = mGuildManager->onSetChannel(pSetter, pChannel);
	return true;
}

const bool Zone::onGuildStatusRequest(Character* pCharacter, const String& pCharacterName) {
	// TODO
	return true;
}

const bool Zone::onGuildSetFlags(Character* pSetter, const String& pCharacterName, const bool pBanker, const bool pAlt) {
	if (!pSetter) return false;

	const bool success = mGuildManager->onSetFlags(pSetter, pCharacterName, pBanker, pAlt);
	return true;
}

const bool Zone::onGuildSetPublicNote(Character* pSetter, const String& pCharacterName, const String& pPublicNote) {
	if (!pSetter) return false;

	const bool success = mGuildManager->onSetPublicNote(pSetter, pCharacterName, pPublicNote);
	return true;
}
