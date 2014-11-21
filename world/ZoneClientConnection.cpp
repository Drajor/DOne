#include "ZoneClientConnection.h"
#include "GuildManager.h"
#include "GroupManager.h"
#include "RaidManager.h"
#include "TitleManager.h"
#include "Zone.h"
#include "ZoneManager.h"
#include "ZoneData.h"
#include "Character.h"
#include "Inventory.h"
#include "LogSystem.h"
#include "DataStore.h"
#include "CommandHandler.h"
#include "../common/EQStreamIntf.h"
#include "../common/emu_opcodes.h"
#include "../common/EQPacket.h"
#include "../common/eq_packet_structs.h"
#include "Utility.h"
#include "Limits.h"
#include "Payload.h"
#include "Item.h"
#include "ItemDataStore.h"
#include "ItemFactory.h"
#include "Transmutation.h"

#include "../common/MiscFunctions.h"

/*
Known GM Commands
/goto
/kill
/summon
/emotezone
*/
//ZoneClientConnection::mGroupUpdatePacket = nullptr;

EQApplicationPacket* ZoneClientConnection::mPlayerProfilePacket = nullptr;
EQApplicationPacket* ZoneClientConnection::mGroupJoinPacket = nullptr;
EQApplicationPacket* ZoneClientConnection::mGroupLeavePacket = nullptr;
EQApplicationPacket* ZoneClientConnection::mGroupDisbandPacket = nullptr;
EQApplicationPacket* ZoneClientConnection::mGroupLeaderChangePacket = nullptr;
EQApplicationPacket* ZoneClientConnection::mGroupUpdateMembersPacket = nullptr;

ZoneClientConnection::ZoneClientConnection(EQStreamInterface* pStreamInterface, Zone* pZone) :
mStreamInterface(pStreamInterface),
mZone(pZone),
mCharacter(nullptr),
mCommandHandler(nullptr),
mZoneConnectionStatus(ZoneConnectionStatus::NONE),
mConnected(true),
mConnectionOrigin(ConnectionOrigin::Unknown)
{
	mCommandHandler = new CommandHandler();
	mCommandHandler->initialise();
	mForceSendPositionTimer.Disable();
}

ZoneClientConnection::~ZoneClientConnection() {
	dropConnection();
	mStreamInterface->ReleaseFromUse();
	// NOTE: mStreamInterface is intentionally not deleted here.
	safe_delete(mCommandHandler);
}

void ZoneClientConnection::initalise() {
	mPlayerProfilePacket = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));
	mGroupJoinPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	mGroupLeavePacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	mGroupDisbandPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
	mGroupLeaderChangePacket = new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));
	mGroupUpdateMembersPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
}

void ZoneClientConnection::deinitialise() {
	safe_delete(mPlayerProfilePacket);
	safe_delete(mGroupJoinPacket);
	safe_delete(mGroupLeavePacket);
	safe_delete(mGroupDisbandPacket);
	safe_delete(mGroupLeaderChangePacket);
	safe_delete(mGroupUpdateMembersPacket);
}

bool ZoneClientConnection::isConnected() {
	return mConnected && mStreamInterface->CheckState(ESTABLISHED);
}


void ZoneClientConnection::update() {
	if (!mConnected || !mStreamInterface->CheckState(ESTABLISHED)) {
		if (mCharacter) {
			//mZone->notifyCharacterLinkDead(mCharacter);
		}
		else {
			Log::error("[Zone Client Connection] Disconnected before Character assignment.");
		}
		return;
	}

	// [UF] When a character has been standing still for too long they disappear. Here we force send the position to stop that.
	if (mForceSendPositionTimer.Check()) {
		mZone->handleActorPositionChange(mCharacter);
	}

	EQApplicationPacket* packet = 0;
	bool ret = true;
	while (ret && mConnected && (packet = (EQApplicationPacket*)mStreamInterface->PopPacket())) {
		ret = _handlePacket(packet);
		delete packet;
	}
}

void ZoneClientConnection::dropConnection() {
	mConnected = false;
	mStreamInterface->Close();
}

bool ZoneClientConnection::_handlePacket(const EQApplicationPacket* pPacket) {
	EXPECTED_BOOL(pPacket);

	if (!mStreamInterface->CheckState(ESTABLISHED)) return false;

	EmuOpcode opcode = pPacket->GetOpcode();
	if (/*opcode == 0 || */opcode == OP_FloatListThing) return true;

	switch (opcode) {
	case OP_Unknown:
		_handleUnknown(pPacket);
		break;
	case OP_AckPacket:
		// Ignore.
		break;
	case OP_ZoneEntry:
		_handleZoneEntry(pPacket);
		break;
	case OP_ReqClientSpawn:
		_handleRequestClientSpawn(pPacket);
		break;
	case OP_SetServerFilter:
		_handleSetServerFiler(pPacket);
		break;
	case OP_SendAATable:
		_handleSendAATable(pPacket);
		break;
	case OP_SendExpZonein:
		Utility::print("[UNHANDLED OP_SendExpZonein]");
		break;
	case OP_WorldObjectsSent:
		// Ignore. Because I don't understand what this is for.
		break;
	case OP_ZoneComplete:
		Utility::print("[UNHANDLED OP_ZoneComplete]");
		break;
	case OP_ReqNewZone:
		// UF sends this but still works when there is no reply.
		_handleRequestNewZoneData(pPacket);
		break;
	case OP_SpawnAppearance:
		_handleSpawnAppearance(pPacket);
		break;
	case OP_WearChange:
		// Ignore.
		break;
	case OP_ClientUpdate:
		// NOTE: Sent when a Character moves
		// NOTE: Sent automatically every X seconds by the client.
		_handleClientUpdate(pPacket);
		break;
	case OP_ClientError:
		Utility::print("[UNHANDLED OP_ClientError]");
		break;
	case OP_ApproveZone:
		Utility::print("[UNHANDLED OP_ApproveZone]");
		break;
	case OP_TGB:
		/*
		/tgb is a little strange
		When a player enters /tgb on two packets are sent
			- first 0 or 1 for on and off
			- then 2 for some reason.
		*/
		_handleTGB(pPacket);
		break;
	case OP_SendTributes:
		// Ignore. Tribute system later.
		break;
	case OP_SendGuildTributes:
		Utility::print("[UNHANDLED OP_SendGuildTributes]");
		break;
	case OP_SendAAStats:
		Utility::print("[UNHANDLED OP_SendAAStats]");
		break;
	case OP_ClientReady:
		_handleClientReady(pPacket);
		break;
	case OP_UpdateAA:
		_handleUpdateAA(pPacket);
		break;
	case OP_BlockedBuffs:
		// Ignore. Not Implemented.
		break;
	case OP_XTargetRequest:
		Utility::print("[UNHANDLED OP_XTargetRequest]");
		break;
	case OP_XTargetAutoAddHaters:
		Utility::print("[UNHANDLED OP_XTargetAutoAddHaters]");
		break;
	case OP_GetGuildsList:
		Utility::print("[UNHANDLED OP_GetGuildsList]");
		break;
	case OP_TargetMouse:
	case OP_TargetCommand:
		// NOTE: This is sent when the current target dies.
		_handleTarget(pPacket);
		break;
	case OP_Camp:
		// Sent when user types /camp or presses the camp button.
		_handleCamp(pPacket);
		break;
	case OP_Logout:
		// This occurs 30 seconds after /camp
		_handleLogOut(pPacket);
		return false;
	case OP_DeleteSpawn:
		// Client sends this after /camp
		// NOTE: Sent as a Character is about to zone out.
		_handleDeleteSpawn(pPacket);
		break;
	case OP_ChannelMessage:
		_handleChannelMessage(pPacket);
		break;
	case OP_Jump:
		// Ignore.
		break;
	case OP_BazaarSearch:
		// Ignore.
		break;
		// Ignore all mercenary packets.
	case OP_MercenaryDataRequest:
	case OP_MercenaryDataResponse:
	case OP_MercenaryHire:
	case OP_MercenaryUnknown1:
	case OP_MercenaryTimer:
	case OP_MercenaryAssign:
	case OP_MercenaryDataUpdate:
	case OP_MercenaryCommand:
	case OP_MercenarySuspendRequest:
	case OP_MercenarySuspendResponse:
	case OP_MercenaryUnsuspendResponse:
	case OP_MercenaryDataUpdateRequest:
	case OP_MercenaryDismiss:
	case OP_MercenaryTimerRequest:
		break;
	case OP_Emote:
		_handleEmote(pPacket);
		break;
	case OP_Animation:
		_handleAnimation(pPacket);
		break;
	case OP_Save:
		Utility::print("[UNHANDLED OP_Save]");
		break;
	case OP_SaveOnZoneReq:
		Utility::print("[UNHANDLED OP_SaveOnZoneReq]");
		break;
	case OP_FaceChange:
		// NOTE: This occurs when the player presses 'Accept' in the Face interface.
		_handleFaceChange(pPacket);
		break;
	case OP_WhoAllRequest:
		_handleWhoAllRequest(pPacket);
	case OP_GroupInvite:
		// NOTE: This occurs when the player presses 'Invite' on the group window.
		// NOTE: This also occurs when the player uses the /invite command.
		// NOTE: This also occurs when using the 'Invite/follow' mapped command (Options->Keys).
		_handleGroupInvite(pPacket);
		break;
	case OP_GroupInvite2:
		Utility::print("[UNHANDLED OP_GroupInvite2]");
		break;
	case OP_GroupFollow:
		// NOTE: This occurs when the player presses 'Follow' on the group window.
		// NOTE: This also occurs when the player uses the /invite command when they have a current group invitation.
		// NOTE: This also occurs when using the 'Invite/follow' mapped command (Options->Keys).
		_handleGroupFollow(pPacket);
		break;
	case OP_GroupFollow2:
		Utility::print("[UNHANDLED OP_GroupFollow2]");
		break;
	case OP_GroupCancelInvite:
		// NOTE: This occurs when the player presses 'Decline' on the group window.
		// NOTE: This also occurs when the player uses the /disband command when they have a current group invitation.
		// NOTE: This also occurs when using the 'Disband' mapped command (Options->Keys).
		_handleGroupCanelInvite(pPacket);
		break;
	case OP_GroupDisband:
		// NOTE: This occurs when the player presses 'Disband' on group window.
		// NOTE: This occurs when the player uses the /disband command.
		_handleGroupDisband(pPacket);
		break;
	case OP_GroupMakeLeader:
		// NOTE: This occurs when the player uses the /makeleader command.
		// NOTE: This occurs when the player uses the context menu on the group window (Roles->Leader).
		_handleGroupMakeLeader(pPacket);
		break;
	case OP_GuildCreate:
		// NOTE: This occurs when the player uses the /guildcreate command.
		_handleGuildCreate(pPacket);
		break;
	case OP_GuildDelete:
		// Note: This occurs when the player uses the /guilddelete command.
		_handleGuildDelete(pPacket);
		break;
	case OP_GuildInvite:
		// NOTE: This occurs when a player uses the /guildinvite command.
		_handleGuildInvite(pPacket);
	case OP_GuildInviteAccept:
		// NOTE: This occurs when a player presses 'accept' on the guild invite window.
		_handleGuildInviteAccept(pPacket);
		break;
	case OP_GuildRemove:
		// NOTE: This occurs when the player uses the /guildremove command.
		_handleGuildRemove(pPacket);
		break;
	case OP_SetGuildMOTD:
		// NOTE: This occurs when the player uses the /guildmotd command.
		_handleSetGuildMOTD(pPacket);
		break;
	case OP_GetGuildMOTD:
		// NOTE: This occurs when the player uses the /getguildmotd command.
		_handleGetGuildMOTD(pPacket);
		break;
	case OP_GuildUpdateURLAndChannel:
		// NOTE: This occurs via the guild window.
		_handleSetGuildURLOrChannel(pPacket);
		break;
	case OP_GuildPublicNote:
		// NOTE: This occurs via the guild window.
		_handleSetGuildPublicNote(pPacket);
		break;
	case OP_GuildStatus:
		// NOTE: This occurs when the player uses the /guildstatus command.
		_handleGetGuildStatus(pPacket);
		break;
	case OP_GuildDemote:
		_handleGuildDemote(pPacket);
		break;
	case OP_GuildManageBanker:
		_handleGuildBanker(pPacket);
		break;
	case OP_GuildLeader:
		_handleGuildMakeLeader(pPacket);
		break;
	case OP_GuildWar:
		// NOT IMPLEMENTED
		_unimplementedFeature("OP_GuildWar");
		break;
	case OP_GuildPeace:
		// NOT IMPLEMENTED
		_unimplementedFeature("OP_GuildPeace");
		break;
	case OP_ZoneChange:
		Utility::print("[GOT OP_ZoneChange]");
		_handleZoneChange(pPacket);
		break;
	case OP_AutoAttack:
		_handleAutoAttack(pPacket);
		break;
	case OP_AutoAttack2:
		// Ignore. This sent directly after OP_AutoAttack. I don't know why.
		break;
		// Spells Begin
	case OP_MemorizeSpell:
		_handleMemoriseSpell(pPacket);
		break;
	case OP_DeleteSpell:
		_handleDeleteSpell(pPacket);
		break;
	case OP_LoadSpellSet:
		_handleLoadSpellSet(pPacket);
		break;
	case OP_SwapSpell:
		_handleSwapSpell(pPacket);
		break;
	case OP_CastSpell:
		_handleCastSpell(pPacket);
		break;
		// Spells End
	case OP_CombatAbility:
		_handleCombatAbility(pPacket);
		break;
	case OP_Taunt:
		_handleTaunt(pPacket);
		break;
	case OP_Consider:
		_handleConsider(pPacket);
		break;
	case OP_ConsiderCorpse:
		_handleConsiderCorpse(pPacket);
		break;
	case OP_Surname:
		// NOTE: This occurs when the player uses the /surname command.
		_handleSurname(pPacket);
		break;
	case OP_ClearSurname:
		// NOTE: This occurs when the player uses the /surname command with no text
		_handleClearSurname(pPacket);
		break;
	case OP_GMLastName:
		// NOTE: This occurs when the player uses the /lastname command.
		_handleGMLastName(pPacket);
		break;
	case OP_SetTitle:
		// NOTE: This occurs when the player presses the 'Change Title', 'Clear Title', 'Change Suffix' or 'Clear Suffix' buttons.
		_handleSetTitle(pPacket);
		break;
	case OP_RequestTitles:
		// NOTE: This occurs when the player opens the title window.
		_handleRequestTitles(pPacket);
		break;
	case OP_LootRequest:
		_handleBeginLootRequest(pPacket);
		break;
	case OP_EndLootRequest:
		_handleEndLootRequest(pPacket);
		break;
	case OP_LootItem:
		_handleLootItem(pPacket);
		break;
	case OP_MoveItem:
		_handleMoveItem(pPacket);
		break;
	case OP_Consume:
		_handleConsume(pPacket);
		break;
	case OP_ItemVerifyRequest:
		// NOTE: UF Requires a reply or the UI will lock up.
		// NOTE: This occurs when the player right clicks on any Item (except containers).
		_handleItemRightClick(pPacket);
		break;
	case OP_PotionBelt:
		_handlePotionBelt(pPacket);
		break;
	case OP_OpenContainer:
		// NOTE: This occurs when the player opens a container.
		_handleOpenContainer(pPacket);
		break;
	case OP_TradeRequest:
		// NOTE: This occurs when a player left clicks on an NPC with an Item on the cursor.
		_handleTradeRequest(pPacket);
		break;
	case OP_TradeRequestAck:
		_handleTradeRequestAck(pPacket);
		break;
	case OP_CancelTrade:
		// NOTE: This occurs when a player presses the 'Cancel' button whilst trading with an NPC.
		_handleCancelTrade(pPacket);
		break;
	case OP_TradeAcceptClick:
		// NOTE: This occurs when a player presses 'Give' button whilst trading with an NPC.
		_handleAcceptTrade(pPacket);
		break;
	case OP_TradeBusy:
		_handleTradeBusy(pPacket);
		break;
	case OP_ItemLinkClick:
		// Note: This occurs when a player clicks on an Item Link.
		_handleItemLinkClick(pPacket);
		break;
	case OP_ItemViewUnknown:
		_handleItemView(pPacket);
		break;
	case OP_MoveCoin:
		_handleMoveCoin(pPacket);
		break;
	case OP_CrystalCreate:
		// NOTE: This occurs when a player selects Radiant or Ebon Crystals from within the inventory window.
		_handleCrystalCreate(pPacket);
		break;
	case OP_CrystalReclaim:
		// NOTE: This occurs when a player clicks the 'Reclaim' button in the inventory window.
		// NOTE: No payload is sent (Size=0)
		_handleCrystalReclaim(pPacket);
		break;
	case OP_EvolvingItem:
		Log::error("OP_EvolvingItem");
		break;
	case OP_EnvDamage:
		_handleEnvironmentalDamage(pPacket);
		break;
	case OP_PopupResponse:
		_handlePopupResponse(pPacket);
		break;
	case OP_VetClaimRequest:
		// NOTE: This occurs when a player clicks the 'Refresh' button in the Claim Window.
		_handleClaimRequest(pPacket);
		break;
	case OP_AugmentItem:
		// NOTE: This occurs when the player presses 'Insert' on the Augmentation Sealer.
		_handleAugmentItem(pPacket);
		break;
	case OP_WeaponEquip1:
		Log::info("OP_WeaponEquip1 size= " + std::to_string(pPacket->size));
		break;
	case OP_WeaponEquip2:
		Log::info("OP_WeaponEquip2 size= " + std::to_string(pPacket->size));
		break;
	case OP_WeaponUnequip2:
		Log::info("OP_WeaponUnequip2 size= " + std::to_string(pPacket->size));
		break;
	case OP_AugmentInfo:
		_handleAugmentInfo(pPacket);
		break;
	case OP_ReadBook:
		_handleReadBook(pPacket);
		break;
	case OP_TradeSkillCombine:
		// NOTE: This occurs when a player presses the 'Combine' button on an opened container.
		_handleCombine(pPacket);
		break;
	case OP_ShopRequest:
		// NOTE: This occurs when a player right clicks on a merchant.
		_handleShopRequest(pPacket);
		break;
	case OP_ShopEnd:
		// NOTE: This occurs when a player presses the 'Done' button on the merchant window.
		// NOTE: This occurs when a player presses 'escape' whilst a merchant window is open.
		// NOTE: This occurs when a player moves their character too far away from a merchant.
		_handleShopEnd(pPacket);
		break;
	default:
		StringStream ss;
		ss << "Unknown Packet: " << opcode;
		Utility::print(ss.str());
		break;
	}
	return true;
}

