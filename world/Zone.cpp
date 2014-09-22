#include "Zone.h"
#include "ZoneData.h"
#include "World.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "GroupManager.h"
#include "GuildManager.h"
#include "RaidManager.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "Constants.h"
#include "DataStore.h"
#include "SpellDataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "../common/eq_packet_structs.h"
#include "LogSystem.h"
#include "Scene.h"

Zone::Zone(const uint32 pPort, const ZoneID pZoneID, const InstanceID pInstanceID) :
	mPort(pPort),
	mID(pZoneID),
	mInstanceID(pInstanceID)
{
}

Zone::~Zone() {
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
	safe_delete(mScene);
}

bool Zone::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(ZoneStream);
	EXPECTED_BOOL(mStreamFactory->Open(mPort));

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	EXPECTED_BOOL(ZoneData::getInstance().getLongNameStringID(mID, mLongNameStringID));
	EXPECTED_BOOL(ZoneData::getInstance().getLongName(mID, mLongName));
	EXPECTED_BOOL(ZoneData::getInstance().getShortName(mID, mShortName));

	mScene = new Scene(this);

	mInitialised = true;
	return true;
}

void Zone::addAuthentication(ClientAuthentication& pAuthentication, String pCharacterName) {
	mAuthenticatedCharacters.insert(std::make_pair(pCharacterName, pAuthentication));
}

void Zone::removeAuthentication(String pCharacterName) {
	mAuthenticatedCharacters.erase(pCharacterName);
}

bool Zone::checkAuthentication(String pCharacterName) {
	for (auto i : mAuthenticatedCharacters) {
		if (i.first == pCharacterName) {
			Log::info("[Zone] Authentication Passed");
			return true;
		}
	}

	Log::error("[Zone] checkAuthentication is returning false. This is unexpected.");
	return false;
}

bool Zone::getAuthentication(String pCharacterName, ClientAuthentication& pAuthentication) {
	for (auto i : mAuthenticatedCharacters) {
		if (i.first == pCharacterName) {
			pAuthentication = i.second;
			return true;
		}
	}

	return false;
}


void Zone::update() {
	// Check if any new clients are connecting to this Zone.
	_handleIncomingConnections();

	_updatePreConnections();
	_updateConnections();
	_updateCharacters();

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
	auto outPacket = new EQApplicationPacket(OP_ClientUpdate, pActor->getPositionData(), 22); // sizeof(PlayerPositionUpdateServer_Struct)
	for (auto i : pActor->getVisibleTo()) {
		if (i->isLinkDead() == false)
			i->getConnection()->sendPacket(outPacket);
	}
	outPacket->pBuffer = nullptr;
	safe_delete(outPacket);
}


void Zone::handleAFK(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::AFK, pCharacter->isAFK()); }
void Zone::handleShowHelm(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::ShowHelm, pCharacter->getShowHelm()); }
void Zone::handleAnonymous(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::Anonymous, pCharacter->getAnonymous()); }
void Zone::handleStanding(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_Animation, SpawnAppearanceAnimation::Standing); }
void Zone::handleSitting(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_Animation, SpawnAppearanceAnimation::Sitting); }
void Zone::handleCrouching(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_Animation, SpawnAppearanceAnimation::Crouch); }
void Zone::notifyCharacterGM(Character* pCharacter){ _sendSpawnAppearance(pCharacter, SpawnAppearanceType::GM, pCharacter->getIsGM() ? 1 : 0, true); }
void Zone::handleLinkDead(Character* pCharacter) { _sendSpawnAppearance(pCharacter, SpawnAppearanceType::LinkDead, 1, false); }

void Zone::_sendSpawnAppearance(Character* pCharacter, SpawnAppearanceType pType, uint32 pParameter, bool pIncludeSender) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	auto outPacket = new EQApplicationPacket(OP_SpawnAppearance, SpawnAppearance::size());
	auto payload = SpawnAppearance::convert(outPacket->pBuffer);
	payload->mSpawnID = pCharacter->getSpawnID();
	payload->mType = pType;
	payload->mParameter = pParameter;

	sendToVisible(pCharacter, outPacket, pIncludeSender);
	safe_delete(outPacket);
}

void Zone::notifyCharacterChatSay(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_SAY, pMessage);
}

void Zone::notifyCharacterChatShout(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_SHOUT, pMessage);
}

void Zone::notifyCharacterChatAuction(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_AUCTION, pMessage);

	// TODO: Server Auction
}

