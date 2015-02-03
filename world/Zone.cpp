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
#include "TitleManager.h"
#include "TradeHandler.h"
#include "LootHandler.h"
#include "ShopHandler.h"

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
	memset(mActors, 0, sizeof(mActors));
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

const bool Zone::initialise(ZoneManager* pZoneManager, ILogFactory* pLogFactory, Data::Zone* pZoneData, Experience::Calculator* pExperienceCalculator, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, TitleManager* pTitleManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory, NPCFactory* pNPCFactory) {
	if (mInitialised) return false;
	if (!pZoneManager) return false;
	if (!pLogFactory) return false;
	if (!pZoneData) return false;
	if (!pExperienceCalculator) return false;
	if (!pGroupManager) return false;
	if (!pRaidManager) return false;
	if (!pGuildManager) return false;
	if (!pTitleManager) return false;
	if (!pCommandHandler) return false;
	if (!pItemFactory) return false;
	if (!pNPCFactory) return false;

	mZoneManager = pZoneManager;
	mLogFactory = pLogFactory;
	mExperienceCalculator = pExperienceCalculator;
	mGroupManager = pGroupManager;
	mRaidManager = pRaidManager;
	mGuildManager = pGuildManager;
	mTitleManager = pTitleManager;
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
	if (!mZoneConnectionManager->initialise(mPort, this, mLogFactory, mGuildManager)) {
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
	mTradeHandler = new TradeHandler(this, mLogFactory);
	mLootHandler = new LootHandler(this, mLogFactory);
	mShopHandler = new ShopHandler(this, mItemFactory, mLogFactory);

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

	// Add.
	addActor(pCharacter);

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

	// NOTE: UF actually retains this. Not sure how to handle it.
	// Character has a Group invitation.
	if (pCharacter->hasGroupInvitation())
		// Decline it.
		onGroupInviteDecline(pCharacter);

	// Character has a Raid invitation.
	if (pCharacter->hasRaidInvitation())
		// Decline it.
		onRaidInviteDecline(pCharacter);

	// Character has a Guild invitation.
	if (pCharacter->hasGuildInvitation())
		// Decline it.
		onGuildInviteDecline(pCharacter);

	// Character has a Guild.
	if (pCharacter->hasGuild())
		mGuildManager->onLeaveZone(pCharacter);

	// Character has a Raid.
	if (pCharacter->hasRaid())
		mRaidManager->onLeaveZone(pCharacter);

	// Character has a Group.
	if (pCharacter->hasGroup())
		mGroupManager->onLeaveZone(pCharacter);

	pCharacter->onLeaveZone();

	// Remove.
	removeActor(pCharacter);
	pCharacter->setSpawnID(0);

	// Handle: Character leaving Zone while looting.
	if (pCharacter->isLooting()) {
		auto corpse = pCharacter->getLootingCorpse();

		// Clear Character reference from corpse.
		corpse->getLootController()->clearLooter();
		// Clear corpse reference from Character.
		pCharacter->setLootingCorpse(nullptr);
	}

	// Handle: Character leaving Zone while shopping.
	if (pCharacter->isShopping()) {
		auto npc = pCharacter->getShoppingWith();

		// Disassociate Character and NPC.
		npc->removeShopper(pCharacter);
		pCharacter->setShoppingWith(nullptr);
	}

	// Handle: Character is leaving the Zone while targeted by other Actors.
	if (pCharacter->hasTargeters())
		pCharacter->clearTargeters();

	// Handle: Character is leaving the Zone while trading.
	if (pCharacter->isTrading()) {
		onTradeCancel(pCharacter, pCharacter->getSpawnID());
	}

	// Handle: Character is leaving the zone and has agro.
	if (pCharacter->hasHaters()) {
		auto haters = pCharacter->getHaters();
		for (auto i : haters) {
			i->getHateController()->remove(pCharacter);
		}

		pCharacter->clearHaters();
	}

	// Casting.

	// Notify ZoneManager.
	mZoneManager->onLeaveZone(pCharacter);
}

void Zone::onCampComplete(Character* pCharacter) {
	if (!pCharacter) return;

	mLog->info("Character (" + pCharacter->getName() + ") camped.");

	// Save
	saveCharacter(pCharacter);

	// Remove.
	removeActor(pCharacter);

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

	// Update Zone.
	_sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::LinkDead, 1, false);

	// Cancel trade if a Character goes LD.
	if (pCharacter->isTrading()) {
		onTradeCancel(pCharacter, pCharacter->getSpawnID());
	}

	// Handle: Character leaving Zone while shopping.
	if (pCharacter->isShopping()) {
		auto npc = pCharacter->getShoppingWith();

		// Disassociate Character and NPC.
		npc->removeShopper(pCharacter);
		pCharacter->setShoppingWith(nullptr);
	}

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
	saveCharacter(pCharacter);

	// Remove.
	removeActor(pCharacter);

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

void Zone::_sendSpawnAppearance(Actor* pActor, const u16 pType, const u32 pParameter, const bool pIncludeSender) {
	using namespace Payload::Zone;
	EXPECTED(pActor);

	auto packet = SpawnAppearance::construct(pActor->getSpawnID(), pType, pParameter);
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

const bool Zone::onChannelMessage(Character* pCharacter, const u32 pChannelID, const String& pSenderName, const String& pTargetName, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	// Check: Character may be muted.
	if (pCharacter->isMuted()) {
		pCharacter->notify("You are muted! Your message was not delivered.");
		return true;
	}

	// Handle: Guild.
	if (pChannelID == ChannelID::Guild) {
		onGuildMessage(pCharacter, pMessage);
	}
	// Handle: Group.
	else if (pChannelID == ChannelID::Group) {
		onGroupMessage(pCharacter, pMessage);
	}
	// Handle: Shout.
	else if (pChannelID == ChannelID::Shout) {
		onShoutMessage(pCharacter, pMessage);
	}
	// Handle: Auction.
	else if (pChannelID == ChannelID::Auction) {
		onAuctionMessage(pCharacter, pMessage);
	}
	// Handle: OOC.
	else if (pChannelID == ChannelID::OOC) {
		onOOCMessage(pCharacter, pMessage);
	}
	// Handle: Broadcast.
	else if (pChannelID == ChannelID::Broadcast) {
		onBroadcastMessage(pCharacter, pMessage);
	}
	// Handle: Tell.
	else if (pChannelID == ChannelID::Tell) {
		onTellMessage(pCharacter, pTargetName, pMessage);
	}
	// Handle: Say.
	else if (pChannelID == ChannelID::Say) {
		onSayMessage(pCharacter, pMessage);
	}
	// Handle: GMSay.
	else if (pChannelID == ChannelID::GMSay) {
		onGMSayMessage(pCharacter, pMessage);
	}
	// Handle: Raid.
	else if (pChannelID == ChannelID::Raid) {
		onRaidMessage(pCharacter, pMessage);
	}
	// Handle: UCS.
	else if (pChannelID == ChannelID::UCS) {
		onUCSMessage(pCharacter, pMessage);
	}
	// Handle: Emote.
	else if (pChannelID == ChannelID::Emote) {
		onEmoteMessage(pCharacter, pMessage);
	}
	// Handle: Unknown.
	else {
		mLog->error("Unknown channel ID: " + toString(pChannelID) + " from: " + pCharacter->getName() + " in " + __FUNCTION__);
	}

	return true;
}

const bool Zone::onGuildMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (!pCharacter->hasGuild()) return false;
	if (pMessage.empty()) return false;

	// Notify GuildManager.
	mGuildManager->onMessage(pCharacter, pMessage);
	return true;
}

const bool Zone::onGroupMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (!pCharacter->hasGroup()) return false;
	if (pMessage.empty()) return false;

	// Notify GroupManager.
	mGroupManager->onMessage(pCharacter, pMessage);
	return true;
}