void ZoneClientConnection::_handleZoneEntry(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mZoneConnectionStatus == ZoneConnectionStatus::NONE);
	EXPECTED(ZoneEntry::sizeCheck(pPacket->size));

	auto payload = ZoneEntry::convert(pPacket->pBuffer);
	String characterName = Utility::safeString(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(characterName));
	Log::info("Got ZoneEntry from " + characterName);

	mZoneConnectionStatus = ZoneConnectionStatus::ZoneEntryReceived;

	// Retrieve Character
	mCharacter = ZoneManager::getInstance().getZoningCharacter(characterName);
	EXPECTED(mCharacter);
	EXPECTED(ZoneManager::getInstance().removeZoningCharacter(mCharacter->getName()));

	// Check: Character authenticated for this Zone.
	EXPECTED(mZone->checkAuthentication(mCharacter));
	// Clear ZoneChange
	mCharacter->clearZoneChange();

	// If character is not initialised yet they are loading from the Character Selection Screen.
	if (mCharacter->isInitialised() == false) {
		EXPECTED(mCharacter->initialise());
	}

	mCharacter->setZone(mZone);
	mCharacter->setSpawnID(mZone->getNextSpawnID());
	mCharacter->setConnection(this);

	// REPLY
	_sendGuildNames();
	// OP_PlayerProfile
	_sendPlayerProfile();
	mZoneConnectionStatus = ZoneConnectionStatus::PlayerProfileSent;
	// OP_ZoneEntry
	_sendZoneEntry();
	// Bulk Spawns
	_sendZoneSpawns();
	// Corpses Bulk
	// PvP updates?
	// OP_TimeOfDay
	_sendTimeOfDay();
	// Tributes
	_sendTributeUpdate();
	// Bulk Inventory
	_sendInventory();
	// Cursor Items (only sent when some item is on the cursor)
	// Tasks
	// XTargets
	// Weather
	
	_sendWeather();
	mZoneConnectionStatus = ZoneConnectionStatus::ZoneInformationSent;
}

void ZoneClientConnection::_sendTimeOfDay() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
	auto payload = (TimeOfDay_Struct*)outPacket->pBuffer;
	memset(payload, 0, sizeof(TimeOfDay_Struct)); // TODO:
	outPacket->priority = 6; // TODO: Look into this.
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendPlayerProfile() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));
	auto payload = reinterpret_cast<PlayerProfile_Struct*>(outPacket->pBuffer);
	*payload = { 0 }; // Clear memory.

	// The entityid field in the Player Profile is used by the Client in relation to Group Leadership AA // TODO: How?

	strncpy(payload->name, mCharacter->getName().c_str(), Limits::Character::MAX_NAME_LENGTH);
	strncpy(payload->last_name, mCharacter->getLastName().c_str(), Limits::Character::MAX_LAST_NAME_LENGTH);
	payload->gender = mCharacter->getGender();
	payload->race = mCharacter->getRace();
	payload->class_ = mCharacter->getClass();
	payload->level = mCharacter->getLevel();
	//payload->binds[5];
	payload->deity = mCharacter->getDeity();
	payload->guild_id = mCharacter->getGuildID();
	//payload->birthday;			// characters bday
	//payload->lastlogin;			// last login or zone time
	//payload->timePlayedMin;		// in minutes
	//payload->pvp;
	//payload->level2;
	payload->mAnonymous = mCharacter->getAnonymous();
	payload->gm = mCharacter->isGM() ? 1 : 0;
	payload->guildrank = mCharacter->getGuildRank();
	//payload->guildbanker;
	//payload->intoxication;
	//payload->spellSlotRefresh[MAX_PP_MEMSPELL];	//in ms
	//payload->abilitySlotRefresh;
	payload->haircolor = mCharacter->getHairColour();
	payload->beardcolor = mCharacter->getBeardColour();
	payload->eyecolor1 = mCharacter->getLeftEyeColour();
	payload->eyecolor2 = mCharacter->getRightEyeColour();
	payload->hairstyle = mCharacter->getHairStyle();
	payload->beard = mCharacter->getBeardStyle();
	//payload->ability_time_seconds;
	//payload->ability_number;
	//payload->ability_time_minutes;
	//payload->ability_time_hours;
	//payload->item_material[_MaterialCount];
	//payload->item_tint[_MaterialCount];
	for (int i = 0; i < MAX_MATERIAL_SLOTS; i++) {
		//payload->item_tint[i].color = mCharacter->getColour(i).mColour;
		// TODO!
		payload->item_material[i] = mCharacter->getMaterial(i);
	}
	//payload->aa_array[MAX_PP_AA_ARRAY];
	//payload->servername[32];
	strncpy(payload->title, mCharacter->getTitle().c_str(), Limits::Character::MAX_TITLE_LENGTH);
	strncpy(payload->suffix, mCharacter->getSuffix().c_str(), Limits::Character::MAX_SUFFIX_LENGTH);
	//payload->guildid2;
	payload->exp = mCharacter->getExperience();
	payload->points = 0;
	payload->mana = mCharacter->getCurrentMana();
	payload->cur_hp = mCharacter->getHPPercent();
	payload->STR = mCharacter->getBaseStatistic(Statistic::Strength);
	payload->STA = mCharacter->getBaseStatistic(Statistic::Stamina);
	payload->CHA = mCharacter->getBaseStatistic(Statistic::Charisma);
	payload->DEX = mCharacter->getBaseStatistic(Statistic::Dexterity);
	payload->INT = mCharacter->getBaseStatistic(Statistic::Intelligence);
	payload->AGI = mCharacter->getBaseStatistic(Statistic::Agility);
	payload->WIS = mCharacter->getBaseStatistic(Statistic::Wisdom);
	payload->face = mCharacter->getFaceStyle();

	mCharacter->setSkill(Skills::Meditate, 1000);
	mCharacter->setSkill(Skills::SenseHeading, 1000);
	mCharacter->setSkill(Skills::Swimming, 1000);

	mCharacter->setLanguage(Languages::COMMON_TONGUE_LANG, 100);
	mCharacter->setLanguage(Languages::BARBARIAN_LANG, 100);

	// Copy skills into profile.
	for (int i = 0; i < Limits::Skills::MAX_ID; i++) {
		payload->skills[i] = mCharacter->getAdjustedSkill(i);
	}

	// Copy languages into profile.
	for (int i = 0; i < Limits::Languages::MAX_ID; i++) {
		payload->languages[i] = mCharacter->getLanguage(i);
	}

	// Copy spell book data into profile.
	if (mCharacter->isCaster()) {
		const std::vector<uint32> spellbook = mCharacter->getSpellBookData();
		for (auto i = 0; i < Limits::SpellBook::MAX_SLOTS; i++)
			payload->spell_book[i] = spellbook[i];
	}
	// Copy spell bar data into profile.
	if (mCharacter->isCaster()) {
		const std::vector<uint32> spellbar = mCharacter->getSpellBarData();
		for (auto i = 0; i < Limits::SpellBar::MAX_SLOTS; i++)
			payload->mem_spells[i] = spellbar[i];
	}
	payload->y = mCharacter->getY();
	payload->x = mCharacter->getX();
	payload->z = mCharacter->getZ();
	payload->heading = mCharacter->getHeading();

	// Personal Currency.
	payload->platinum = mCharacter->getInventory()->getPersonalPlatinum();
	payload->gold = mCharacter->getInventory()->getPersonalGold();
	payload->silver = mCharacter->getInventory()->getPersonalSilver();
	payload->copper = mCharacter->getInventory()->getPersonalCopper();

	// Bank Currency.
	payload->platinum_bank = mCharacter->getInventory()->getBankPlatinum();
	payload->gold_bank = mCharacter->getInventory()->getBankGold();
	payload->silver_bank = mCharacter->getInventory()->getBankSilver();
	payload->copper_bank = mCharacter->getInventory()->getBankCopper();

	// Cursor Currency
	payload->platinum_cursor = mCharacter->getInventory()->getCursorPlatinum();
	payload->gold_cursor = mCharacter->getInventory()->getCursorGold();
	payload->silver_cursor = mCharacter->getInventory()->getCursorSilver();
	payload->copper_cursor = mCharacter->getInventory()->getCursorCopper();

	// Shared Bank Platinum
	payload->platinum_shared = mCharacter->getInventory()->getSharedBankPlatinum();

	//payload->pvp2;				//
	//payload->pvptype;			//
	//payload->ability_down;		// Guessing
	//payload->autosplit;			//not used right now
	//payload->zone_change_count;
	payload->drakkin_heritage = mCharacter->getDrakkinHeritage();
	payload->drakkin_tattoo = mCharacter->getDrakkinTattoo();
	payload->drakkin_details = mCharacter->getDrakkinDetails();
	//payload->expansions = ;
	//payload->toxicity;			//from drinking potions, seems to increase by 3 each time you drink
	//payload->unknown5496[16];	//
	payload->hunger_level = 0;
	payload->thirst_level = 0;
	//payload->ability_up;
	payload->zone_id = mZone->getID();
	payload->zoneInstance = mZone->getInstanceID();
	//payload->buffs[BUFF_COUNT];	// Buffs currently on the player
	//payload->groupMembers[6][64];//
	//payload->entityid;
	//payload->leadAAActive;
	//payload->ldon_points_guk;	//client uses these as signed
	//payload->ldon_points_mir;
	//payload->ldon_points_mmc;
	//payload->ldon_points_ruj;
	//payload->ldon_points_tak;
	//payload->ldon_points_available;
	//payload->ldon_wins_guk;
	//payload->ldon_wins_mir;
	//payload->ldon_wins_mmc;
	//payload->ldon_wins_ruj;
	//payload->ldon_wins_tak;
	//payload->ldon_losses_guk;
	//payload->ldon_losses_mir;
	//payload->ldon_losses_mmc;
	//payload->ldon_losses_ruj;
	//payload->ldon_losses_tak;
	//payload->tribute_time_remaining;	//in miliseconds
	payload->showhelm = mCharacter->getShowHelm() ? 1 : 0;
	//payload->career_tribute_points;
	//payload->tribute_points;
	//payload->tribute_active;		//1=active
	//payload->tributes[MAX_PLAYER_TRIBUTES];
	//payload->disciplines;
	//payload->recastTimers[MAX_RECAST_TYPES];	// Timers (GMT of last use)
	payload->endurance = mCharacter->getCurrentEndurance();

	payload->currentRadCrystals = mCharacter->getInventory()->getRadiantCrystals();
	payload->careerRadCrystals = mCharacter->getInventory()->getTotalRadiantCrystals();
	payload->currentEbonCrystals = mCharacter->getInventory()->getEbonCrystals();
	payload->careerEbonCrystals = mCharacter->getInventory()->getTotalEbonCrystals();

	payload->groupAutoconsent = mCharacter->getAutoConsentGroup() ? 1 : 0;
	payload->raidAutoconsent = mCharacter->getAutoConsentRaid() ? 1 : 0;
	payload->guildAutoconsent = mCharacter->getAutoConsentGuild() ? 1 : 0;

	outPacket->priority = 6;
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendZoneEntry() {
	EXPECTED(mConnected);

	mCharacter->_syncPosition();

	uint32 size = mCharacter->getDataSize();
	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);
	EXPECTED(mCharacter->copyData(ds));
	
	auto outPacket = new EQApplicationPacket(OP_ZoneEntry, data, size);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendZoneSpawns() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ZoneSpawns, 0, 0);
	
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendTributeUpdate() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_TributeUpdate, sizeof(TributeInfo_Struct));
	auto payload = (TributeInfo_Struct *)outPacket->pBuffer;
	
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendInventory() {
	EXPECTED(mConnected);

	auto inventory = mCharacter->getInventory();
	EXPECTED(inventory);

	uint32 size = 0;
	auto data = inventory->getData(size);
	EXPECTED(data);
	EXPECTED(size >= 4); // 4 bytes = empty inventory.

	auto outPacket = new EQApplicationPacket(OP_CharInventory, data, size);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendWeather() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_Weather, 12);
	auto payload = (Weather_Struct*)outPacket->pBuffer;
	//ws->val1 = 0x000000FF;
	//if (zone->zone_weather == 1)
	//	
	//if (zone->zone_weather == 2)
	//{
	//	outapp->pBuffer[8] = 0x01;
	//	ws->type = 0x02;
	//}
	payload->type = 0x31; // Rain
	outPacket->priority = 6;
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleRequestClientSpawn(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);

	mZoneConnectionStatus = ZoneConnectionStatus::ClientRequestSpawn;
	_sendDoors();
	_sendObjects();
	_sendAAStats();
	_sendZonePoints();
	_sendZoneServerReady();
	_sendExpZoneIn();
	_sendWorldObjectsSent();

	if (mCharacter->hasGuild()) {
		GuildManager::getInstance().onEnterZone(mCharacter);
	}
	if (mCharacter->hasGroup()) {
		GroupManager::getInstance().onEnterZone(mCharacter);
	}
	if (mCharacter->hasRaid()) {
		RaidManager::getInstance().onEnterZone(mCharacter);
	}
}