void Zone::notifyCharacterChatOOC(Character* pCharacter, const String pMessage) {
	_sendChat(pCharacter, ChannelID::CH_OOC, pMessage);
	
	// TODO: Server OOC
}

void Zone::notifyCharacterEmote(Character* pCharacter, const String pMessage) {
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

void Zone::notifyCharacterChatTell(Character* pCharacter, const String& pTargetName, const String& pMessage) {
	ZoneManager::getInstance().notifyCharacterChatTell(pCharacter, pTargetName, pMessage);
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

void Zone::notifyCharacterAnimation(Character* pCharacter, uint8 pAction, uint8 pAnimationID, bool pIncludeSender) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	auto outPacket = new EQApplicationPacket(OP_Animation, Payload::Zone::Animation::size());
	auto payload = Payload::Zone::Animation::convert(outPacket->pBuffer);
	payload->mSpawnID = pCharacter->getSpawnID();
	payload->mAction = pAction;
	payload->mValue = pAnimationID;

	sendToVisible(pCharacter, outPacket, pIncludeSender);
	safe_delete(outPacket);
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

void Zone::notifyCharacterZoneChange(Character* pCharacter, ZoneID pZoneID, uint16 pInstanceID) {
	// TODO: Are we expecting this character to zone out?

	ZoneManager::getInstance().registerZoneTransfer(pCharacter, pZoneID, pInstanceID);
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
	ARG_PTR_CHECK(pCharacter);

	_sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_GuildID, pCharacter->getGuildID(), true);
	_sendSpawnAppearance(pCharacter, SpawnAppearanceType::SA_GuildRank, pCharacter->getGuildRank(), true);
}

void Zone::_onLeaveZone(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);

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
	ARG_PTR_CHECK(pCharacter);

	if (pCharacter->hasGuild())
		GuildManager::getInstance().onCamp(pCharacter);

	if (pCharacter->hasGroup())
		GroupManager::getInstance().onCamp(pCharacter);

	if (pCharacter->hasRaid())
		RaidManager::getInstance().onCamp(pCharacter);
}

void Zone::_onLinkdead(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);

	pCharacter->setLinkDead();

	// Tidy up Character
	mCharacters.remove(pCharacter); // Remove from active Character list.
	mActors.remove(pCharacter);
	pCharacter->setConnection(nullptr); // Update Character(ZCC) pointer.

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

void Zone::handleTarget(Character* pCharacter, SpawnID pSpawnID) {
	EXPECTED(pCharacter);

	// Character is clearing their target.
	if (pSpawnID == NO_TARGET) {
		pCharacter->setTarget(nullptr);
		return;
	}

	Actor* actor = findActor(pSpawnID);
	EXPECTED(actor);
	pCharacter->setTarget(actor);
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

	Log::info(pCharacter->getName() + " can now see " + pAddActor->getName());

	pAddActor->_syncPosition();
	auto outPacket = new EQApplicationPacket(OP_NewSpawn, pAddActor->getActorData(), sizeof(Payload::SpawnData));
	pCharacter->getConnection()->sendPacket(outPacket);
	outPacket->pBuffer = nullptr;
	safe_delete(outPacket);
}

void Zone::handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor) {
	EXPECTED(pCharacter);
	EXPECTED(pRemoveActor);

	Log::info(pCharacter->getName() + " can no longer see " + pRemoveActor->getName());

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
}

void Zone::removeActor(Actor* pActor) {

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

void Zone::handleTitleChanged(Character* pCharacter, TitleOption pOption) {
	using namespace Payload::Zone;
	EXPECTED(pCharacter);

	auto outPacket = new EQApplicationPacket(OP_SetTitleReply, TitleUpdate::size());
	auto payload = TitleUpdate::convert(outPacket->pBuffer);

	payload->mSpawnID = pCharacter->getSpawnID();

	if (pOption == TitleOption::TO_Title) {
		payload->mOption = TitleUpdate::UPDATE_PREFIX;
		strcpy(payload->mTitle, pCharacter->getPrefix().c_str());

	}
	else {
		payload->mOption = TitleUpdate::UPDATE_SUFFIX;
		strcpy(payload->mTitle, pCharacter->getSuffix().c_str());
	}

	// Update Character + those visible to
	sendToVisible(pCharacter, outPacket, true);
	safe_delete(outPacket);
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
	EXPECTED(pPacket);

	// Update anyone who can see pActor.
	for (auto i : pActor->getVisibleTo())
		i->getConnection()->sendPacket(pPacket);
}