const bool Zone::onShoutMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	// Send to other Characters.
	sendMessage(pCharacter, ChannelID::Shout, pMessage);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Shout, pCharacter);

	return true;
}

const bool Zone::onAuctionMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	sendMessage(pCharacter, ChannelID::Auction, pMessage);

	// TODO: Server Auction
	return true;
}

const bool Zone::onOOCMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	sendMessage(pCharacter, ChannelID::OOC, pMessage);

	// TODO: Server OOC
	return true;
}

const bool Zone::onBroadcastMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	// Check: Character is a GM.
	if (!pCharacter->isGM()) {
		mLog->error("Character: " + pCharacter->getName() + " tried to use Broadcast channel while not GM!");
		return false;
	}

	return true;
}

const bool Zone::onTellMessage(Character* pCharacter, const String& pTargetName, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	mZoneManager->handleTell(pCharacter, pTargetName, pMessage);
	return true;
}

const bool Zone::onSayMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	// Check: Command entered.
	if (pMessage[0] == COMMAND_TOKEN) {
		mCommandHandler->command(pCharacter, pMessage);
		return true;
	}

	// Send to other Characters.
	sendMessage(pCharacter, ChannelID::Say, pMessage);

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Say, pCharacter);

	return true;
}

const bool Zone::onGMSayMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	// Check: Character is a GM.
	if (!pCharacter->isGM()) {
		mLog->error("Character: " + pCharacter->getName() + " tried to use GMSay channel while not GM!");
		return false;
	}

	return true;
}

const bool Zone::onRaidMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (!pCharacter->hasRaid()) return false;
	if (pMessage.empty()) return false;

	// Notify RaidManager.
	mRaidManager->onMessage(pCharacter, pMessage);
	return true;
}

const bool Zone::onUCSMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	return true;
}

const bool Zone::onEmoteMessage(Character* pCharacter, const String& pMessage) {
	using namespace Payload::Zone;
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	String message = pCharacter->getName() + " " + pMessage;
	auto packet = Emote::construct(message);
	sendToVisible(pCharacter, packet, false);
	delete packet;

	return true;
}