void ZoneClientConnection::_handleClientReady(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	mZoneConnectionStatus = ZoneConnectionStatus::Complete;
	mForceSendPositionTimer.Start(4000);
}

void ZoneClientConnection::_sendDoors() {
	EXPECTED(mConnected);
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpawnDoor, 0);
	//mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendObjects() {
	EXPECTED(mConnected);
	return;
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_GroundSpawn, 0);
	//mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendZonePoints() {
	EXPECTED(mConnected);

	const std::list<ZonePoint*>& zonePoints = mZone->getZonePoints();
	const std::size_t numZonePoints = zonePoints.size();
	if (numZonePoints == 0)
		return;

	uint32 packetSize = 0;
	packetSize += sizeof(uint32); // id
	packetSize += sizeof(uint16); // zone id
	packetSize += sizeof(uint16); // instance id
	packetSize += sizeof(float) * 4; // position / heading.
	packetSize *= numZonePoints;
	packetSize += sizeof(uint32); // count.

	auto outPacket = new EQApplicationPacket(OP_SendZonepoints, packetSize);
	Utility::DynamicStructure ds(outPacket->pBuffer, packetSize);
	
	ds.write<uint32>(numZonePoints); // Count.
	for (auto i : zonePoints) {
		ds.write<uint32>(i->mID);
		ds.write<float>(i->mDestinationPosition.x);
		ds.write<float>(i->mDestinationPosition.y);
		ds.write<float>(i->mDestinationPosition.z);
		ds.write<float>(i->mDestinationHeading);
		ds.write<uint16>(i->mDestinationZoneID);
		ds.write<uint16>(i->mDestinationInstanceID);
	}

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
	EXPECTED(ds.check());
}

void ZoneClientConnection::_sendAAStats() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_SendAAStats, 0);
	mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendZoneServerReady() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ZoneServerReady, 0);
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendExpZoneIn() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_SendExpZonein, 0);
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendWorldObjectsSent() {
	auto outPacket = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleClientUpdate(const EQApplicationPacket* pPacket) {
	using namespace Payload;
	EXPECTED(pPacket);
	EXPECTED(PositionUpdate::sizeCheck(pPacket) || pPacket->size == PositionUpdate::size() + 1); // Payload has an extra byte from time to time.

	auto payload = PositionUpdate::convert(pPacket);
	
	Vector3 previousPosition = mCharacter->getPosition();
	Vector3 newPosition(payload->mX, payload->mY, payload->mZ);
	float delta = previousPosition.distance(newPosition);
	
	//Log::info("InPosition = " + newPosition.toString() + " Delta: " + std::to_string(delta));

	//if (mCharacter->getX() != payload->mX || mCharacter->getY() != payload->mY || mCharacter->getZ() != payload->mZ || FloatToEQ19(mCharacter->getHeading()) != payload->heading || mCharacter->getAnimation() != payload->animation) {
		mCharacter->setPosition(Vector3(payload->mX, payload->mY, payload->mZ));
		mCharacter->setHeading(EQ19toFloat(payload->heading));
		mCharacter->setAnimation(payload->animation);
		mCharacter->setPositionDelta(Vector3(payload->mDeltaX, payload->mDeltaY, payload->mDeltaZ));
		mCharacter->setHeadingDelta(NewEQ13toFloat(payload->delta_heading));
		mZone->handleActorPositionChange(mCharacter);

		// Restart the force send timer.
		mForceSendPositionTimer.Start();
	//}
}

void ZoneClientConnection::_handleSpawnAppearance(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(SpawnAppearance::sizeCheck(pPacket));

	auto payload = SpawnAppearance::convert(pPacket);
	const uint16 actionType = payload->mType;
	const uint32 actionParameter = payload->mParameter;

	// Ignore if spawn id does not match this characters ID.
	if (payload->mSpawnID != mCharacter->getSpawnID()) {
		// Note: UF client sends spawn ID (0) and action type (51) every few seconds. Not sure why.
		return;
	}

	switch (actionType) {
		// Handle animation.
	case SpawnAppearanceType::SA_Animation:
		switch (actionParameter) {
		case SpawnAppearanceAnimation::Standing:
			mCharacter->setStanding(true);
			mZone->handleStanding(mCharacter);
			mCharacter->setStandingState(SpawnAppearanceAnimation::Standing);
			break;
		case SpawnAppearanceAnimation::Freeze:
			mCharacter->setStandingState(SpawnAppearanceAnimation::Freeze);
			break;
		case SpawnAppearanceAnimation::Looting:
			mCharacter->setStandingState(SpawnAppearanceAnimation::Looting);
			break;
		case SpawnAppearanceAnimation::Sitting:
			mCharacter->setStanding(false);
			mZone->handleSitting(mCharacter);
			mCharacter->setStandingState(SpawnAppearanceAnimation::Sitting);
			break;
		case SpawnAppearanceAnimation::Crouch:
			// Crouch or Jump triggers this.
			mZone->handleCrouching(mCharacter);
			mCharacter->setStandingState(SpawnAppearanceAnimation::Crouch);
			break;
		case SpawnAppearanceAnimation::Death:
			mCharacter->setStandingState(SpawnAppearanceAnimation::Death);
			break;
		default:
			StringStream ss;
			ss << "[Zone Client Connection] Got unexpected SpawnAppearanceTypes::Animation parameter : " << actionParameter;
			Log::info(ss.str());
			break;
		}
		// Handle anonymous / roleplay
	case SpawnAppearanceType::Anonymous:
		// 0 = Normal, 1 = Anonymous, 2 = Roleplay
		if (actionParameter >= 0 && actionParameter <= 2) {
			// Update character and notify zone.
			mCharacter->setAnonymous(actionParameter);
			mZone->handleAnonymous(mCharacter);
		}
		// Anything else is ignored.
		break;
		// Handle AFK
	case SpawnAppearanceType::AFK:
		if (actionParameter == 0) {
			// Update character and notify zone.
			mCharacter->setIsAFK(false);
			mZone->handleAFK(mCharacter);
		}
		else if (actionParameter == 1) {
			// Update character and notify zone.
			mCharacter->setIsAFK(true);
			mZone->handleAFK(mCharacter);
		}
		// Anything else is ignored.
		break;
	case SpawnAppearanceType::ShowHelm:
		if (actionParameter == 0) {
			// Update character and notify zone.
			mCharacter->setShowHelm(false);
			mZone->handleShowHelm(mCharacter);
		}
		else if (actionParameter == 1) {
			// Update character and notify zone.
			mCharacter->setShowHelm(true);
			mZone->handleShowHelm(mCharacter);
		}
		// Anything else is ignored.
		break;
		// Ignore!
	case SpawnAppearanceType::HP:
		break;
	case SpawnAppearanceType::Split:
		break;
	case SpawnAppearanceType::Die:
		break;
	case SpawnAppearanceType::DamageState:
		break;
	case SpawnAppearanceType::SA_Sneak:
		break;
	case SpawnAppearanceType::SA_Invisible:
		break;
	case SpawnAppearanceType::Size:
		break;
	case SpawnAppearanceType::SA_Light:
		break;
	case SpawnAppearanceType::SA_PVP:
		Log::info("Got PVP");
		// NOTE: Not sure if this even possible.
		break;
	case SpawnAppearanceType::AutoConsentGroup:
		mCharacter->setAutoConsentGroup(actionParameter == 1);
		break;
	case SpawnAppearanceType::AutoConsentGuild:
		mCharacter->setAutoConsentGuild(actionParameter == 1);
		break;
	case SpawnAppearanceType::AutoConsentRaid:
		mCharacter->setAutoConsentRaid(actionParameter == 1);
		break;
	default:
		StringStream ss;
		ss << "[Zone Client Connection] Got unexpected SpawnAppearanceTypes : " << actionType;
		Log::error(ss.str());
		break;
	}
}

void ZoneClientConnection::_handleCamp(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	mCharacter->startCamp();
}

void ZoneClientConnection::_handleChannelMessage(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_SIZE = sizeof(ChannelMessage_Struct); // NOTE: This packet size increases with message size.
	static const auto MAXIMUM_SIZE = 661; // This is the absolute largest (513 characters + 148 bytes for the rest of the contents).

	EXPECTED(pPacket);
	EXPECTED(pPacket->size <= MAXIMUM_SIZE);
	EXPECTED(pPacket->size >= EXPECTED_SIZE);

	auto payload = reinterpret_cast<ChannelMessage_Struct*>(pPacket->pBuffer);

	static const auto MAX_MESSAGE_SIZE = 513;
	static const auto MAX_SENDER_SIZE = 64;
	static const auto MAX_TARGET_SIZE = 64;
	const String message = Utility::safeString(payload->message, MAX_MESSAGE_SIZE);
	const String senderName = Utility::safeString(payload->sender, MAX_SENDER_SIZE);
	const String targetName = Utility::safeString(payload->targetname, MAX_TARGET_SIZE);
	const uint32 channel = payload->chan_num;
	
	switch (channel) {
	case ChannelID::CH_GUILD:
		EXPECTED(mCharacter->hasGuild());
		GuildManager::getInstance().handleMessage(mCharacter, message);
		break;
	case ChannelID::CH_GROUP:
		GroupManager::getInstance().handleMessage(mCharacter, message);
		break;
	case ChannelID::CH_SHOUT:
		mZone->handleShout(mCharacter, message);
		break;
	case ChannelID::CH_AUCTION:
		mZone->handleAuction(mCharacter, message);
		break;
	case ChannelID::CH_OOC:
		mZone->handleOOC(mCharacter, message);
		break;
	case ChannelID::CH_GMSAY:
	case ChannelID::CH_BROADCAST:
		// GM_SAY / CH_BROADAST are unused as far as I know.
		break;
	case ChannelID::CH_TELL:
		if (senderName.length() > 0 && targetName.length() > 0) {
			mZone->handleTell(mCharacter, targetName, message);
		}
		break;
	case ChannelID::CH_SAY:
		// Check whether user has entered a command.
		if (message[0] == COMMAND_TOKEN) {
			mCommandHandler->command(mCharacter, message);
			break;
		}
		mZone->handleSay(mCharacter, message);
		break;
	case ChannelID::CH_RAID:
		break;
	case ChannelID::CH_UCS:
		break;
		// /emote dances around wildly!
	case ChannelID::CH_EMOTE:
		mZone->handleEmote(mCharacter, message);
		break;
	default:
		StringStream ss;
		ss << "[Zone Client Connection] " << __FUNCTION__ << " Got unexpected channel number: " << channel;
		Log::error(ss.str());
		break;
	}
}

void ZoneClientConnection::sendPosition() {
	using namespace Payload;
	EXPECTED(mConnected);

	mCharacter->_syncPosition();
	auto position = mCharacter->getPositionData();
	auto packet = ActorPosition::create(position);
	sendPacket(packet);
	packet->pBuffer = nullptr;
	safe_delete(packet);
}

/*
header[0]
0x00 = asdf
0x01 = says 'asdf'
0x02 = shouts 'asdf'
0x03 = asdf
0x04 = asdf
0x05 = tells the group, 'asdf'
6 says
7 says
..
30 says

header[1]
*/

void ZoneClientConnection::sendMessage(MessageType pType, String pMessage) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_SpecialMesg, sizeof(SpecialMesg_Struct)+pMessage.length());
	auto payload = reinterpret_cast<SpecialMesg_Struct*>(outPacket->pBuffer);
	payload->header[0] = 0x00; // Header used for #emote style messages..
	payload->header[1] = 0x00; // Play around with these to see other types
	payload->header[2] = 0x00;
	payload->msg_type = static_cast<uint32>(pType);
	strcpy(payload->message, pMessage.c_str());
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleLogOut(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
	auto payload = reinterpret_cast<CancelTrade_Struct*>(outPacket->pBuffer);
	payload->fromid = mCharacter->getSpawnID();
	payload->action = groupActUpdate;
	mStreamInterface->FastQueuePacket(&outPacket);

	_sendPreLogOutReply();
	_sendLogOutReply();

	mCharacter->setCampComplete(true);

	// NOTE: Zone picks up the dropped connection next update.
	dropConnection();
}

void ZoneClientConnection::_sendLogOutReply() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_LogoutReply);
	mStreamInterface->FastQueuePacket(&outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendPreLogOutReply() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_PreLogoutReply);
	mStreamInterface->FastQueuePacket(&outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleDeleteSpawn(const EQApplicationPacket* pPacket) {
	_sendLogOutReply();
	mCharacter->setZoningOut();

	// NOTE: Zone picks up the dropped connection next update.
	dropConnection();
}

void ZoneClientConnection::_handleRequestNewZoneData(const EQApplicationPacket* pPacket) {
	mZoneConnectionStatus = ZoneConnectionStatus::ClientRequestZoneData;
	_sendNewZoneData();
}

void ZoneClientConnection::_sendNewZoneData() {
	EXPECTED(mConnected);

	// TODO: Send some real data.
	auto outPacket = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	auto payload = reinterpret_cast<NewZone_Struct*>(outPacket->pBuffer);
	strcpy(payload->char_name, mCharacter->getName().c_str());
	payload->zone_id = mZone->getID();
	payload->zone_instance = mZone->getInstanceID();
	strcpy(payload->zone_short_name, mZone->getShortName().c_str());
	strcpy(payload->zone_long_name, mZone->getLongName().c_str()); // NOTE: This affects the zone in message "You have entered ..."
	payload->gravity = mZone->getGravity();
	payload->underworld = mZone->getMinimumZ();
	payload->ztype = mZone->getZoneType();

	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::sendAppearance(uint16 pType, uint32 pParameter) {
	EXPECTED(mConnected);
	
	auto outPacket = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	auto payload = reinterpret_cast<SpawnAppearance_Struct*>(outPacket->pBuffer);
	payload->spawn_id = mCharacter->getSpawnID();
	payload->type = pType;
	payload->parameter = pParameter;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleSendAATable(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	// TODO:
}

void ZoneClientConnection::_handleUpdateAA(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	// TODO:
}

void ZoneClientConnection::_handleTarget(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Target::sizeCheck(pPacket->size));

	auto payload = Target::convert(pPacket->pBuffer);
	mZone->handleTarget(mCharacter, payload->mSpawnID);
}

void ZoneClientConnection::_handleTGB(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(uint32);

	EXPECTED(pPacket);
	EXPECTED(pPacket->size == EXPECTED_PAYLOAD_SIZE);

	const uint32 tgb = *(uint32 *)pPacket->pBuffer;
	if (tgb == 0 || tgb == 1) {
		mCharacter->setTGB(tgb == 1);
		sendSimpleMessage(MessageType::White, mCharacter->getTGB() ? StringID::TGB_ON : StringID::TGB_OFF);
	}
	// Ignore anything else, including the extra 2 packet UF sends.
}

void ZoneClientConnection::sendSimpleMessage(MessageType pType, StringID pStringID) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_SimpleMessage, sizeof(SimpleMessage_Struct));
	auto payload = reinterpret_cast<SimpleMessage_Struct*>(outPacket->pBuffer);
	payload->color = static_cast<uint32>(pType);
	payload->string_id = static_cast<uint32>(pStringID);

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendSimpleMessage(MessageType pType, StringID pStringID, String pParameter0, String pParameter1, String pParameter2, String pParameter3, String pParameter4, String pParameter5, String pParameter6, String pParameter7, String pParameter8, String pParameter9) {
	EXPECTED(mConnected);

	int packetSize = 0;
	String message;

	if (pParameter0.length() != 0) packetSize += pParameter0.length() + 1;
	if (pParameter1.length() != 0) packetSize += pParameter1.length() + 1;
	if (pParameter2.length() != 0) packetSize += pParameter2.length() + 1;
	if (pParameter3.length() != 0) packetSize += pParameter3.length() + 1;
	if (pParameter4.length() != 0) packetSize += pParameter4.length() + 1;
	if (pParameter5.length() != 0) packetSize += pParameter5.length() + 1;
	if (pParameter6.length() != 0) packetSize += pParameter6.length() + 1;
	if (pParameter7.length() != 0) packetSize += pParameter7.length() + 1;
	if (pParameter8.length() != 0) packetSize += pParameter8.length() + 1;
	if (pParameter9.length() != 0) packetSize += pParameter9.length() + 1;

	packetSize += sizeof(FormattedMessage_Struct);
	auto outPacket = new EQApplicationPacket(OP_FormattedMessage, packetSize);
	auto payload = reinterpret_cast<FormattedMessage_Struct*>(outPacket->pBuffer);
	payload->type = static_cast<uint32>(pType);
	payload->string_id = static_cast<uint32>(pStringID);

	Utility::DynamicStructure ds(outPacket->pBuffer, packetSize);
	ds.movePointer(sizeof(FormattedMessage_Struct));

	if (pParameter0.length() != 0) ds.writeString(pParameter0);
	if (pParameter1.length() != 0) ds.writeString(pParameter1);
	if (pParameter2.length() != 0) ds.writeString(pParameter2);
	if (pParameter3.length() != 0) ds.writeString(pParameter3);
	if (pParameter4.length() != 0) ds.writeString(pParameter4);
	if (pParameter5.length() != 0) ds.writeString(pParameter5);
	if (pParameter6.length() != 0) ds.writeString(pParameter6);
	if (pParameter7.length() != 0) ds.writeString(pParameter7);
	if (pParameter8.length() != 0) ds.writeString(pParameter8);
	if (pParameter9.length() != 0) ds.writeString(pParameter9);


	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
	EXPECTED(ds.check());
}

void ZoneClientConnection::sendHPUpdate() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	HPUpdate payload;
	payload.mSpawnID = mCharacter->getSpawnID();
	payload.mCurrentHP = mCharacter->getCurrentHP();
	payload.mMaximumHP = mCharacter->getMaximumHP();

	auto packet = HPUpdate::create(payload);
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneClientConnection::sendPacket(EQApplicationPacket* pPacket) {
	EXPECTED(mConnected);
	mStreamInterface->QueuePacket(pPacket);
}

void ZoneClientConnection::_handleEmote(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(Emote_Struct);

	EXPECTED(pPacket);
	EXPECTED(pPacket->size == EXPECTED_PAYLOAD_SIZE);

	static const unsigned int MAX_EMOTE_SIZE = 1024;
	auto payload = reinterpret_cast<Emote_Struct*>(pPacket->pBuffer);
	String message = Utility::safeString(payload->message, MAX_EMOTE_SIZE);
	mZone->handleEmote(mCharacter, message);
}

void ZoneClientConnection::_handleAnimation(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ActorAnimation::sizeCheck(pPacket));
	
	auto payload = ActorAnimation::convert(pPacket);
	EXPECTED(payload->mSpawnID == mCharacter->getSpawnID());

	mZone->handleAnimation(mCharacter, payload->mAnimation, payload->mSpeed, false);
}

void ZoneClientConnection::sendExperienceUpdate() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
	auto payload = reinterpret_cast<ExpUpdate_Struct*>(outPacket->pBuffer);
	payload->exp = mCharacter->getExperienceRatio();
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::sendLevelUpdate() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_LevelUpdate, sizeof(LevelUpdate_Struct));
	auto payload = reinterpret_cast<LevelUpdate_Struct*>(outPacket->pBuffer);
	payload->level = mCharacter->getLevel();
	payload->exp = mCharacter->getExperienceRatio();

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendExperienceGain() {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::Experience, StringID::GAIN_XP);
}

void ZoneClientConnection::sendExperienceLoss() {
	EXPECTED(mConnected);
	// There is no StringID for this message apparently.
	sendMessage(MessageType::Yellow, "You have lost experience.");
}

void ZoneClientConnection::sendLevelGain() {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::Experience, StringID::GAIN_LEVEL, std::to_string(mCharacter->getLevel()));
}

void ZoneClientConnection::sendLevelLost() {
	// NOTE: UF Handles this message itself, no need to send.
	//StringStream ss;
	//ss << mCharacter->getLevel();
	//sendSimpleMessage(MT_Experience, LOSE_LEVEL, ss.str());
}

void ZoneClientConnection::sendLevelAppearance() {
	EXPECTED(mConnected);
}

void ZoneClientConnection::sendStats() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_IncreaseStats, sizeof(IncreaseStat_Struct));
	auto payload = (IncreaseStat_Struct*)outPacket->pBuffer;
	payload->str = 5;
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleWhoAllRequest(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(Who_All_Struct);

	EXPECTED(pPacket);
	EXPECTED(pPacket->size == EXPECTED_PAYLOAD_SIZE);

	auto payload = reinterpret_cast<Who_All_Struct*>(pPacket->pBuffer);
	if (payload->type != 0 && payload->type != 3) return;

	WhoFilter filter;
	filter.mType = payload->type == 0 ? WHO_ZONE : WHO_WORLD;
	filter.mName = Utility::safeString(payload->whom, 64);

	if (payload->lvllow != 0xFFFFFFFF) filter.mMinLevel = payload->lvllow;
	if (payload->lvlhigh != 0xFFFFFFFF) filter.mMaxLevel = payload->lvlhigh;
	if (payload->wrace != 0xFFFFFFFF) filter.mRace = payload->wrace;
	if (payload->wclass != 0xFFFFFFFF) filter.mClass = payload->wclass;

	mZone->whoRequest(mCharacter, filter);
}

void ZoneClientConnection::sendWhoResults(std::list<Character*>& pMatches) {
	EXPECTED(mConnected);

	static const String LINE("---------------------------");
	int packetSize = 0;
	int numResults = pMatches.size();

	String FakeGuild = "WooHoo";
	String FakeAccount = "WHAT IS THIS?";
	
	// The first loop over pMatches is required to calculate the space needed for Character name / guild name.
	for (auto i : pMatches) {
		packetSize += i->getName().length() + 1; // + 1 is for the null terminator (String::length includes only characters).
		if (i->getGuildID() != 0xFFFFFFFF) { // TODO: Remove this hex
			// TODO: When Guilds.
		}

		packetSize += FakeGuild.length() + 1;
		packetSize += FakeAccount.length() + 1;
	}

	packetSize += sizeof(WhoAllReturnStruct) + (numResults * sizeof(WhoAllPlayer));
	auto outPacket = new EQApplicationPacket(OP_WhoAllResponse, packetSize);
	auto payload = reinterpret_cast<WhoAllReturnStruct*>(outPacket->pBuffer);
	payload->id = 0;
	payload->playerineqstring = 5001; // TODO: Magic.
	strcpy(payload->line, LINE.c_str());
	payload->unknown35 = 0x0a;
	payload->unknown36 = 0;
	payload->playersinzonestring = 5029;
	payload->unknown44[0] = 0;
	payload->unknown44[1] = 0;
	payload->unknown52 = numResults;
	payload->unknown56 = numResults;
	payload->playercount = numResults;

	StringStream ss;
	ss << "Size WhoAllReturnStruct= " << sizeof(WhoAllReturnStruct);
	Log::error(ss.str());
	ss.str("");
	ss << "Size WhoAllPlayer= " << sizeof(WhoAllPlayer);
	Log::error(ss.str());

	Utility::DynamicStructure ds(outPacket->pBuffer, packetSize);
	ds.movePointer(sizeof(WhoAllReturnStruct)); // Move the pointer to where WhoAllPlayer begin.

	ss << "Written 1: " << ds.getBytesWritten();
	Log::error(ss.str());

	enum RankMessageID {
		RM_STEWARD = 5007,				// * Steward *
		RM_APPRENTICE_GUIDE = 5008,		// * Apprentice Guide *
		RM_GUIDE = 5009,				// * Guide *
		RM_QUESTTROUPE = 5010,			// * QuestTroupe *
		RM_SENIOR_GUIDE = 5011,			// * Senior Guide *
		RM_GM_TESTER = 5012,			// * GM - Tester *
		RM_EQ_SUPPORT = 5013,			// * EQ Support *
		RM_GM_STAFF = 5014,				// * GM - Staff *
		RM_GM_ADMIN = 5015,				// * GM - Admin *
		RM_GM_LEAD_ADMIN = 5016,		// * GM - Lead Admin *
		RM_QUESTMASTER = 5017,			// * QuestMaster *
		RM_GM_AREAS = 5018,				// * GM - Areas *
		RM_GM_CODER = 5019,				// * GM - Coder *
		RM_GM_MGMT = 5020,				// * GM - Mgmt *
		RM_GM_IMPOSSIBLE = 5021,		// * GM - Impossible *
		RM_AFK = 12311,
		RM_GM = 12312,
		RM_LD = 12313,
		RM_LFG = 12314,
		RM_TRADER = 12315
	};

	enum FormatStringID {
		FS_GM_ANONYMOUS = 5022,		// % T1[ANON(% 2 % 3)] % 4 (% 5) % 6 % 7 % 8
		FS_ROLEPLAY = 5023,		// % T1[ANONYMOUS] % 2 % 3 % 4
		FS_ANONYMOUS = 5024,		//% T1[ANONYMOUS] % 2 % 3
		FS_DEFAULT = 5025		//% T1[% 2 % 3] % 4 (% 5) % 6 % 7 % 8 % 9
	};
	const bool receiverIsGM = mCharacter->isGM() == 1 ? true : false;
	for (auto i : pMatches) {
		// NOTE: The write methods below *MUST* stay in order.

		// Determine Format String ID.
		uint32 formatString = FS_DEFAULT;
		if (i->getAnonymous() != AnonType::None) {
			// Player is /roleplay
			if (i->isRoleplaying()) 
				formatString = FS_ROLEPLAY;
			// Player is /anonymous
			if (i->isAnonymous())
				formatString = FS_ANONYMOUS;

			// Allows GM to see through anonymous.
			if (receiverIsGM)
				formatString = FS_GM_ANONYMOUS;
		}
		ds.write<uint32>(formatString); // formatstring

		ds.write<uint32>(0xFFFFFFFF); // pidstring (Not sure what this does).

		ds.writeString(i->getName()); // name
		
		// Determine Rank String ID.
		uint32 rankStringID = 0xFFFFFFFF;
		if (i->getStatus() > 0) {
			// TODO: Can break these up further and use more rank IDs.
			if (i->getStatus() == 255) {
				rankStringID = RM_GM_IMPOSSIBLE;
			}
			else {
				rankStringID = RM_GM;
			}
		}
		ds.write<uint32>(rankStringID); // rankstring
		//dynamicStructure.write<uint32>(1509); // rankstring

		ds.writeString(FakeGuild); // guild
		//dynamicStructure.write<uint32>(0); // guild
		ds.write<uint32>(0xFFFFFFFF); // unknown80[0]
		ds.write<uint32>(0xFFFFFFFF); // unknown80[1]
		//dynamicStructure.write<uint32>(0xFFFFFFFF); // zonestring
		ds.write<uint32>(i->getZone()->getLongNameStringID()); // zonestring // This is StringID 
		ds.write<uint32>(4); // zone (Not sure what this does).
		ds.write<uint32>(i->getClass()); // class_
		ds.write<uint32>(i->getLevel()); // level
		ds.write<uint32>(i->getRace()); // race
		ds.writeString(FakeAccount); // account
		//dynamicStructure.write<uint32>(0); // account
		ds.write<uint32>(0); // unknown100
		//ss << "Written 4: " << dynamicStructure.getBytesWritten();
		//Log::error(ss.str());
		//ss.str("");
	}

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
	EXPECTED(ds.check());
}

void ZoneClientConnection::sendChannelMessage(const ChannelID pChannel, const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ChannelMessage, sizeof(ChannelMessage_Struct)+pMessage.length() + 1);
	auto payload = reinterpret_cast<ChannelMessage_Struct*>(outPacket->pBuffer);
	payload->language = Languages::COMMON_TONGUE_LANG;
	payload->skill_in_language = 0;
	payload->chan_num = static_cast<uint32>(pChannel);
	strcpy(payload->message, pMessage.c_str());
	strcpy(payload->sender, pSenderName.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendTell(const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);
	sendChannelMessage(ChannelID::CH_TELL, pSenderName, pMessage);
}

void ZoneClientConnection::sendGroupMessage(const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);
	sendChannelMessage(ChannelID::CH_GROUP, pSenderName, pMessage);
}

void ZoneClientConnection::sendGuildMessage(const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);
	sendChannelMessage(ChannelID::CH_GUILD, pSenderName, pMessage);
}

// NOTE: This occurs when the player presses 'Invite' on the group window.
void ZoneClientConnection::_handleGroupInvite(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupInvite_Struct));

	auto payload = reinterpret_cast<GroupInvite_Struct*>(pPacket->pBuffer);

	const String inviterName = Utility::safeString(payload->inviter_name, Limits::Character::MAX_NAME_LENGTH);
	const String inviteeName = Utility::safeString(payload->invitee_name, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(Limits::Character::nameLength(inviteeName));
	EXPECTED(inviterName == mCharacter->getName()); // Check: Spoofing
	EXPECTED(inviteeName != mCharacter->getName()); // Check: Not inviting ourself
	
	GroupManager::getInstance().handleInviteSent(mCharacter, inviteeName);
}