void Zone::sendMessage(Character* pCharacter, const u32 pChannel, const String pMessage) {
	EXPECTED(pCharacter);

	const auto sender = pCharacter->getConnection();
	auto packet = ZoneConnection::makeChannelMessage(pChannel, pCharacter->getName(), pMessage);

	for (auto i : mCharacters) {
		if (i != pCharacter)
			i->getConnection()->sendPacket(packet);
	}

	delete packet;
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

const bool Zone::onAnimationChange(Actor* pActor, const u8 pAnimation, const u8 pSpeed, const bool pIncludeSender) {
	using namespace Payload::Zone;
	if (!pActor) return false;

	// Update Zone.
	auto packet = ActorAnimation::construct(pActor->getSpawnID(), pAnimation, pSpeed);
	sendToVisible(pActor, packet, pIncludeSender);
	delete packet;

	return true;
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

void Zone::saveCharacter(Character*pCharacter) {
	if (!pCharacter) return;

	pCharacter->_updateForSave();

	if (!mZoneManager->saveCharacter(pCharacter)) {
		mLog->error("Failed to save Character: " + pCharacter->getName());
	}
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
	auto packet = Payload::makeGuildNameList(mGuildManager->getGuilds());
	for (auto i : mCharacters) {
		i->getConnection()->sendPacket(packet);
	}
	delete packet;
}

void Zone::onChangeGuild(Character* pCharacter) {
	if (!pCharacter) return;

	_sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::GuildID, pCharacter->getGuildID(), true);
	_sendSpawnAppearance(pCharacter, SpawnAppearanceTypeID::GuildRank, pCharacter->getGuildRank(), true);
}

const bool Zone::onTargetChange(Character* pCharacter, const u16 pSpawnID) {
	using namespace Payload::Zone;
	if (!pCharacter) return false;

	// Character is clearing their target.
	if (pSpawnID == 0) {
		pCharacter->clearTarget();
		return true;
	}

	// Find Actor being targeted.
	auto actor = getActor(pSpawnID);
	if (!actor) {
		// This can happen for legitimate reasons, just reject the target.
		// THIS DOES NOT WORK.
		//pCharacter->getConnection()->sendRejectTarget();
		return true;
	}

	pCharacter->setTarget(actor);

	// Send HP of new target.
	auto packet = ActorHPUpdate::construct(actor->getHPPercent(), actor->getHPPercent());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;

	// Dispatch Event.
	EventDispatcher::getInstance().event(Event::Target, pCharacter);

	return true;
}

void Zone::handleFaceChange(Character* pCharacter) {
	EXPECTED(pCharacter);
	_sendAppearanceUpdate(pCharacter);
}

void Zone::handleVisibilityAdd(Character* pCharacter, Actor* pAddActor) {
	EXPECTED(pCharacter);
	EXPECTED(pAddActor);

	pAddActor->_syncPosition();

	u32 size = pAddActor->getDataSize();
	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);
	EXPECTED(pAddActor->copyData(writer));

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
	using namespace Payload::Zone;
	EXPECTED(pCharacter);
	EXPECTED(pRemoveActor);

	auto packet = DespawnActor::construct(pRemoveActor->getSpawnID());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Zone::addActor(Actor* pActor) {
	EXPECTED(pActor);
	
	// Adding NPC.
	if (pActor->isNPC()) {
		mNPCs.push_back(Actor::cast<NPC*>(pActor));
	}
	// Adding Character.
	else if (pActor->isCharacter()) {
		mCharacters.push_back(Actor::cast<Character*>(pActor));
	}

	// Notify Scene.
	mScene->add(pActor);

	mActors[pActor->getSpawnID()] = pActor;
}

void Zone::removeActor(Actor* pActor) {
	if (!pActor) return;

	mActors[pActor->getSpawnID()] = nullptr;

	// Notify Scene.
	mScene->remove(pActor);
	
	// Removing NPC.
	if (pActor->isNPC()) {
		mNPCs.remove(Actor::cast<NPC*>(pActor));
	}
	// Removing Character.
	else if (pActor->isCharacter()) {
		mCharacters.remove(Actor::cast<Character*>(pActor));
	}
	// Error.
	else {
		mLog->error("Unknown Actor type in " + String(__FUNCTION__));
	}
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

void Zone::handleCastingBegin(Character* pCharacter, const uint16 pSlot, const u32 pSpellID) {
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

void Zone::sendToVisible(Actor* pActor, EQApplicationPacket* pPacket, bool pIncludeSender) {
	if (!pActor) return;
	if (!pPacket) return;

	if (pActor->isCharacter())
		sendToVisible(Actor::cast<Character*>(pActor), pPacket, pIncludeSender);
	else
		sendToVisible(pActor, pPacket);
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

void Zone::handleDeath(Actor* pActor, Actor* pKiller, const u32 pDamage, const u32 pSkill) {
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

void Zone::onLootRequest(Character* pCharacter, const u32 pSpawnID) { mLootHandler->onRequest(pCharacter, pSpawnID); }
void Zone::onLootFinished(Character* pCharacter) { mLootHandler->onFinished(pCharacter); }
void Zone::onLootItem(Character* pCharacter, const u32 pSlotID) { mLootHandler->onLootItem(pCharacter, pSlotID); }

void Zone::handleConsider(Character* pCharacter, const u32 pSpawnID) {
	EXPECTED(pCharacter);

	if (pSpawnID == pCharacter->getSpawnID()){
		// TODO: Client sends self consider, need to double check how this should be handled.
		return;
	}
	
	Actor* actor = pCharacter->findVisible(pSpawnID);
	EXPECTED(actor);

	pCharacter->getConnection()->sendConsiderResponse(pSpawnID, 1); // TODO: Message
}

void Zone::handleConsiderCorpse(Character* pCharacter, const u32 pSpawnID) {
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
	if (!pCharacter) return false;
	static const float MaxBankingDistance = 405.0f; // TODO:

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

void Zone::handleDamage(Actor* pAttacker, Actor* pDefender, const i32 pAmount, const uint8 pType, const uint16 pSpellID) {
	using namespace Payload::Zone;

	u32 sequence = Random::make(0, 20304843);
	auto packet = Damage::construct(pDefender->getSpawnID(), pAttacker->getSpawnID(), pAmount, pType, sequence, pSpellID);

	if (pDefender->isCharacter())
		sendToVisible(Actor::cast<Character*>(pDefender), packet, true);
	else
		sendToVisible(pDefender, packet);

	delete packet;
}

void Zone::handleCriticalHit(Actor* pActor, const i32 pDamage) {
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

void Zone::handleNimbusAdded(Actor* pActor, const u32 pNimbusID) {
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

void Zone::handleNimbusRemoved(Actor* pActor, const u32 pNimbusID) {
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

void Zone::handleRandomRequest(Character* pCharacter, const u32 pLow, const u32 pHigh) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	u32 low = pLow;
	u32 high = pHigh;

	if (low == 0 && high == 0) { high = 100; }

	EXPECTED(low <= high);

	const u32 result = Random::make(low, high);
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

	const auto actorType = pActor->getActorType();
	switch (actorType) {
	case ActorType::AT_NPC:
		handleSetLevel(Actor::cast<NPC*>(pActor), pLevel);
		break;
	case ActorType::AT_PLAYER:
		handleSetLevel(Actor::cast<Character*>(pActor), pLevel);
		break;
	default:
		mLog->error("Unknown ActorType: " + toString(actorType) + " in " + String(__FUNCTION__));
		break;
	}
}


void Zone::handleSetLevel(Character* pCharacter, const u8 pLevel) {
	if (!pCharacter) return;
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
	EXPECTED(SlotID::isMain(pSlot));

	// Check: Cursor is empty. (UF Client Checked)
	EXPECTED(pCharacter->getInventory()->isCursorEmpty() == true);

	// Check: Valid Item at slot id.
	auto container = pCharacter->getInventory()->get(pSlot);
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

	// Notify GuildManager.
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

	// Notify GuildManager.
	const bool success = mGuildManager->onInvite(pInviter, character);

	// Handle: Failure.
	if (!success) {

		// Handle: Character was found but is already in another guild.
		if (character->hasGuild()) {
			pInviter->message(MessageType::Yellow, pInviteeName + " already has a guild.");
			return true;
		}
		// Handle: Character has a pending guild invite.
		else if (character->hasGuildInvitation()) {
			pInviter->message(MessageType::Yellow, pInviteeName + " is considering joining another guild.");
			return true;
		}

		// TODO: Log
		return false;
	}

	// Record invitation.
	auto& invitation = character->getGuildInvitation();
	invitation.mGuildID = pInviter->getGuildID();
	invitation.mInviterName = pInviter->getName();
	invitation.mTimeInvited = Utility::Time::now();

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
	auto& invitation = pCharacter->getGuildInvitation();
	auto inviter = mZoneManager->findCharacter(invitation.mInviterName);
	if (inviter)
		inviter->message(MessageType::Yellow, pCharacter->getName() + " has accepted your invitation to join the guild.");

	// Update Zone.
	onChangeGuild(pCharacter);

	pCharacter->clearGuildInvitation();
	return true;
}

const bool Zone::onGuildInviteDecline(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: The Character declining the invite has a pending invite.
	if (!pCharacter->hasGuildInvitation()) return false;

	// Let the inviter know the invite was accepted.
	auto& invitation = pCharacter->getGuildInvitation();
	auto inviter = mZoneManager->findCharacter(invitation.mInviterName);
	if (inviter)
		inviter->message(MessageType::Yellow, pCharacter->getName() + " has declined your invitation to join the guild.");

	pCharacter->clearGuildInvitation();
	return true;
}

const bool Zone::onGuildRemove(Character* pRemover, const String& pRemoveeName) {
	if (!pRemover) return false;

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

	// Notify GuildManager.
	const auto success = mGuildManager->onPromote(pPromoter, promotee);

	// Respond.
	if (!success)
		pPromoter->notify("Failed to promote " + pPromoteeName);

	return true;
}

const bool Zone::onGuildDemote(Character* pDemoter, const String& pDemoteeName) {
	if (!pDemoter) return false;

	// Check: Character has a valid target.
	if (!pDemoter->hasTarget()) return false;
	if (!pDemoter->targetIsCharacter()) return false;

	auto demotee = Actor::cast<Character*>(pDemoter->getTarget());

	// Notify GuildManager.
	const auto success = mGuildManager->onDemote(pDemoter, demotee);

	// Respond.
	if (!success)
		pDemoter->notify("Failed to demote " + pDemoteeName);

	return success;
}

const bool Zone::onGuildMakeLeader(Character* pCharacter, const String& pLeaderName) {
	if (!pCharacter) return false;
	// NOTE: Changing leader does not require that the new leader is targeted (like promote/demote).

	// Notify GuildManager.
	const bool success = mGuildManager->onMakeLeader(pCharacter, pLeaderName);

	// Respond.
	if (!success)
		pCharacter->notify("Failed to make " + pLeaderName + " leader.");

	return success;
}

const bool Zone::onGuildSetMOTD(Character* pCharacter, const String& pMOTD) {
	if (!pCharacter) return false;

	// Notify GuildManager.
	const bool success = mGuildManager->onSetMOTD(pCharacter, pMOTD);

	// Respond.
	if (!success)
		pCharacter->notify("Failed to update guild MOTD.");

	return success;
}

const bool Zone::onGuildMOTDRequest(Character* pCharacter) {
	if (!pCharacter) return false;

	// Notify GuildManager.
	const bool success = mGuildManager->onMOTDRequest(pCharacter);

	// Respond.
	if (!success)
		pCharacter->notify("Failed to retrieve guild MOTD.");

	return success;
}

const bool Zone::onGuildSetURL(Character* pSetter, const String& pURL) {
	if (!pSetter) return false;

	// Notify GuildManager.
	const bool success = mGuildManager->onSetURL(pSetter, pURL);

	// Respond.
	if (!success)
		pSetter->notify("Failed to update guild URL.");

	return success;
}

const bool Zone::onGuildSetChannel(Character* pSetter, const String& pChannel) {
	if (!pSetter) return false;

	// Notify GuildManager.
	const bool success = mGuildManager->onSetChannel(pSetter, pChannel);

	// Respond.
	if (!success)
		pSetter->notify("Failed to update guild channel.");

	return success;
}

const bool Zone::onGuildSetFlags(Character* pSetter, const String& pCharacterName, const u32 pFlags) {
	if (!pSetter) return false;

	const bool success = mGuildManager->onSetFlags(pSetter, pCharacterName, pFlags);

	// Respond.
	if (!success)
		pSetter->notify("Failed to update flags.");
	else
		pSetter->notify(pCharacterName + " flags updated.");

	return success;
}

const bool Zone::onGuildSetPublicNote(Character* pSetter, const String& pCharacterName, const String& pPublicNote) {
	if (!pSetter) return false;

	// Notify GuildManager.
	const bool success = mGuildManager->onSetPublicNote(pSetter, pCharacterName, pPublicNote);

	// Respond.
	if (!success)
		pSetter->notify("Failed to set public note.");

	return success;
}

void Zone::onGroupInvite(Character* pInviter, const String& pInviteeName) { mGroupManager->onInvite(pInviter, pInviteeName); }
void Zone::onGroupInviteAccept(Character* pCharacter) { mGroupManager->onInviteAccept(pCharacter); }
void Zone::onGroupInviteDecline(Character* pCharacter) { mGroupManager->onInviteDecline(pCharacter); }
void Zone::onGroupLeave(Character* pCharacter) { mGroupManager->onLeave(pCharacter); }
void Zone::onGroupRemove(Character* pCharacter, const String& pCharacterName) { mGroupManager->onRemove(pCharacter, pCharacterName); }
void Zone::onGroupMakeLeader(Character* pCharacter, const String& pTargetName) { mGroupManager->onMakeLeader(pCharacter, pTargetName); }
void Zone::onGroupRoleChange(Character* pCharacter, const String& pTargetName, const u32 pRoleID, const u8 pToggle) { mGroupManager->onRoleChange(pCharacter, pTargetName, pRoleID, pToggle); }

void Zone::onShopRequest(Character* pCharacter, const u32 pSpawnID) { mShopHandler->onRequest(pCharacter, pSpawnID); }
void Zone::onShopFinished(Character* pCharacter) { mShopHandler->onFinished(pCharacter); }
void Zone::onShopSell(Character* pCharacter, const u32 pSlotID, const u32 pStacks) { mShopHandler->onSell(pCharacter, pSlotID, pStacks); }
void Zone::onShopBuy(Character* pCharacter, const u32 pInstanceID, const u32 pStacks) { mShopHandler->onBuy(pCharacter, pInstanceID, pStacks); }

void Zone::onTradeRequest(Character* pCharacter, const u32 pSpawnID) { mTradeHandler->onRequest(pCharacter, pSpawnID); }
void Zone::onTradeResponse(Character* pCharacter, const u32 pSpawnID) { mTradeHandler->onResponse(pCharacter, pSpawnID); }
void Zone::onTradeBusy(Character* pCharacter, const u32 pSpawnID) { mTradeHandler->onBusy(pCharacter, pSpawnID); }
void Zone::onTradeAccept(Character* pCharacter, const u32 pSpawnID) { mTradeHandler->onAccept(pCharacter, pSpawnID); }
void Zone::onTradeCancel(Character* pCharacter, const u32 pSpawnID) { mTradeHandler->onCancel(pCharacter, pSpawnID); }

const bool Zone::onPetCommand(Character* pCharacter, const u32 pCommand) {
	return true;
}

const bool Zone::onEmote(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;
	if (pMessage.empty()) return false;

	// Check: Character may be muted.
	if (pCharacter->isMuted()) {
		pCharacter->notify("You are muted! Your message was not delivered.");
		return true;
	}

	return onEmoteMessage(pCharacter, pMessage);
}

const bool Zone::mute(Character* pCharacter, const String& pCharacterName) {
	// Find Character.
	auto character = mZoneManager->findCharacter(pCharacterName, true); // Muting a zoning Character is fine.

	// Check: Character exists.
	if (!character) {
		pCharacter->notify("Could not find Character: " + pCharacterName);
		return false;
	}
	
	// Check: You can not mute yourself.
	if (character == pCharacter) {
		pCharacter->notify("You can not mute yourself.");
		return false;
	}

	// Check: You can not mute someone with higher status.
	if (pCharacter->getStatus() < character->getStatus()) {
		pCharacter->notify("You can not mute a Character with higher status.");
		return false;
	}

	// Check: Already muted.
	if (character->isMuted()) {
		pCharacter->notify(pCharacterName + " is already muted.");
		return false;
	}

	// Send the muted Character a message if they are not zoning.
	if (!character->isZoning())
		character->notify("You have been muted.");

	character->setMuted(true);
	pCharacter->notify(pCharacterName + " has been muted.");
	return true;
}

const bool Zone::unmute(Character* pCharacter, const String& pCharacterName) {
	// Find Character.
	auto character = mZoneManager->findCharacter(pCharacterName, true); // Unmuting a zoning Character is fine.

	// Check: Character exists.
	if (!character) {
		pCharacter->notify("Could not find Character: " + pCharacterName);
		return false;
	}

	// Check: You can not unmute yourself.
	if (character == pCharacter) {
		pCharacter->notify("You can not unmute yourself.");
		return false;
	}

	// Check: You can not unmute someone with higher status.
	if (pCharacter->getStatus() < character->getStatus()) {
		pCharacter->notify("You can not unmute a Character with higher status.");
		return false;
	}

	// Check: Not muted.
	if (!character->isMuted()) {
		pCharacter->notify(pCharacterName + " is not muted.");
		return false;
	}

	// Send the unmuted Character a message if they are not zoning.
	if (!character->isZoning())
		character->notify("You have been unmuted.");

	character->setMuted(false);
	pCharacter->notify(pCharacterName + " has been unmuted.");
	return true;
}

const bool Zone::onRaidInviteDecline(Character* pCharacter) {
	return true;
}

const bool Zone::onRequestTitles(Character* pCharacter) {
	if (!pCharacter) return false;

	auto titles = mTitleManager->getTitles(pCharacter);
	auto packet = Payload::makeTitleList(titles);
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;

	return true;
}

const bool Zone::onSetTitle(Character* pCharacter, const u32 pTitleID) {
	using namespace Payload::Zone;
	if (!pCharacter) return false;
	// TODO: Check eligibility

	String prefix = "";

	// NOTE: Where pTitleID = 0 the player has pressed the 'Clear Title' button.
	if (pTitleID != 0)
		prefix = mTitleManager->getPrefix(pTitleID);

	// Update Character.
	pCharacter->setTitle(prefix);

	// Update Zone.
	auto packet = TitleUpdate::construct(TitleUpdateAction::Title, pCharacter->getSpawnID(), pCharacter->getTitle());
	sendToVisible(pCharacter, packet, true);
	delete packet;
	
	return true;
}

const bool Zone::onSetSuffix(Character* pCharacter, const u32 pSuffixID) {
	using namespace Payload::Zone;
	if (!pCharacter) return false;
	// TODO: Check eligibility

	String suffix = "";

	// NOTE: Where pSuffixID = 0 the player has pressed the 'Clear Title' button.
	if (pSuffixID != 0)
		suffix = mTitleManager->getSuffix(pSuffixID);

	// Update Character.
	pCharacter->setSuffix(suffix);

	// Update Zone.
	auto packet = TitleUpdate::construct(TitleUpdateAction::Suffix, pCharacter->getSpawnID(), pCharacter->getSuffix());
	sendToVisible(pCharacter, packet, true);
	delete packet;

	return true;
}

const bool Zone::onSizeChange(Actor * pActor, float pSize) {
	using namespace Payload::Zone;
	if (!pActor) return false;

	pActor->setSize(pSize);
	
	// Update Zone.
	auto packet = ActorSize::construct(pActor->getSpawnID(), pSize);
	sendToVisible(pActor, packet, true);
	delete packet;

	return true;
}

const bool Zone::onMoveItem(Character* pCharacter, const u32 pFromSlot, const u32 pToSlot, const u32 pStacks) {
	if (!pCharacter) return false;

	// Check: Character is trying to move an Item FROM a trade slot.
	// Note: This is never allowed under any circumstances.
	if (SlotID::isTrade(pFromSlot)) {
		mLog->error(pCharacter->getName() + " attempted to move Item from trade slot.");
		return false;
	}

	auto inventory = pCharacter->getInventory();
	Item* wornNew = nullptr; // Item that will be equipped after move.
	Item* wornOld = nullptr; // Item that will be unequipped after move.

	const bool equipItem = SlotID::isWorn(pToSlot);
	const bool unequipItem = SlotID::isWorn(pFromSlot);
	const bool tradeMove = SlotID::isTrade(pToSlot);

	// Check: Character is trying to equip an Item.
	if (equipItem) {
		// Check: The Item being equipped is coming from the cursor.
		if (!SlotID::isCursor(pFromSlot)) {
			mLog->error(pCharacter->getName() + " attempted to equip Item from non cursor slot.");
			return false;
		}

		// Find the Item being equipped.
		wornNew = inventory->get(pFromSlot);
		if (!wornNew) {
			mLog->error(pCharacter->getName() + " attempted to equip null Item.");
			return false;
		}

		// Check: Character can equip this Item.
		if (!pCharacter->canEquip(wornNew, pToSlot)) {
			mLog->error(pCharacter->getName() + " attempted to equip Item they are not allowed to.");
			return false;
		}

		// NOTE: Can be null if there was no Item in the slot.
		wornOld = inventory->get(pToSlot);
	}
	// Check: Character is trying to unequip an Item.
	else if (unequipItem) {
		// Check: The Item being unequipped is going to the cursor.
		if (!SlotID::isCursor(pToSlot)) {
			mLog->error(pCharacter->getName() + " attempted to unequip Item to non cursor slot.");
			return false;
		}

		// Find the Item being unequipped.
		wornOld = inventory->get(pFromSlot);
		if (!wornOld) {
			mLog->error(pCharacter->getName() + " attempted to unequip null Item.");
			return false;
		}

		// NOTE: Can be null if there is no Item on cursor.
		wornNew = inventory->get(pToSlot);
	}

	// Character is trying to put an Item into a trade slot.
	if (tradeMove) {
		// Check: Character is currently trading.
		if (!pCharacter->isTrading()) {
			mLog->error(pCharacter->getName() + " attempted to move Item to trade slot but they are not trading.");
			return false;
		}

		// Check: The Item being moved is coming from the cursor.
		if (!SlotID::isCursor(pFromSlot)) {
			mLog->error(pCharacter->getName() + " attempted to move Item to trade slot from non cursor slot.");
			return false;
		}

		// Check: Valid Item being moved into trade.
		auto item = inventory->get(pFromSlot);
		if (!item) {
			mLog->error(pCharacter->getName() + " attempted to move null Item to trade slot.");
			return false;
		}
		
		// Check: No existing Item
		auto existingItem = inventory->get(pToSlot);
		if (existingItem) {
			mLog->error(pCharacter->getName() + " attempted to move Item to trade slot but the slot is not empty.");
			return false;
		}

		// Check: When trading with an NPC, a Character can only move a single stack.
		if (pCharacter->isTradingWithNPC() && pStacks > 0) {
			mLog->error(pCharacter->getName() + " attempted to move stacked Item to trade slot while trading with NPC.");
			return false;
		}

		if (pCharacter->isTradingWithCharacter()) {
			// Moving an Item into trade resets 'accepted'.
			pCharacter->setTradeAccepted(false);
		}
	}

	// Notify Inventory.
	if (!inventory->moveItem(pFromSlot, pToSlot, pStacks)) {
		mLog->error(pCharacter->getName() + " | MoveItem failed.");
		return false;
	}

	// Trading with Character.
	if (tradeMove && pCharacter->isTradingWithCharacter()) {
		auto item = inventory->get(pToSlot);
		auto tradingWith = Actor::cast<Character*>(pCharacter->getTradingWith());

		// Note: This makes the Item appear in the 'Trade Window' for the Character that pCharacter is trading with.
		tradingWith->getConnection()->sendItemTradeCharacter(item);
	}

	// Notify Character that a worn slot has changed.
	if (equipItem) {
		pCharacter->onWornSlotChange(pToSlot, wornOld, wornNew);
	}
	else if (unequipItem) {
		pCharacter->onWornSlotChange(pFromSlot, wornOld, wornNew);
	}

	return true;
}

const bool Zone::onMoveCurrency(Character* pCharacter, const u32 pFromSlot, const u32 pToSlot, const u32 pFromType, const u32 pToType, const i32 pAmount) {
	if (!pCharacter) return false;

	// Check: From slot ID is valid.
	if (!Limits::General::moneySlotIDValid(pFromSlot)) {
		mLog->error(pCharacter->getName() + " sent invalid FROM currency slot ID: " + toString(pFromSlot));
		return false;
	}

	// Check: From slot ID is not 'trade'
	if (CurrencySlot::isTrade(pFromSlot)) {
		// UF does not move currency from trade slots.
		mLog->error(pCharacter->getName() + " attempted to move currency from trade slot ID: " + toString(pFromSlot));
		// [CHEAT]
		return false;
	}

	// Check: Character is trading if moving currency into 'trade' slot.
	if (CurrencySlot::isTrade(pToSlot) && !pCharacter->isTrading()) {
		mLog->error(pCharacter->getName() + " attempted to move currency from trade slot but they are not trading.");
		return false;
	}

	// Check: To slot ID is valid.
	if (!Limits::General::moneySlotIDValid(pToSlot)) {
		mLog->error(pCharacter->getName() + " sent invalid TO currency slot ID: " + toString(pToSlot));
		return false;
	}

	// Check: From currency type is valid.
	if (!Limits::General::moneyTypeValid(pFromType)) {
		mLog->error(pCharacter->getName() + " sent invalid currency type: " + toString(pFromType));
		return false;
	}

	// Check: To currency type is valid.
	if (!Limits::General::moneyTypeValid(pToType)) {
		mLog->error(pCharacter->getName() + " sent invalid currency type: " + toString(pToType));
		return false;
	}

	// Check: Amount is greater than zero.
	if (pAmount <= 0) {
		mLog->error(pCharacter->getName() + " sent invalid currency amount: " + toString(pAmount));
		return false;
	}

	// Check: Currency moving into Shared Bank is platinum only.
	if (CurrencySlot::isSharedBank(pToSlot) && pToType != CurrencyType::Platinum) {
		mLog->error(pCharacter->getName() + " attempted to move non-platinum currency type: " + toString(pToType) + " into shared bank.");
		return false;
	}

	// Check: Currency moving from Shared Bank is platinum only.
	if (CurrencySlot::isSharedBank(pFromSlot) && pFromType != CurrencyType::Platinum) {
		mLog->error(pCharacter->getName() + " attempted to move non-platinum currency type: " + toString(pToType) + " from shared bank.");
		return false;
	}

	auto isBankingSlot = [](const u32 pSlot) { return pSlot == CurrencySlot::Bank || pSlot == CurrencySlot::SharedBank; };
	const bool isConversion = pFromType != pToType;
	const bool isBankSlot = isBankingSlot(pFromSlot) || isBankingSlot(pToSlot);
	const bool bankRequired = isConversion || isBankSlot;

	// Check: Character is in range of a bank.
	if (bankRequired && !canBank(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to move currency that requires a bank and they are out of range.");
		// [CHEAT]
		return false;
	}

	// Notify Inventory.
	if (!pCharacter->getInventory()->moveCurrency(pFromSlot, pToSlot, pFromType, pToType, pAmount)) {
		return false;
	}

	// Notify TradeHandler.
	if (CurrencySlot::isTrade(pToSlot) && pCharacter->isTradingWithCharacter())
		mTradeHandler->onMoveCurrency(pCharacter, pToSlot, pAmount);

	return true;
}

const bool Zone::onWearChange(Character* pCharacter, const u32 pMaterialID, u32 pEliteMaterialID, u32 pColour, u8 pSlot) {
	using namespace Payload::Zone;
	if (!pCharacter) return false;

	auto packet = WearChange::construct(pCharacter->getSpawnID(), pMaterialID, pEliteMaterialID, pColour, pSlot);
	sendToVisible(pCharacter, packet, false);
	delete packet;

	return true;
}

const bool Zone::giveStackableItem(Character* pCharacter, Item* pItem, const String& pReason) {
	if (!pCharacter) return false;
	if (!pItem) return false;

	auto inventory = pCharacter->getInventory();

	// Iterate until all the stacks in pItem have been added to the Inventory.
	while (pItem->getStacks() != 0) {
		// Find a partial stack to add to.
		auto mergeItem = inventory->findPartialStack(pItem->getID());

		// Partial stack found - merge stacks into that Item.
		if (mergeItem) {
			const auto emptyStacks = mergeItem->getEmptyStacks();

			// Item can take all the remaining stacks.
			if (emptyStacks >= pItem->getStacks()) {
				mergeItem->addStacks(pItem->getStacks());
				delete pItem;

				pCharacter->getConnection()->sendItemTrade(mergeItem);		

				return true;
			}
			// Item can take some of the remaining stacks.
			else {
				// Move stacks from one Item to the other.
				pItem->removeStacks(emptyStacks);
				mergeItem->addStacks(emptyStacks); // mergeItem should be at full stacks now.

				pCharacter->getConnection()->sendItemTrade(mergeItem);

				continue; // Still more stacks to place in Inventory.
			}
		}
		// No partial stack found.
		else {
			// Try to find empty slot
			auto slotID = inventory->findEmptySlot(pItem);

			// No empty slot was found, push to cursor.
			if (SlotID::isCursor(slotID)) {
				inventory->pushCursor(pItem);
				pCharacter->getConnection()->sendItemSummon(pItem);

				return true;
			}
			// Move to empty slot.
			else {
				inventory->put(pItem, slotID, pReason);
				pCharacter->getConnection()->sendItemTrade(pItem);

				if (pItem->isContainer())
					pItem->updateContentsSlots();

				return true;
			}
		}
	}

	return true;
}

const bool Zone::giveItem(Character* pCharacter, Item* pItem, const String& pReason) {
	if (!pCharacter) return false;
	if (!pItem) return false;

	auto inventory = pCharacter->getInventory();

	// Handle: Stackable Item, these are much more complex.
	if (pItem->isStackable()) {
		if (!giveStackableItem(pCharacter, pItem, pReason)) {
			return false;
			//mLog->error("Failed to return trade item to ");
		}

		return true;
	}

	// Handle: Non-stackable Item.
	const auto slotID = pCharacter->getInventory()->findEmptySlot(pItem);

	// No empty slot was found, push to cursor.
	if (SlotID::isCursor(slotID)) {
		inventory->pushCursor(pItem);
		pCharacter->getConnection()->sendItemSummon(pItem);
	}
	// Move to empty slot.
	else {
		pCharacter->getInventory()->put(pItem, slotID, pReason);
		pCharacter->getConnection()->sendItemTrade(pItem);
	}

	return true;
}

const bool Zone::giveItems(Character* pCharacter, std::list<Item*>& pItems, const String& pReason) {
	if (!pCharacter) return false;
	if (pItems.empty()) return false;

	for (auto i : pItems) {
		if (!giveItem(pCharacter, i, pReason)){
			return false;
		}
	}

	return true;
}

const bool Zone::onCampBegin(Character* pCharacter) {
	if (!pCharacter) return false;

	// Cancel trade if a Character starts camping.
	if (pCharacter->isTrading()) {
		onTradeCancel(pCharacter, pCharacter->getSpawnID());
	}

	pCharacter->startCamp();

	// Instant camp for GM.
	if (pCharacter->isGM()) {
		pCharacter->setCampComplete(true);
		auto connection = pCharacter->getConnection();

		connection->sendPreLogOutReply();
		connection->sendLogOutReply();
		connection->dropConnection();
		return true;;
	}

	return true;
}