void ZoneClientConnection::sendGroupInvite(const String pFromCharacterName) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_GroupInvite, sizeof(GroupInvite_Struct));
	auto payload = reinterpret_cast<GroupInvite_Struct*>(outPacket->pBuffer);
	strcpy(payload->inviter_name, pFromCharacterName.c_str());
	strcpy(payload->invitee_name, mCharacter->getName().c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleGroupFollow(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupGeneric_Struct));

	auto payload = reinterpret_cast<GroupGeneric_Struct*>(pPacket->pBuffer);
	
	String inviterName = Utility::safeString(payload->name1, Limits::Character::MAX_NAME_LENGTH); // Character who invited.
	String inviteeName = Utility::safeString(payload->name2, Limits::Character::MAX_NAME_LENGTH); // Character accepting invite.
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(Limits::Character::nameLength(inviteeName));
	EXPECTED(inviteeName == mCharacter->getName()); // Check: Sanity

	// TODO: This can be spoofed to join groups...

	GroupManager::getInstance().handleAcceptInvite(mCharacter, inviterName);
}

void ZoneClientConnection::_handleGroupCanelInvite(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupCancel_Struct));

	auto payload = reinterpret_cast<GroupCancel_Struct*>(pPacket->pBuffer);
	String inviterName = Utility::safeString(payload->name1, Limits::Character::MAX_NAME_LENGTH);
	String inviteeName = Utility::safeString(payload->name2, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(Limits::Character::nameLength(inviteeName));
	EXPECTED(inviteeName == mCharacter->getName()); // Check: Sanity

	GroupManager::getInstance().handleDeclineInvite(mCharacter, inviterName);
}

void ZoneClientConnection::sendGroupCreate() {
	EXPECTED(mConnected);

	int packetSize = 31 + mCharacter->getName().length() + 1; // Magic number due to no packet structure.
	auto outPacket = new EQApplicationPacket(OP_GroupUpdateB, packetSize);

	Utility::DynamicStructure ds(outPacket->pBuffer, packetSize);
	ds.write<uint32>(0); // 4
	ds.write<uint32>(1); // 8
	ds.write<uint8>(0); // 9
	ds.write<uint32>(0); // 13
	ds.writeString(mCharacter->getName()); // dynamic
	ds.write<uint8>(0); // 14
	ds.write<uint8>(0); // 15
	ds.write<uint8>(0); // 16
	ds.write<uint32>(mCharacter->getLevel()); // 20
	ds.write<uint8>(0); // 21
	ds.write<uint32>(0); // 25
	ds.write<uint32>(0); // 29
	ds.write<uint16>(0); // 31

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
	EXPECTED(ds.check());
}

void ZoneClientConnection::sendGroupLeaderChange(const String pCharacterName) {
	EXPECTED(mConnected);

	// Configure.
	auto payload = reinterpret_cast<GroupLeaderChange_Struct*>(mGroupLeaderChangePacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	strcpy(payload->LeaderName, pCharacterName.c_str());

	// Send.
	mStreamInterface->QueuePacket(mGroupLeaderChangePacket);
}

void ZoneClientConnection::sendGroupAcknowledge() {
	EXPECTED(mConnected);

	static const auto PACKET_SIZE = 4;
	auto outPacket = new EQApplicationPacket(OP_GroupAcknowledge, PACKET_SIZE);

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendGroupFollow(const String& pLeaderCharacterName, const String& pMemberCharacterName) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_GroupFollow, sizeof(GroupGeneric_Struct));
	auto payload = reinterpret_cast<GroupGeneric_Struct*>(outPacket->pBuffer);
	strcpy(payload->name1, pLeaderCharacterName.c_str());
	strcpy(payload->name2, pMemberCharacterName.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendGroupJoin(const String& pCharacterName) {
	EXPECTED(mConnected);

	// Configure.
	auto payload = reinterpret_cast<GroupJoin_Struct*>(mGroupJoinPacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActJoin;
	strcpy(payload->membername, pCharacterName.c_str());
	strcpy(payload->yourname, mCharacter->getName().c_str());

	// Send.
	mStreamInterface->QueuePacket(mGroupJoinPacket);
}

void ZoneClientConnection::sendGroupUpdate(std::list<String>& pGroupMemberNames) {
	EXPECTED(mConnected);

	// Configure.
	auto payload = reinterpret_cast<GroupUpdate2_Struct*>(mGroupUpdateMembersPacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActUpdate;
	strcpy(payload->yourname, mCharacter->getName().c_str());

	int count = 0;
	for (auto i : pGroupMemberNames) {
		strcpy(payload->membername[count], i.c_str());
		count++;
	}
	
	// Send.
	mStreamInterface->QueuePacket(mGroupUpdateMembersPacket);
}

void ZoneClientConnection::_handleGroupDisband(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupGeneric_Struct));
	
	auto payload = reinterpret_cast<GroupGeneric_Struct*>(pPacket->pBuffer);

	String removeCharacterName = Utility::safeString(payload->name1, Limits::Character::MAX_NAME_LENGTH);
	String myCharacterName = Utility::safeString(payload->name2, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(removeCharacterName));
	EXPECTED(Limits::Character::nameLength(myCharacterName));
	EXPECTED(myCharacterName == mCharacter->getName()); // Check: Sanity
	EXPECTED(mCharacter->hasGroup());

	GroupManager::getInstance().handleDisband(mCharacter, removeCharacterName);
}

void ZoneClientConnection::sendGroupLeave(const String& pLeavingCharacterName) {
	// Configure.
	auto payload = reinterpret_cast<GroupJoin_Struct*>(mGroupLeavePacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActLeave;
	strcpy(payload->yourname, mCharacter->getName().c_str());
	strcpy(payload->membername, pLeavingCharacterName.c_str());

	// Send.
	mStreamInterface->QueuePacket(mGroupLeavePacket);
}

void ZoneClientConnection::sendGroupDisband() {
	// Configure.
	auto payload = reinterpret_cast<GroupUpdate_Struct*>(mGroupDisbandPacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActDisband;
	strcpy(payload->yourname, mCharacter->getName().c_str());

	// Send.
	mStreamInterface->QueuePacket(mGroupDisbandPacket);
}

void ZoneClientConnection::_handleGroupMakeLeader(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupMakeLeader_Struct));
	EXPECTED(mCharacter->hasGroup());

	auto payload = reinterpret_cast<GroupMakeLeader_Struct*>(pPacket->pBuffer);

	String currentLeader = Utility::safeString(payload->CurrentLeader, Limits::Character::MAX_NAME_LENGTH);
	String newLeader = Utility::safeString(payload->NewLeader, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(currentLeader));
	EXPECTED(Limits::Character::nameLength(newLeader));
	EXPECTED(currentLeader == mCharacter->getName());

	GroupManager::getInstance().handleMakeLeader(mCharacter, newLeader);
}

void ZoneClientConnection::sendRequestZoneChange(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = RequestZoneChange::construct(pZoneID, pInstanceID, pPosition);
	Log::info("sendRequestZoneChange: " + pPosition.toString());
	mStreamInterface->QueuePacket(packet);
	safe_delete(packet);
}

void ZoneClientConnection::sendZoneChange(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition, const int32 pSuccess) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ZoneChange::construct(mCharacter->getName(), pZoneID, pInstanceID, pPosition, pSuccess);
	Log::info("sendZoneChange: " + pPosition.toString());
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneClientConnection::_handleZoneChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ZoneChange::sizeCheck(pPacket));

	auto payload = ZoneChange::convert(pPacket);
	mZone->handleZoneChange(mCharacter, payload->mZoneID, payload->mInstanceID, Vector3(payload->mX, payload->mY, payload->mZ));
}

void ZoneClientConnection::_handleGuildCreate(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild() == false);
	EXPECTED(pPacket->size == Limits::Guild::MAX_NAME_LENGTH);
	
	const String guildName = Utility::safeString(reinterpret_cast<char*>(pPacket->pBuffer), Limits::Guild::MAX_NAME_LENGTH);
	EXPECTED(Limits::Guild::nameLength(guildName));

	GuildManager::getInstance().handleCreate(mCharacter, guildName);
}

void ZoneClientConnection::_handleGuildDelete(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(GuildManager::getInstance().isLeader(mCharacter)); // Check: Permission.

	GuildManager::getInstance().handleDelete(mCharacter);
}

void ZoneClientConnection::sendGuildRank() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));
	auto payload = reinterpret_cast<GuildSetRank_Struct*>(outPacket->pBuffer);
	payload->Rank = mCharacter->getGuildRank();
	payload->Banker = 0;
	strcpy(payload->MemberName, mCharacter->getName().c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendGuildNames() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_GuildsList);
	outPacket->size = Limits::Guild::MAX_NAME_LENGTH + (Limits::Guild::MAX_NAME_LENGTH * Limits::Guild::MAX_GUILDS); // TODO: Work out the minimum sized packet UF will accept.
	outPacket->pBuffer = reinterpret_cast<unsigned char*>(GuildManager::getInstance()._getGuildNames());

	mStreamInterface->QueuePacket(outPacket);
	outPacket->pBuffer = nullptr;
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleGuildInvite(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild()); // Check: Character has a guild.
	EXPECTED(GuildManager::getInstance().isLeader(mCharacter) || GuildManager::getInstance().isOfficer(mCharacter)); // Check: Permission.
	EXPECTED(pPacket->size == sizeof(GuildCommand_Struct));

	auto payload = reinterpret_cast<GuildCommand_Struct*>(pPacket->pBuffer);

	EXPECTED(payload->guildeqid == mCharacter->getGuildID()); // Check: Sanity. Why the fuck does the client send this?

	String toCharacterName = Utility::safeString(payload->othername, Limits::Character::MAX_NAME_LENGTH);
	String fromCharacterName = Utility::safeString(payload->myname, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(toCharacterName)); // NOTE: Client does not check this..
	EXPECTED(Limits::Character::nameLength(fromCharacterName));
	EXPECTED(fromCharacterName == mCharacter->getName()); // Check: Sanity.

	if (payload->officer == 0)
		GuildManager::getInstance().handleInviteSent(mCharacter, toCharacterName);
	if (payload->officer == 1)
		GuildManager::getInstance().handlePromote(mCharacter, toCharacterName);

	// NOTE: UF requires that the character being promoted is targetted we can verify this later.
}

void ZoneClientConnection::_handleGuildRemove(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild()); // Check: Character has a guild.
	EXPECTED(pPacket->size == sizeof(GuildCommand_Struct));

	auto payload = reinterpret_cast<GuildCommand_Struct*>(pPacket->pBuffer);

	String toCharacterName = Utility::safeString(payload->othername, Limits::Character::MAX_NAME_LENGTH);
	String fromCharacterName = Utility::safeString(payload->myname, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(toCharacterName));
	EXPECTED(Limits::Character::nameLength(fromCharacterName));
	EXPECTED(fromCharacterName == mCharacter->getName()) // Check: Sanity.

	// Check: Permission
	if (toCharacterName != fromCharacterName)
		EXPECTED(GuildManager::getInstance().isLeader(mCharacter) || GuildManager::getInstance().isOfficer(mCharacter));

	GuildManager::getInstance().handleRemove(mCharacter, toCharacterName);
}

void ZoneClientConnection::sendGuildInvite(String pInviterName, GuildID pGuildID) {
	EXPECTED(mConnected);
	EXPECTED(pGuildID != NO_GUILD);
	EXPECTED(mCharacter->hasGuild() == false);
	EXPECTED(mCharacter->getPendingGuildInviteID() == pGuildID);

	auto outPacket = new EQApplicationPacket(OP_GuildInvite, sizeof(GuildCommand_Struct));
	auto payload = reinterpret_cast<GuildCommand_Struct*>(outPacket->pBuffer);

	payload->guildeqid = pGuildID;
	// NOTE: myname/othername were poor choices for variable names.
	strcpy(payload->othername, mCharacter->getName().c_str());
	strcpy(payload->myname, pInviterName.c_str());
	
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleGuildInviteAccept(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	if (mCharacter->hasGuild()) { return; } // NOTE: UF sends OP_GuildInvite and OP_GuildInviteAccept(response=2,guildid=0) when using /guildinvite .. not sure why.
	EXPECTED(mCharacter->hasPendingGuildInvite()); // Check: This Character has actually been invited to *A* Guild
	EXPECTED(pPacket->size == sizeof(GuildInviteAccept_Struct));

	auto payload = reinterpret_cast<GuildInviteAccept_Struct*>(pPacket->pBuffer);
	
	String characterName = Utility::safeString(payload->newmember, Limits::Character::MAX_NAME_LENGTH);
	String inviterName = Utility::safeString(payload->inviter, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(characterName));
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(mCharacter->getName() == characterName); // Check: Sanity.
	EXPECTED(mCharacter->getPendingGuildInviteName() == inviterName); // Check: This Character is responding to the correct inviter.
	EXPECTED(mCharacter->getPendingGuildInviteID() == payload->guildeqid); // Check: This Character is responding to the correct Guild invite.
	
	static const auto Accept = 0;
	static const auto Decline = 5;

	// Character is accepting the invite.
	if (payload->response == Accept) {
		GuildManager::getInstance().handleInviteAccept(mCharacter, inviterName);
		return;
	}
	// Character is declining the invite.
	if (payload->response == Decline) {
		GuildManager::getInstance().handleInviteDecline(mCharacter, inviterName);
		return;
	}

	Log::error("[Zone Client Connection] Got unexpected response(" + std::to_string(payload->response) + ") to Guild invite " + Utility::characterLogDetails(mCharacter));
	mCharacter->clearPendingGuildInvite();
}

void ZoneClientConnection::_handleSetGuildMOTD(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(GuildManager::getInstance().isLeader(mCharacter) || GuildManager::getInstance().isOfficer(mCharacter)); // Only leader or officers can set the MOTD.
	EXPECTED(pPacket->size == sizeof(GuildMOTD_Struct));

	auto payload = reinterpret_cast<GuildMOTD_Struct*>(pPacket->pBuffer);

	String characterName = Utility::safeString(payload->name, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(characterName == mCharacter->getName()); // Check: Sanity.
	String motd = Utility::safeString(payload->motd, Limits::Guild::MAX_MOTD_LENGTH);

	GuildManager::getInstance().handleSetMOTD(mCharacter, motd);
}

void ZoneClientConnection::sendGuildMOTD(const String& pMOTD, const String& pMOTDSetByName) {
	EXPECTED(mConnected);
	//EXPECTED(mCharacter->hasGuild());

	auto outPacket = new EQApplicationPacket(OP_GuildMOTD, sizeof(GuildMOTD_Struct));
	auto payload = reinterpret_cast<GuildMOTD_Struct*>(outPacket->pBuffer);
	payload->unknown0 = 0;
	strcpy(payload->name, mCharacter->getName().c_str());
	strcpy(payload->setby_name, pMOTDSetByName.c_str());
	strcpy(payload->motd, pMOTD.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendGuildMOTDReply(const String& pMOTD, const String& pMOTDSetByName) {
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild());

	auto outPacket = new EQApplicationPacket(OP_GetGuildMOTDReply, sizeof(GuildMOTD_Struct));
	auto payload = reinterpret_cast<GuildMOTD_Struct*>(outPacket->pBuffer);
	payload->unknown0 = 0;
	strcpy(payload->name, mCharacter->getName().c_str());
	strcpy(payload->setby_name, pMOTDSetByName.c_str());
	strcpy(payload->motd, pMOTD.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleGetGuildMOTD(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());

	GuildManager::getInstance().handleGetMOTD(mCharacter);
}

void ZoneClientConnection::sendGuildMembers(const std::list<GuildMember*>& pGuildMembers) {
	EXPECTED(mConnected);

	static const auto HeaderSize = 76; // Member count etc.
	static const auto MemberBodySize = 36; // Level, class etc.

	// Calculate payload size.
	std::size_t payloadSize = HeaderSize + (MemberBodySize * pGuildMembers.size());
	uint32 namesLength = 0;
	uint32 notesLength = 0;
	for (auto i : pGuildMembers) {
		payloadSize += i->mName.length() + 1;
		payloadSize += i->mPublicNote.length() + 1;
		namesLength += i->mName.length();
		notesLength += i->mPublicNote.length();
	}

	auto outPacket = new EQApplicationPacket(OP_GuildMemberList, payloadSize);
	
	Utility::DynamicStructure ds(outPacket->pBuffer, payloadSize);
	
	// Write Header.
	ds.writeFixedString(mCharacter->getName(), Limits::Character::MAX_NAME_LENGTH);
	ds.write<uint32>(pGuildMembers.size());
	ds.write<uint32>(namesLength);
	ds.write<uint32>(notesLength);

	// Write member data.
	for (auto i : pGuildMembers) {
		ds.write<uint32>(i->mLevel);
		ds.write<uint32>(i->mBanker + i->mAlt * 2);
		ds.write<uint32>(i->mClass);
		ds.write<uint32>(i->mRank);
		ds.write<uint32>(i->mTimeLastOn);
		ds.write<uint32>(i->mTributeEnabled ? 1 : 0);
		ds.write<uint32>(i->mTotalTribute);
		ds.write<uint32>(i->mLastTribute);
		ds.write<uint16>(i->mInstanceID);
		ds.write<uint16>(i->mZoneID);
	}
	// Write Character names.
	for (auto i : pGuildMembers) {
		ds.writeString(i->mName);
	}
	// Write public notes.
	for (auto i : pGuildMembers) {
		ds.writeString(i->mPublicNote);
	}

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
	EXPECTED(ds.check());
}

void ZoneClientConnection::sendGuildURL(const String& pURL) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(outPacket->pBuffer);
	payload->mAction = Payload::Guild::GuildUpdate::GUILD_URL;
	strcpy(&payload->mText[0], pURL.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendGuildChannel(const String& pChannel) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(outPacket->pBuffer);
	payload->mAction = Payload::Guild::GuildUpdate::GUILD_CHANNEL;
	strcpy(&payload->mText[0], pChannel.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleSetGuildURLOrChannel(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(GuildManager::getInstance().isLeader(mCharacter)); // Only a Guild leader can perform this operation.
	EXPECTED(pPacket->size == sizeof(Payload::Guild::GuildUpdate));

	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(pPacket->pBuffer);
	if (payload->mAction == GUILD_URL) {
		String url = Utility::safeString(payload->mText, Limits::Guild::MAX_URL_LENGTH);
		GuildManager::getInstance().handleSetURL(mCharacter, url);
	}
	else if (payload->mAction == GUILD_CHANNEL) {
		String channel = Utility::safeString(payload->mText, Limits::Guild::MAX_CHANNEL_LENGTH);
		GuildManager::getInstance().handleSetChannel(mCharacter, channel);
	}
	else {
		StringStream ss; ss << "[Zone Client Connection]Got unknown action value(" << payload->mAction << ") in _handleSetGuildURLOrChannel from " << Utility::characterLogDetails(mCharacter);
		Log::error(ss.str());
	}
}

void ZoneClientConnection::_handleSetGuildPublicNote(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	// TODO: Put an upper-limit check on packet size.
	EXPECTED(pPacket->size >= sizeof(Payload::Guild::PublicNote));

	auto payload = reinterpret_cast<Payload::Guild::PublicNote*>(pPacket->pBuffer);

	String senderName = Utility::safeString(payload->mSenderName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(senderName == mCharacter->getName()); // Check: Sanity
	String targetName = Utility::safeString(payload->mTargetName, Limits::Character::MAX_NAME_LENGTH);
	String note = Utility::safeString(payload->mNote, Limits::Guild::MAX_PUBLIC_NOTE_LENGTH);

	// Changing the note of someone else, check permission.
	if (targetName != senderName)
		EXPECTED(GuildManager::getInstance().isLeader(mCharacter) || GuildManager::getInstance().isOfficer(mCharacter));

	GuildManager::getInstance().handleSetPublicNote(mCharacter, targetName, note);
}

void ZoneClientConnection::_handleGetGuildStatus(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GuildStatus_Struct));

	auto payload = reinterpret_cast<GuildStatus_Struct*>(pPacket->pBuffer);

	String targetName = Utility::safeString(payload->Name, Limits::Character::MAX_NAME_LENGTH);
	
	// NOTE: UF does not prevent the player from sending smaller than possible names.
	if (Limits::Character::nameLength(targetName) == false)
		return;
	
	GuildManager::getInstance().handleStatusRequest(mCharacter, targetName);
}

void ZoneClientConnection::_handleGuildDemote(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(Demote::sizeCheck(pPacket->size));

	auto payload = Demote::convert(pPacket->pBuffer);

	String characterName = Utility::safeString(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(characterName));
	EXPECTED(characterName == mCharacter->getName()); // Check: Sanity
	String demoteName = Utility::safeString(payload->mDemoteName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(demoteName));

	// Check: Permission (Only guild leader can demote others).
	if (characterName != demoteName)
		EXPECTED(GuildManager::getInstance().isLeader(mCharacter));

	// Check: Leader can not self-demote.
	if (characterName == demoteName)
		EXPECTED(GuildManager::getInstance().isLeader(mCharacter) == false);

	GuildManager::getInstance().handleDemote(mCharacter, demoteName);
}

void ZoneClientConnection::_handleGuildBanker(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(BankerAltStatus::sizeCheck(pPacket->size));

	auto payload = BankerAltStatus::convert(pPacket->pBuffer);

	// NOTE: UF does not send BankerAltStatus::mCharacterName like other packets. /shrug
	String otherName = Utility::safeString(payload->mOtherName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(otherName));

	bool banker = payload->mStatus & 0x01;
	bool alt = payload->mStatus & 0x02;

	GuildManager::getInstance().handleSetBanker(mCharacter, otherName, banker);
	GuildManager::getInstance().handleSetAlt(mCharacter, otherName, alt);
}

void ZoneClientConnection::_handleGuildMakeLeader(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(GuildManager::getInstance().isLeader(mCharacter));
	EXPECTED(MakeLeader::sizeCheck(pPacket->size));

	auto payload = MakeLeader::convert(pPacket->pBuffer);

	String characterName = Utility::safeString(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(characterName));
	EXPECTED(mCharacter->getName() == characterName); // Check: Sanity.
	String leaderName = Utility::safeString(payload->mLeaderName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(leaderName));

	GuildManager::getInstance().handleMakeLeader(mCharacter, leaderName);
}

void ZoneClientConnection::_unimplementedFeature(String pOpCodeName)
{
}

void ZoneClientConnection::_handleFaceChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(FaceChange::sizeCheck(pPacket->size));

	auto payload = FaceChange::convert(pPacket->pBuffer);

	// TODO: Validation of values?

	mCharacter->setHairColour(payload->mHairColour);
	mCharacter->setBeardColour(payload->mBeardColour);
	mCharacter->setLeftEyeColour(payload->mLeftEyeColour);
	mCharacter->setRightEyeColour(payload->mRightEyeColour);
	mCharacter->setHairStyle(payload->mHairStyle);
	mCharacter->setBeardStyle(payload->mBeardStyle);
	mCharacter->setFaceStyle(payload->mFaceStyle);
	mCharacter->setDrakkinHeritage(payload->mDrakkinHeritage);
	mCharacter->setDrakkinTattoo(payload->mDrakkinTattoo);
	mCharacter->setDrakkinDetails(payload->mDrakkinDetails);

	// Notify Zone.
	mZone->handleFaceChange(mCharacter);
}

void ZoneClientConnection::sendWearChange(const uint16 pSpawnID, const uint8 pSlotID, const uint32 pMaterialID, const uint32 pColour) {
	using namespace Payload::Zone;
	auto outPacket = new EQApplicationPacket(OP_WearChange, WearChange::size());
	auto payload = WearChange::convert(outPacket->pBuffer);

	payload->mSpawnID = pSpawnID;
	payload->mSlotID = pSlotID;
	payload->mMaterialID = pMaterialID;
	payload->mColour.mColour = pColour;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleAutoAttack(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AutoAttack::sizeCheck(pPacket));

	auto payload = AutoAttack::convert(pPacket->pBuffer);
	mCharacter->setAutoAttack(payload->mAttacking);
}

void ZoneClientConnection::_handleMemoriseSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(MemoriseSpell::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isCaster()); // Check: Sanity- This class can cast spells.

	auto payload = MemoriseSpell::convert(pPacket->pBuffer);

	switch (payload->mAction){
	case MemoriseSpell::SCRIBE:
		EXPECTED(mCharacter->handleScribeSpell(payload->mSlot, payload->mSpellID));
		break;
	case MemoriseSpell::MEMORISE:
		EXPECTED(mCharacter->handleMemoriseSpell(payload->mSlot, payload->mSpellID));
		break;
	case MemoriseSpell::UNMEMORISE:
		EXPECTED(mCharacter->handleUnmemoriseSpell(payload->mSlot));
		break;
	default:
		Log::error("Unknown action in _handleMemoriseSpell from " + Utility::characterLogDetails(mCharacter));
		break;
	}
}

void ZoneClientConnection::_handleDeleteSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(DeleteSpell::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isCaster()); // Check: Sanity- This class can cast spells.

	auto payload = DeleteSpell::convert(pPacket->pBuffer);
	const bool success = mCharacter->handleDeleteSpell(payload->mSpellBookSlot);
	sendDeleteSpellDelete(payload->mSpellBookSlot, success);
}

void ZoneClientConnection::_handleLoadSpellSet(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(LoadSpellSet::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isCaster()); // Check: Sanity- This class can cast spells.

	auto payload = LoadSpellSet::convert(pPacket->pBuffer);
}

void ZoneClientConnection::_handleSwapSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(SwapSpell::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isCaster()); // Check: Sanity- This class can cast spells.

	auto payload = SwapSpell::convert(pPacket->pBuffer);
	EXPECTED(mCharacter->handleSwapSpells(payload->mFrom, payload->mTo));

	// Client requires a reply.
	mStreamInterface->QueuePacket(pPacket);
}

void ZoneClientConnection::_handleCastSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(CastSpell::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isCasting() == false);
	
	auto payload = CastSpell::convert(pPacket->pBuffer);

	if (payload->mInventorySlot == 0xFFFF)
		EXPECTED(mCharacter->isCaster());

	// Check: Target validity
	if (mCharacter->hasTarget()) {
		// Match spawn IDs
		EXPECTED(payload->mTargetID == mCharacter->getTarget()->getSpawnID());
	}
	else {
		// Expect both zero.
		EXPECTED(payload->mTargetID == 0);
	}

	EXPECTED(Limits::SpellBar::slotValid(payload->mSlot));
	EXPECTED(Limits::SpellBar::spellIDValid(payload->mSpellID));

	// Casting from Spell Bar.
	mZone->handleCastingBegin(mCharacter, payload->mSlot, payload->mSpellID);
}

void ZoneClientConnection::_handleCombatAbility(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(CombatAbility::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->hasTarget());

	auto payload = CombatAbility::convert(pPacket->pBuffer);

	EXPECTED(payload->mTargetID == mCharacter->getTarget()->getSpawnID());
}

void ZoneClientConnection::_handleTaunt(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Taunt::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->hasTarget());
	EXPECTED(mCharacter->getTarget()->isNPC());
	EXPECTED(mCharacter->canTaunt());

	auto payload = Taunt::convert(pPacket->pBuffer);

	EXPECTED(payload->mSpawnID == mCharacter->getTarget()->getSpawnID());
}

void ZoneClientConnection::_handleConsider(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Consider::sizeCheck(pPacket));
	
	auto payload = Consider::convert(pPacket);
	Log::info(payload->_debug());
	mZone->handleConsider(mCharacter, payload->mTargetSpawnID);
}

void ZoneClientConnection::_handleConsiderCorpse(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Consider::sizeCheck(pPacket));

	auto payload = Consider::convert(pPacket);
	Log::info(payload->_debug());
	mZone->handleConsiderCorpse(mCharacter, payload->mTargetSpawnID);
}

void ZoneClientConnection::_handleSurname(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Surname::sizeCheck(pPacket));
	EXPECTED(mCharacter->getLevel() >= Limits::Character::MIN_LEVEL_SURNAME); // Hacker!

	auto payload = Surname::convert(pPacket);

	String characterName = Utility::safeString(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(characterName));
	EXPECTED(characterName == mCharacter->getName());

	String lastName = Utility::safeString(payload->mLastName, Limits::Character::MAX_LAST_NAME_LENGTH);
	EXPECTED(Limits::Character::surnameLengthClient(lastName)); // Match Client check.
	// TODO: Check for special characters / Captialisation.

	// Update Character.
	mCharacter->setLastName(lastName);
	sendSurnameApproval(true);
	// Update Zone.
	mZone->handleSurnameChange(mCharacter);
	
}

void ZoneClientConnection::sendSurnameApproval(const bool pSuccess) {
	// NOTE: This packet notifies the client that their surname was approved.
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = Surname::construct(pSuccess ? 1 : 0, mCharacter->getName(), mCharacter->getLastName());
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::_handleGMLastName(const EQApplicationPacket* pPacket) {
	mCharacter->notify("Please use the command system.");
}

void ZoneClientConnection::_handleClearSurname(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	// Update Character.
	mCharacter->setLastName("");
	// Update Zone.
	mZone->handleSurnameChange(mCharacter);
}

void ZoneClientConnection::_handleSetTitle(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(SetTitle::sizeCheck(pPacket->size));
	
	auto payload = SetTitle::convert(pPacket->pBuffer);

	EXPECTED(payload->mOption == SetTitle::SET_TITLE || payload->mOption == SetTitle::SET_SUFFIX);

	// TODO: Check eligibility

	// Prefix changing.
	if (payload->mOption == SetTitle::SET_TITLE) {
		String prefix = "";

		// NOTE: Where mTitleID = 0 the player has pressed the 'Clear Title' button.
		if (payload->mTitleID != 0)
			prefix = TitleManager::getInstance().getPrefix(payload->mTitleID);

		// Update Character.
		mCharacter->setTitle(prefix);
	}

	// Suffix changing.
	if (payload->mOption == SetTitle::SET_SUFFIX) {
		String suffix = "";

		// NOTE: Where mTitleID = 0 the player has pressed the 'Clear Suffix' button.
		if (payload->mTitleID != 0)
			suffix = TitleManager::getInstance().getSuffix(payload->mTitleID);

		// Update Character.
		mCharacter->setSuffix(suffix);
	}

	// Update Zone.
	const uint32 t = payload->mOption == SetTitle::SET_TITLE ? TitleOption::Title : TitleOption::Suffix;
	mZone->handleTitleChanged(mCharacter, t);
	
}

void ZoneClientConnection::_handleRequestTitles(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	auto availableTitles = TitleManager::getInstance().getTitles(mCharacter);
	if (availableTitles.empty())
		return;

	int payloadSize = sizeof(uint32); // 4 bytes: store the number of titles.
	for (auto i : availableTitles) {
		payloadSize += sizeof(uint32); // Title ID
		payloadSize += i->mPrefix.length() + 1; // +1 for null terminator.
		payloadSize += i->mSuffix.length() + 1; // +1 for null terminator.
	}

	auto outPacket = new EQApplicationPacket(OP_SendTitleList, payloadSize);
	Utility::DynamicStructure ds(outPacket->pBuffer, payloadSize);
	
	ds.write<uint32>(availableTitles.size());
	for (auto i : availableTitles) {
		ds.write<uint32>(i->mID);
		ds.writeString(i->mPrefix);
		ds.writeString(i->mSuffix);
	}

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
	EXPECTED(ds.check());
}

void ZoneClientConnection::sendDeleteSpellDelete(const uint16 pSlot, const bool pSuccess) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_DeleteSpell, DeleteSpell::size());
	auto payload = DeleteSpell::convert(outPacket->pBuffer);
	payload->mSpellBookSlot = pSlot;
	payload->mSuccess = pSuccess ? 1 : 0;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID, const uint32 pAction) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_MemorizeSpell, MemoriseSpell::size());
	auto payload = MemoriseSpell::convert(outPacket->pBuffer);
	payload->mAction = static_cast<MemoriseSpell::Action>(pAction);
	payload->mSlot = pSlot;
	payload->mSpellID = pSpellID;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpell::MEMORISE);
}

void ZoneClientConnection::sendUnmemoriseSpell(const uint16 pSlot) {
	_sendMemoriseSpell(pSlot, 0, Payload::Zone::MemoriseSpell::UNMEMORISE);
}

void ZoneClientConnection::sendInterruptCast() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_InterruptCast, InterruptCast::size());
	auto payload = InterruptCast::convert(outPacket->pBuffer);
	payload->mSpawnID = mCharacter->getSpawnID();
	payload->mMessageID = static_cast<uint32>(StringID::INTERRUPT_SPELL);

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendRefreshSpellBar(const uint16 pSlot, const uint32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpell::SPELLBAR_REFRESH);
}

void ZoneClientConnection::sendEnableSpellBar(const uint32 pSpellID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ManaChange, ManaChange::size());
	auto payload = ManaChange::convert(outPacket->pBuffer);
	payload->mMana = mCharacter->getCurrentMana();
	payload->mEndurance = mCharacter->getCurrentEndurance();
	payload->mSpellID = pSpellID;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendSkillValue(const uint32 pSkillID, const uint32 pValue) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_SkillUpdate, SkillUpdate::size());
	auto payload = SkillUpdate::convert(outPacket->pBuffer);
	payload->mID = pSkillID;
	payload->mValue = pValue;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleBeginLootRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(LootBeginRequest::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isLooting() == false);

	auto payload = LootBeginRequest::convert(pPacket->pBuffer);
	const uint32 corpseSpawnID = payload->mSpawnID;
	mZone->handleBeginLootRequest(mCharacter, corpseSpawnID);
}

void ZoneClientConnection::_handleEndLootRequest(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->isLooting());

	mZone->handleEndLootRequest(mCharacter);
}

void ZoneClientConnection::sendLootComplete() {
	EXPECTED(mConnected);
	auto outPacket = new EQApplicationPacket(OP_LootComplete, 0);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendLootResponse(uint8 pResponse, uint32 pPlatinum, uint32 pGold, uint32 pSilver, uint32 pCopper) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_MoneyOnCorpse, LootResponse::size());
	auto payload = LootResponse::convert(outPacket->pBuffer);
	payload->mResponse = pResponse;
	payload->mPlatinum = pPlatinum;
	payload->mGold = pGold;
	payload->mSilver = pSilver;
	payload->mCopper = pCopper;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendConsiderResponse(const uint32 pSpawnID, const uint32 pMessage) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = Consider::construct(mCharacter->getSpawnID(), pSpawnID, pMessage);
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneClientConnection::_handleLootItem(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(mCharacter->isLooting());
	EXPECTED(LootItem::sizeCheck(pPacket));
	
	auto payload = LootItem::convert(pPacket);
	Log::info(payload->_debug());

	EXPECTED(payload->mLooterSpawnID == mCharacter->getSpawnID());
	EXPECTED(payload->mCorpseSpawnID == mCharacter->getLootingCorpse()->getSpawnID());

	// Send required reply(echo).
	sendPacket(const_cast<EQApplicationPacket*>(pPacket));

	mZone->handleLootItem(mCharacter, mCharacter->getLootingCorpse(), payload->mSlotID);
}

void ZoneClientConnection::_handleMoveItem(const EQApplicationPacket* pPacket) {
	if (!_handleMoveItemImpl(pPacket)) {
		inventoryError();
	}
}

const bool ZoneClientConnection::_handleMoveItemImpl(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED_BOOL(pPacket);
	EXPECTED_BOOL(MoveItem::sizeCheck(pPacket));

	auto payload = MoveItem::convert(pPacket);
	Log::info("MoveItem: From: " + std::to_string(payload->mFromSlot) + " To: " + std::to_string(payload->mToSlot) + " Stack: " + std::to_string(payload->mStacks));

	Item* wornNew = nullptr; // Item that will be equipped after move.
	Item* wornOld = nullptr; // Item that will be unequipped after move.

	// Character is trying to equip an Item.
	if (SlotID::isWorn(payload->mToSlot)) {
		EXPECTED_BOOL(SlotID::isCursor(payload->mFromSlot)); // We expect the Item to be coming from the cursor.

		wornNew = mCharacter->getInventory()->getItem(payload->mFromSlot);
		EXPECTED_BOOL(wornNew);
		EXPECTED_BOOL(mCharacter->canEquip(wornNew, payload->mToSlot));

		wornOld = mCharacter->getInventory()->getItem(payload->mToSlot); // Can be null if slot was empty.
	}
	// Character is trying to unequip an Item.
	else if (SlotID::isWorn(payload->mFromSlot)) {
		EXPECTED_BOOL(SlotID::isCursor(payload->mToSlot)); // We expect the Item to be going to the cursor.

		wornOld = mCharacter->getInventory()->getItem(payload->mFromSlot);
		EXPECTED_BOOL(wornOld);

		wornNew = mCharacter->getInventory()->getItem(payload->mToSlot); // Can be null if no Item on cursor.
	}

	// Character is trying to put an Item into a trade slot.
	if (SlotID::isTrade(payload->mToSlot)) {
		EXPECTED_BOOL(SlotID::isCursor(payload->mFromSlot)); // We expect the Item to be coming from the cursor.
		EXPECTED_BOOL(mCharacter->isTrading());
		auto item = mCharacter->getInventory()->getItem(payload->mFromSlot);
		EXPECTED_BOOL(item);
		// Check: Can not move stackable items with more than 1 stack.
		if (item->isStackable()) { EXPECTED_BOOL(item->getStacks() == 1); }
	}

	// Move.
	EXPECTED_BOOL(mCharacter->getInventory()->moveItem(payload->mFromSlot, payload->mToSlot, payload->mStacks));

	// Notify Character that a worn slot has changed.
	if (SlotID::isWorn(payload->mToSlot)) {
		EXPECTED_BOOL(mCharacter->onWornSlotChange(payload->mToSlot, wornOld, wornNew));
	}
	else if (SlotID::isWorn(payload->mFromSlot)) {
		EXPECTED_BOOL(mCharacter->onWornSlotChange(payload->mFromSlot, wornOld, wornNew));
	}

	return true;
}

void ZoneClientConnection::_handleConsume(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Consume::sizeCheck(pPacket->size));

	auto payload = Consume::convert(pPacket->pBuffer);

	Log::info("Consume from slot: " + std::to_string(payload->mSlot));
	if (!mCharacter->getInventory()->consume(payload->mSlot, 1)) {
		inventoryError();
	}
	
	sendStamina(0, 0);
}

void ZoneClientConnection::sendStamina(const uint32 pHunger, const uint32 pThirst) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	Stamina payload;
	payload.mHunger = pHunger;
	payload.mThirst = pThirst;

	auto outPacket = Stamina::create(payload);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handlePotionBelt(const EQApplicationPacket* pPacket) {
}

void ZoneClientConnection::_handleItemRightClick(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ItemRightClick::sizeCheck(pPacket->size));

	auto payload = ItemRightClick::convert(pPacket->pBuffer);

	sendItemRightClickResponse(payload->mSlot, payload->mTargetSpawnID);
}

void ZoneClientConnection::sendItemRightClickResponse(const int32 pSlot, const uint32 pTargetSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	ItemRightClickResponse payload;
	payload.mSlot = pSlot;
	payload.mTargetSpawnID = pTargetSpawnID;
	payload.mSpellID = 0;
	
	auto outPacket = ItemRightClickResponse::create(payload);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::inventoryError(){
	sendMessage(MessageType::Red, "Inventory Error. Please inform a GM and relog to prevent loss of items!");
	Log::error("Inventory Error");
}

void ZoneClientConnection::_handleOpenContainer(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(OpenContainer::sizeCheck(pPacket->size));

	auto payload = OpenContainer::convert(pPacket->pBuffer);
	Log::info("Open Container: " + std::to_string(payload->mSlot));
}

void ZoneClientConnection::_handleTradeRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeRequest::sizeCheck(pPacket));

	auto payload = TradeRequest::convert(pPacket);

	EXPECTED(payload->mFromSpawnID == mCharacter->getSpawnID());

	// OP_TradeRequestAck

	// NOTE: Underfoot does appear to require a response.

	mZone->handleTradeRequest(mCharacter, payload->mToSpawnID);
}

void ZoneClientConnection::_handleTradeRequestAck(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeRequest::sizeCheck(pPacket));

	auto payload = TradeRequest::convert(pPacket);
}

void ZoneClientConnection::sendTradeRequest(const uint32 pFromSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	TradeRequest payload;
	payload.mToSpawnID = mCharacter->getSpawnID();
	payload.mFromSpawnID = pFromSpawnID;

	auto outPacket = TradeRequest::create(payload);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleCancelTrade(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeCancel::sizeCheck(pPacket));
	
	// NOTE: Underfoot sends this twice when trade is canceled with an NPC
	// TODO: Test whether this occurs when canceling with PC.
	if (mCharacter->isTrading() == false) {
		Log::info("Got OP_CancelTrade while not trading. Ignoring.");
		return;
	}

	auto payload = TradeCancel::convert(pPacket);

	EXPECTED(payload->mFromSpawnID == mCharacter->getSpawnID());
	Log::info(payload->_debug());

	mZone->handleTradeCancel(mCharacter, payload->mToSpawnID);
}

void ZoneClientConnection::_handleAcceptTrade(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeAccept::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isTrading());

	// TODO: I can check that mCharacter is trading with the specific Actor.

	auto payload = TradeAccept::convert(pPacket->pBuffer);
	Log::info(payload->_debug());

	mZone->handleTradeAccept(mCharacter, payload->mFromSpawnID);

	//sendTradeFinished();
	//mCharacter->setTrading(false);
	// TODO: Consume trade items.
}

void ZoneClientConnection::_handleTradeBusy(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeBusy::sizeCheck(pPacket->size));

	auto payload = TradeBusy::convert(pPacket->pBuffer);
}

void ZoneClientConnection::_handleSetServerFiler(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ServerFilter::sizeCheck(pPacket));

	auto payload = ServerFilter::convert(pPacket->pBuffer);

	mCharacter->setFilters(payload->mFilters);
}

void ZoneClientConnection::_handleItemLinkClick(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ItemLink::sizeCheck(pPacket));

	auto payload = ItemLink::convert(pPacket);

	// Retrieve base ItemData
	auto itemData = ItemDataStore::getInstance().get(payload->mItemID);
	EXPECTED(itemData);

	Item* item = new Item(itemData);

	item->setCurrentEvolvingLevel(payload->mCurrentEvolvingLevel);
	item->setOrnamentationIcon(payload->mOrnamentationIcon);
	
	// NOTE: Untested!
	// Add required augmentations.
	for (auto i = 0; i < 5; i++) {
		if (payload->mAugments[i] != 0) {
			// Retrieve augmentation ItemData.
			auto augmentationItemData = ItemDataStore::getInstance().get(payload->mAugments[i]);
			EXPECTED(augmentationItemData);

			// NOTE: Memory is freed when item is deleted.
			item->setAugmentation(i, new Item(augmentationItemData));
		}
	}

	sendItemView(item);
	delete item;
}

void ZoneClientConnection::_handleItemView(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	Log::info("Got OP_ItemViewUnknown. Size=" + std::to_string(pPacket->size));
}

void ZoneClientConnection::_handleMoveCoin(const EQApplicationPacket* pPacket) {
	if (!_handleMoveCoinImpl(pPacket)) {
		inventoryError();
	}
}

const bool ZoneClientConnection::_handleMoveCoinImpl(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED_BOOL(pPacket);
	EXPECTED_BOOL(MoveCoin::sizeCheck(pPacket));

	auto payload = MoveCoin::convert(pPacket);

	// Sanitise.
	EXPECTED_BOOL(Limits::General::moneySlotIDValid(payload->mFromSlot));
	EXPECTED_BOOL(Limits::General::moneySlotIDValid(payload->mToSlot));
	EXPECTED_BOOL(Limits::General::moneyTypeValid(payload->mFromType));
	EXPECTED_BOOL(Limits::General::moneyTypeValid(payload->mToType));

	// Sanity.
	EXPECTED_BOOL(MoneySlotID::isTrade(payload->mFromSlot) == false); // UF does not move currency from trade slots.
	EXPECTED_BOOL(payload->mAmount > 0); // There is no natural way for UF to send 0/negative values.

	auto isBankingSlot = [](const uint32 pSlot) { return pSlot == MoneySlotID::BANK || pSlot == MoneySlotID::SHARED_BANK; };
	const bool isConversion = payload->mFromType != payload->mToType;
	const bool isBankSlot = isBankingSlot(payload->mFromSlot) || isBankingSlot(payload->mToSlot);
	const bool bankRequired = isConversion || isBankSlot;

	// Check: Currency moving into Shared Bank is platinum only.
	if (MoneySlotID::isSharedBank(payload->mToSlot)) {
		EXPECTED_BOOL(payload->mToType == MoneyType::PLATINUM);
	}
	// Check: Currency moving from Shared Bank is platinum only.
	if (MoneySlotID::isSharedBank(payload->mFromSlot)) {
		EXPECTED_BOOL(payload->mToType == MoneyType::PLATINUM);
	}

	// Check: Character is in range of a bank.
	if (bankRequired && !mZone->canBank(mCharacter)) {
		// TODO: Cheater.
		mCharacter->notify("Cheat Detected! Expect to be contacted shortly.");
	}

	// Check: Character is trading.
	if (MoneySlotID::isTrade(payload->mToSlot)) {
		EXPECTED_BOOL(mCharacter->isTrading());
	}

	// Move.
	EXPECTED_BOOL(mCharacter->getInventory()->moveCurrency(payload->mFromSlot, payload->mToSlot, payload->mFromType, payload->mToType, payload->mAmount));

	return true;
}

void ZoneClientConnection::sendMoneyUpdate() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	const int32 platinum = mCharacter->getInventory()->getPersonalPlatinum();
	const int32 gold = mCharacter->getInventory()->getPersonalGold();
	const int32 silver = mCharacter->getInventory()->getPersonalSilver();
	const int32 copper = mCharacter->getInventory()->getPersonalCopper();
	
	auto packet = MoneyUpdate::construct(platinum, gold, silver, copper);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::_handleCrystalCreate(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(CrystalCreate::sizeCheck(pPacket));

	// NOTE: Underfoot will allow crystals to be summoned to the cursor even when there are Items on it.
	// This is to keep things simple.
	if (mCharacter->getInventory()->isCursorEmpty() == false) {
		mCharacter->notify("Please clear your cursor and try again.");
		return;
	}

	auto payload = CrystalCreate::convert(pPacket);
	EXPECTED(payload->mType == CrystalCreate::RADIANT || payload->mType == CrystalCreate::EBON);
	uint32 itemID = 0;

	uint32 stacks = payload->mAmount;
	String crystalName;

	// Creating Radiant Crystals.
	if (payload->mType == CrystalCreate::RADIANT) {
		itemID = ItemID::RadiantCrystal;
		crystalName = "Radiant Crystals.";
		// Adjust stacks to prevent over-stacking.
		stacks = stacks > MaxRadiantCrystalsStacks ? MaxRadiantCrystalsStacks : stacks;
		// Check: Not trying to create more than is possible.
		EXPECTED(stacks <= mCharacter->getInventory()->getRadiantCrystals());
		// Remove crystals from Character.
		EXPECTED(mCharacter->getInventory()->removeRadiantCrystals(stacks));
	}
	// Creating Ebon Crystals.
	else if (payload->mType == CrystalCreate::EBON) {
		itemID = ItemID::EbonCrystal;
		crystalName = "Ebon Crystals.";
		// Adjust stacks to prevent over-stacking.
		stacks = stacks > MaxEbonCrystalsStacks ? MaxEbonCrystalsStacks : stacks;
		// Check: Not trying to create more than is possible.
		EXPECTED(stacks <= mCharacter->getInventory()->getEbonCrystals());
		// Remove crystals from Character.
		EXPECTED(mCharacter->getInventory()->removeEbonCrystals(stacks));
		
	}

	auto item = ItemFactory::make(itemID, stacks);
	EXPECTED(item);
	mCharacter->getInventory()->pushCursor(item);
	sendItemSummon(item);

	// Notify user.
	mCharacter->notify("You have created (" + std::to_string(stacks) + ") " + crystalName);

	// Update client.
	sendCrystals();
}

void ZoneClientConnection::_handleCrystalReclaim(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);

	// Check: Character has Radiant or Ebon crystals on cursor.
	Item* item = mCharacter->getInventory()->peekCursor();
	if (!item || (item->getID() != ItemID::RadiantCrystal && item->getID() != ItemID::EbonCrystal)) {
		mCharacter->notify("Please place crystals on your cursor and try again.");
		return;
	}

	const uint32 stacks = item->getStacks();
	String crystalName;

	// Add Radiant Crystals.
	if (item->getID() == ItemID::RadiantCrystal) {
		crystalName = "Radiant Crystals.";
		// Consume from Inventory.
		EXPECTED(mCharacter->getInventory()->consume(SlotID::CURSOR, stacks));
		mCharacter->getInventory()->addRadiantCrystals(stacks);
	}
	// Add Ebon Crystals.
	else if (item->getID() == ItemID::EbonCrystal) {
		crystalName = "Ebon Crystals.";
		// Consume from Inventory.
		EXPECTED(mCharacter->getInventory()->consume(SlotID::CURSOR, stacks));
		mCharacter->getInventory()->addEbonCrystals(stacks);
	}

	// Clear cursor.
	sendMoveItem(SlotID::CURSOR, SlotID::SLOT_DELETE);

	// Notify user.
	mCharacter->notify("You have reclaimed (" + std::to_string(stacks) + ") " + crystalName);

	// Update client.
	sendCrystals();
}

void ZoneClientConnection::sendCrystals() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	CrystalUpdate payload;
	payload.mRadiantCrystals = mCharacter->getInventory()->getRadiantCrystals();
	payload.mTotalRadiantCrystals = mCharacter->getInventory()->getTotalRadiantCrystals();
	payload.mEbonCrystals = mCharacter->getInventory()->getEbonCrystals();
	payload.mTotalEbonCrystals = mCharacter->getInventory()->getTotalEbonCrystals();

	auto packet = CrystalUpdate::create(payload);
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneClientConnection::_handleUnknown(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	Log::info("Unknown Packet, size=" + std::to_string(pPacket->size));
	auto raw = static_cast<EQRawApplicationPacket*>(const_cast<EQApplicationPacket*>(pPacket));
	Log::info("OpCode= " + std::to_string(raw->GetRawOpcode()));
}

void ZoneClientConnection::_handleEnvironmentalDamage(const EQApplicationPacket* pPacket)
{
	sendHPUpdate();
}

void ZoneClientConnection::sendPopup(const String& pTitle, const String& pText) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = PopupWindow::construct(pTitle, pText);
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneClientConnection::_handlePopupResponse(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
}

void ZoneClientConnection::_handleClaimRequest(const EQApplicationPacket* pPacket)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void ZoneClientConnection::sendItemView(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketViewLink);

	auto packet = new EQApplicationPacket(OP_ItemLinkResponse, data, payloadSize);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendItemSummon(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketSummonItem);

	auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendItemTrade(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketTrade);

	auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
	sendPacket(packet);
	delete packet;
}


void ZoneClientConnection::_handleAugmentItem(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AugmentItem::sizeCheck(pPacket));

	auto payload = AugmentItem::convert(pPacket);

	Log::info(payload->_debug());

	auto container = mCharacter->getInventory()->getItem(payload->mContainerSlot);
	EXPECTED(container);
	EXPECTED(container->getContainerType() == ContainerType::AugmentationSealer);
	EXPECTED(container->getContainerSlots() == 2);

	// Insert
	if (payload->mAugmentSlot == -1) {
		auto item0 = container->getContents(0);
		EXPECTED(item0);
		auto item1 = container->getContents(1);
		EXPECTED(item1);

		Item* item = nullptr;
		Item* augment = nullptr;

		// Determine which item is the augment.

		if (item0->getItemType() == ItemType::Augmentation) {
			augment = item0;
			item = item1;
		}
		else {
			EXPECTED(item1->getItemType() == ItemType::Augmentation);
			augment = item1;
			item = item0;
		}

		// (Server) Clear container
		container->clearContents(0);
		container->clearContents(1);
		// (Client) Clear container
		sendDeleteItem(augment->getSlot());
		sendDeleteItem(item->getSlot());

		// Send the augmented item back.
		EXPECTED(item->insertAugment(augment));
		mCharacter->getInventory()->pushCursor(item);

		uint32 payloadSize = 0;
		const unsigned char* data = item->copyData(payloadSize, Payload::ItemPacketSummonItem);

		auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
		sendPacket(packet);
		delete packet;

		return;
	}

	// Remove.
	if (payload->mAugmentSlot == 0) {

	}
}

void ZoneClientConnection::sendDeleteItem(const uint32 pSlot, const uint32 pStacks, const uint32 pToSlot) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = DeleteItem::construct(pSlot, pToSlot, pStacks);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendMoveItem(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStacks) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = MoveItem::construct(pFromSlot, pToSlot, pStacks);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendTradeRequestAcknowledge(const uint32 pToSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = TradeRequestAcknowledge::construct(mCharacter->getSpawnID(), pToSpawnID);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendTradeFinished() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishTrade, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendTradeCancel(const uint32 pToSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	//auto packet = TradeCancel::construct(mCharacter->getSpawnID(), pToSpawnID);
	auto packet = TradeCancel::construct(mCharacter->getSpawnID(), mCharacter->getSpawnID());
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendFinishWindow() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishWindow, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::sendFinishWindow2() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishWindow, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::_handleAugmentInfo(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AugmentInformation::sizeCheck(pPacket));

	auto payload = AugmentInformation::convert(pPacket);
	Log::info(payload->_debug());

	//sendReadBook(payload->mWindow, 0, 2, "You must use a <c \"#FF0000\">monster cock</c> to remove this augment safely.");
}

void ZoneClientConnection::sendReadBook(const uint32 pWindow, const uint32 pSlot, const uint32 pType, const String& pText) {
	EXPECTED(mConnected);

	uint32 size = 0;
	size += sizeof(uint32); // Window
	size += sizeof(uint32); // Slot
	size += sizeof(uint32); // Type
	size += pText.length() + 1;
	size += sizeof(uint32); // unknown
	size += sizeof(uint16); // unknown

	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);

	ds.write<uint32>(pWindow);
	ds.write<uint32>(pSlot);
	ds.write<uint32>(pType);
	ds.write<uint32>(0); // unknown.
	ds.write<uint16>(0); // unknown.
	ds.writeString(pText);

	EXPECTED(ds.check());

	auto packet = new EQApplicationPacket(OP_ReadBook, data, size);
	sendPacket(packet);
	delete packet;

	
}

void ZoneClientConnection::_handleReadBook(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(BookRequest::sizeCheck(pPacket));

	auto payload = BookRequest::convert(pPacket);
	Log::info(payload->_debug());

	// Extract Item ID and Instance ID from text.
	uint32 itemID = 0;
	uint32 itemInstanceID = 0;

	String text(payload->mText);
	std::vector<String> elements = Utility::split(text, '|');
	EXPECTED(elements.size() == 2);

	EXPECTED(Utility::stoSafe(itemID, elements[0]));
	EXPECTED(Utility::stoSafe(itemInstanceID, elements[1]));

	auto item = mCharacter->getInventory()->find(itemID, itemInstanceID);

	if (item) {
		Log::info("Found Item " + item->getName());

		// Check: Found Item is not in a worn slot.
		if (SlotID::isWorn(item->getSlot()) == false) {
			const uint32 slots = item->getSlots();

			// Hardcode.
			if (slots == EquipSlots::Chest) {
				auto currentChestItem = mCharacter->getInventory()->getItem(SlotID::CHEST);
				if (currentChestItem) {
					String comparison;
					currentChestItem->compare(item, comparison);
					sendReadBook(payload->mWindow, payload->mSlot, payload->mType, comparison);
				}
			}
		}
	}
}

void ZoneClientConnection::_handleCombine(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Combine::sizeCheck(pPacket));

	auto payload = Combine::convert(pPacket);
	Log::info(payload->_debug());

	// Check: Character is in a state that allows for combining.
	EXPECTED(mCharacter->canCombine());

	// Check: The combine is occurring from a valid slot id. (UF Client Checked)
	EXPECTED(SlotID::isMainInventory(payload->mSlot));

	// Check: Cursor is empty. (UF Client Checked)
	EXPECTED(mCharacter->getInventory()->isCursorEmpty() == true);

	// Check: Valid Item at slot id.
	auto container = mCharacter->getInventory()->getItem(payload->mSlot);
	EXPECTED(container);

	// Check: Item is a container.
	EXPECTED(container->isContainer());
	
	// Check: Container has 'Combine' option.
	EXPECTED(container->isCombineContainer());

	// Check: Container is not empty. (UF Client Checked)
	EXPECTED(container->isEmpty() == false);

	// Check: None of the Items in the container have > 1 stacks. (UF Client Checked)
	auto f = [](Item* pItem) { return pItem->getStacks() == (pItem->isStackable()) ? 1 : 0; }; // This is annoying. Non-stackable Items should still have getStacks == 1.
	EXPECTED(container->forEachContents(f));

	// Test.
	std::list<Item*> results;
	std::list<Item*> items;
	container->getContents(items);

	if (container->getID() == ItemID::TransmuterTen) {
		Item* item = Transmutation::getInstance().transmute(items);
		if (item) results.push_back(item);
	}

	for (auto i : items) {
		// Update Client.
		sendDeleteItem(i->getSlot());
		// Update Inventory.
		EXPECTED(container->clearContents(i->getSubIndex()));
		// Free memory.
		delete i;
	}
	items.clear();

	EXPECTED(container->isEmpty()); // We expect the container to be empty at this point.

	// Note: Underfoot locks the UI until a reply is received.
	sendCombineReply();

	for (auto i : results) {
		mCharacter->getInventory()->pushCursor(i);
		mCharacter->getConnection()->sendItemSummon(i);
	}
}

void ZoneClientConnection::sendCombineReply() {
	EXPECTED(mConnected);
	auto packet = new EQApplicationPacket(OP_TradeSkillCombine, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::_handleShopRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(MerchantRequest::sizeCheck(pPacket));

	// Check: Character is in a state that allows for shopping.
	EXPECTED(mCharacter->canShop());

	auto payload = MerchantRequest::convert(pPacket);
	Log::info(payload->_debug());

	EXPECTED(mCharacter->getSpawnID() == payload->mCharacterSpawnID); // Sanity.

	mZone->handleShopRequest(mCharacter, payload->mNPCSpawnID);
}

void ZoneClientConnection::sendShopRequestReply(const uint32 pNPCSpawnID, const uint32 pAction, const float pRate) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = MerchantRequest::construct(pNPCSpawnID, mCharacter->getSpawnID(), pAction, pRate);
	sendPacket(packet);
	delete packet;
}

void ZoneClientConnection::_handleShopEnd(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(MerchantEnd::sizeCheck(pPacket));

	auto payload = MerchantEnd::convert(pPacket);

	EXPECTED(payload->mCharacterSpawnID == mCharacter->getSpawnID()); // Sanity.
	mZone->handleShopEnd(mCharacter, payload->mNPCSpawnID);
}

void ZoneClientConnection::sendShopEndReply() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_ShopEndConfirm, 0);
	sendPacket(packet);
	delete packet;
}

//
//void ZoneClientConnection::sendSpellCastOn() {
//	using namespace Payload::Zone;
//	EXPECTED(mConnected);
//
//	auto outPacket = new EQApplicationPacket(OP_Action, Action::size());
//	auto payload = Action::convert(outPacket->pBuffer);
//
//	payload->mTargetSpawnID = mCharacter->getSpawnID();
//	payload->mSourceSpawnID = payload->mTargetSpawnID;
//	payload->mSpellID = 1000;
//	payload->mType = 231; // Spell = 231
//	payload->buff_unknown = 0;
//
//	mStreamInterface->QueuePacket(outPacket);
//	safe_delete(outPacket);
//}
