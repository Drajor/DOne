#include "ZoneConnection.h"
#include "ServiceLocator.h"
#include "GroupManager.h"
#include "RaidManager.h"
#include "GuildManager.h"
#include "TitleManager.h"
#include "Zone.h"
#include "ZoneManager.h"
#include "ZoneData.h"
#include "Character.h"
#include "Inventory.h"
#include "LogSystem.h"
#include "IDataStore.h"
#include "Data.h"
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
#include "Random.h"
#include "AlternateCurrencyManager.h"
#include "Object.h"
#include "ExtendedTargetController.h"
#include "RespawnOptions.h"
#include "ExperienceController.h"

#include "../common/MiscFunctions.h"
#include "../common/packet_dump_file.h"

EQApplicationPacket* ZoneConnection::mPlayerProfilePacket = nullptr;
EQApplicationPacket* ZoneConnection::mGroupJoinPacket = nullptr;
EQApplicationPacket* ZoneConnection::mGroupLeavePacket = nullptr;
EQApplicationPacket* ZoneConnection::mGroupDisbandPacket = nullptr;
EQApplicationPacket* ZoneConnection::mGroupLeaderChangePacket = nullptr;
EQApplicationPacket* ZoneConnection::mGroupUpdateMembersPacket = nullptr;

ZoneConnection::ZoneConnection(EQStreamInterface* pStreamInterface, Zone* pZone, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager) :
mZone(pZone),
mStreamInterface(pStreamInterface),
mGroupManager(pGroupManager),
mRaidManager(pRaidManager),
mGuildManager(pGuildManager),
mConnected(true)
{
	mForceSendPositionTimer.Disable();
}

ZoneConnection::~ZoneConnection() {
	dropConnection();
	mStreamInterface->ReleaseFromUse();
	// NOTE: mStreamInterface is intentionally not deleted here.
}

void ZoneConnection::_initalise() {
	mPlayerProfilePacket = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));
	mGroupJoinPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	mGroupLeavePacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	mGroupDisbandPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
	mGroupLeaderChangePacket = new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));
	mGroupUpdateMembersPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
}

void ZoneConnection::_deinitialise() {
	safe_delete(mPlayerProfilePacket);
	safe_delete(mGroupJoinPacket);
	safe_delete(mGroupLeavePacket);
	safe_delete(mGroupDisbandPacket);
	safe_delete(mGroupLeaderChangePacket);
	safe_delete(mGroupUpdateMembersPacket);
}

bool ZoneConnection::isConnected() {
	return mConnected && mStreamInterface->CheckState(ESTABLISHED);
}


void ZoneConnection::update() {
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

void ZoneConnection::dropConnection() {
	mConnected = false;
	mStreamInterface->Close();
}

bool ZoneConnection::_handlePacket(const EQApplicationPacket* pPacket) {
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
		// NOTE: This occurs when the user clicks the 'Auto Add Hater Targets' from the 'Extended Target' window.
		_handleXTargetAutoAddHaters(pPacket);
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
		_handleWhoRequest(pPacket);
		break;
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
	case OP_ShopPlayerSell:
		// NOTE: This occurs when a player tries to sell an Item to a merchant.
		_handleShopSell(pPacket);
		break;
	case OP_ShopPlayerBuy:
		// NOTE: This occurs when a player tries to buy an Item from a merchant.
		_handleShopBuy(pPacket);
		break;
	case OP_AltCurrencyReclaim:
		// NOTE: This occurs when a player presses the 'Reclaim' button in the 'Alt. Currency' tab (Inventory window).
		// NOTE: This occurs when a player presses the 'Create' button in the 'Alt. Currency' tab (Inventory window).
		_handleAlternateCurrencyReclaim(pPacket);
		break;
	case OP_RandomReq:
		// NOTE: This occurs when a player enters the /random command.
		_handleRandomRequest(pPacket);
		break;
	case OP_GroundSpawn:
		// NOTE: This occurs when a player drops an Item on the ground.
		_handleDropItem(pPacket);
		break;
	case OP_RespawnWindow:
		// NOTE: This occurs when a player selects a respawn option from the 'Respawn Window'.
		_handleRespawnWindowSelect(pPacket);
		break;
	case OP_AAAction:
		// NOTE: This occurs when a player adjusts the 'Exp to AA' setting in the 'Alternate Advancement Window'.
		_handleAAAction(pPacket);
		break;
	case OP_LeadershipExpToggle:
		// NOTE: This occurs when a player uses the 'Leadership Exp' toggle in the 'Leadership Window'
		_handleLeadershipExperienceToggle(pPacket);
		break;
	default:
		StringStream ss;
		ss << "Unknown Packet: " << opcode;
		Utility::print(ss.str());
		break;
	}
	return true;
}

void ZoneConnection::_handleZoneEntry(const EQApplicationPacket* pPacket) {
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
	mCharacter = ServiceLocator::getZoneManager()->getZoningCharacter(characterName);
	EXPECTED(mCharacter);
	EXPECTED(ServiceLocator::getZoneManager()->removeZoningCharacter(mCharacter->getName()));

	// Check: Character authenticated for this Zone.
	EXPECTED(mZone->checkAuthentication(mCharacter));
	// Clear ZoneChange
	mCharacter->clearZoneChange();

	mCharacter->setZone(mZone);
	mCharacter->setSpawnID(mZone->getNextSpawnID());
	mCharacter->setConnection(this);

	// REPLY
	_sendGuildNames();
	// OP_PlayerProfile
	_sendPlayerProfile();
	mZoneConnectionStatus = ZoneConnectionStatus::PlayerProfileSent;
	// OP_ZoneEntry
	sendZoneEntry();
	// Bulk Spawns
	_sendZoneSpawns();
	// Corpses Bulk
	// PvP updates?
	// OP_TimeOfDay
	_sendTimeOfDay();
	// Tributes
	_sendTributeUpdate();
	// Inventory
	_sendInventory();

	// Tasks
	// XTargets
	// Weather
	
	_sendWeather();
	mZoneConnectionStatus = ZoneConnectionStatus::ZoneInformationSent;
}

void ZoneConnection::_sendTimeOfDay() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
	auto payload = (TimeOfDay_Struct*)packet->pBuffer;
	memset(payload, 0, sizeof(TimeOfDay_Struct)); // TODO:

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendPlayerProfile() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));
	auto payload = reinterpret_cast<PlayerProfile_Struct*>(packet->pBuffer);
	*payload = { 0 }; // Clear memory.

	// The entityid field in the Player Profile is used by the Client in relation to Group Leadership AA // TODO: How?

	strncpy(payload->name, mCharacter->getName().c_str(), Limits::Character::MAX_NAME_LENGTH);
	strncpy(payload->last_name, mCharacter->getLastName().c_str(), Limits::Character::MAX_LAST_NAME_LENGTH);
	payload->gender = mCharacter->getGender();
	payload->race = mCharacter->getRace();
	payload->class_ = mCharacter->getClass();
	payload->level = mCharacter->getLevel();

	// Bind Locations.
	for (auto i = 0; i < 5; i++) {
		auto bindLocation = mCharacter->getBindLocation(i);
		payload->binds[i].zoneId = bindLocation.getZoneID();
		payload->binds[i].x = bindLocation.getPosition().x;
		payload->binds[i].y = bindLocation.getPosition().y;
		payload->binds[i].z = bindLocation.getPosition().z;
		payload->binds[i].heading = bindLocation.getHeading();
	}

	auto expController = mCharacter->getExperienceController();

	payload->aapoints = expController->getUnspentAAPoints();
	payload->aapoints_spent = expController->getSpentAAPoints(); // not working.
	
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
	payload->exp = mCharacter->getExperienceController()->getExperience();
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

	mCharacter->setLanguage(LanguageID::CommonTongue, 100);
	mCharacter->setLanguage(LanguageID::Barbarian, 100);

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

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendZoneEntry() {
	EXPECTED(mConnected);

	mCharacter->_syncPosition();

	uint32 size = mCharacter->getDataSize();
	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);
	EXPECTED(mCharacter->copyData(ds));
	
	auto packet = new EQApplicationPacket(OP_ZoneEntry, data, size);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendZoneSpawns() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_ZoneSpawns, 0, 0);
	
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendTributeUpdate() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_TributeUpdate, sizeof(TributeInfo_Struct));
	auto payload = (TributeInfo_Struct *)packet->pBuffer;
	
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendInventory() {
	EXPECTED(mConnected);

	auto inventory = mCharacter->getInventory();
	EXPECTED(inventory);

	uint32 size = 0;
	auto data = inventory->getData(size);
	EXPECTED(data);
	EXPECTED(size >= 4); // 4 bytes = empty inventory.

	auto packet = new EQApplicationPacket(OP_CharInventory, data, size);
	sendPacket(packet);
	safe_delete(packet);

	// Send the remaining cursor Items.
	std::list<Item*> cursorItems = mCharacter->getInventory()->getCursor();
	for (auto i = cursorItems.begin(); i != cursorItems.end(); i++) {
		if (i == cursorItems.begin())
			continue; // First cursor Item is sent with OP_CharInventory.

		sendItemSummon(*i);
	}
}

void ZoneConnection::_sendWeather() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = Weather::construct(0, 0, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleRequestClientSpawn(const EQApplicationPacket* pPacket) {
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

	// Alternate Currency
	sendAlternateCurrencies();
	sendAlternateCurrencyQuantities(false);

	sendMOTD("Welcome to Fading Light.");

	//if (mCharacter->hasGuild()) {
	//	mGuildManager->onEnterZone(mCharacter);
	//}
	//if (mCharacter->hasGroup()) {
	//	GroupManager::getInstance().onEnterZone(mCharacter);
	//}
	//if (mCharacter->hasRaid()) {
	//	RaidManager::getInstance().onEnterZone(mCharacter);
	//}
}

void ZoneConnection::_handleClientReady(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	mZoneConnectionStatus = ZoneConnectionStatus::Complete;
	mForceSendPositionTimer.Start(4000);
}

void ZoneConnection::_sendDoors() {
	EXPECTED(mConnected);
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpawnDoor, 0);
	//mStreamInterface->QueuePacket(outPacket);
}

void ZoneConnection::_sendObjects() {
	EXPECTED(mConnected);
	
	const auto objects = mZone->getObjects();
	for (auto i : objects) {
		sendObject(i);
	}
}

void ZoneConnection::_sendZonePoints() {
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

	auto packet = new EQApplicationPacket(OP_SendZonepoints, packetSize);
	Utility::DynamicStructure ds(packet->pBuffer, packetSize);
	
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

	sendPacket(packet);
	delete packet;
	EXPECTED(ds.check());
}

void ZoneConnection::_sendAAStats() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_SendAAStats, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendZoneServerReady() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_ZoneServerReady, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendExpZoneIn() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_SendExpZonein, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendWorldObjectsSent() {
	auto packet = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleClientUpdate(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::_handleSpawnAppearance(const EQApplicationPacket* pPacket) {
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
	case SpawnAppearanceTypeID::Animation:
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
	case SpawnAppearanceTypeID::Anonymous:
		// 0 = Normal, 1 = Anonymous, 2 = Roleplay
		if (actionParameter >= 0 && actionParameter <= 2) {
			// Update character and notify zone.
			mCharacter->setAnonymous(actionParameter);
			mZone->handleAnonymous(mCharacter);
		}
		// Anything else is ignored.
		break;
		// Handle AFK
	case SpawnAppearanceTypeID::AFK:
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
	case SpawnAppearanceTypeID::ShowHelm:
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
	case SpawnAppearanceTypeID::HP:
		break;
	case SpawnAppearanceTypeID::Split:
		break;
	case SpawnAppearanceTypeID::Die:
		break;
	case SpawnAppearanceTypeID::DamageState:
		break;
	case SpawnAppearanceTypeID::Sneak:
		break;
	case SpawnAppearanceTypeID::Invisible:
		break;
	case SpawnAppearanceTypeID::Size:
		break;
	case SpawnAppearanceTypeID::Light:
		break;
	case SpawnAppearanceTypeID::PVP:
		Log::info("Got PVP");
		// NOTE: Not sure if this even possible.
		break;
	case SpawnAppearanceTypeID::AutoConsentGroup:
		mCharacter->setAutoConsentGroup(actionParameter == 1);
		break;
	case SpawnAppearanceTypeID::AutoConsentGuild:
		mCharacter->setAutoConsentGuild(actionParameter == 1);
		break;
	case SpawnAppearanceTypeID::AutoConsentRaid:
		mCharacter->setAutoConsentRaid(actionParameter == 1);
		break;
	default:
		StringStream ss;
		ss << "[Zone Client Connection] Got unexpected SpawnAppearanceTypes : " << actionType;
		Log::error(ss.str());
		break;
	}
}

void ZoneConnection::_handleCamp(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	mCharacter->startCamp();
}

void ZoneConnection::_handleChannelMessage(const EQApplicationPacket* pPacket) {
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
	const u32 channel = payload->chan_num;

	mZone->handleChannelMessage(mCharacter, channel, senderName, targetName, message);
	//
	//switch (channel) {
	//case ChannelID::Guild:
	//	EXPECTED(mCharacter->hasGuild());
	//	mGuildManager->handleMessage(mCharacter, message);
	//	break;
	//case ChannelID::Group:
	//	mGroupManager->handleMessage(mCharacter, message);
	//	break;
	//case ChannelID::Shout:
	//	mZone->handleShout(mCharacter, message);
	//	break;
	//case ChannelID::Auction:
	//	mZone->handleAuction(mCharacter, message);
	//	break;
	//case ChannelID::OOC:
	//	mZone->handleOOC(mCharacter, message);
	//	break;
	//case ChannelID::GMSay:
	//case ChannelID::Broadcast:
	//	// GM_SAY / CH_BROADAST are unused as far as I know.
	//	break;
	//case ChannelID::Tell:
	//	if (senderName.length() > 0 && targetName.length() > 0) {
	//		mZone->handleTell(mCharacter, targetName, message);
	//	}
	//	break;
	//case ChannelID::Say:
	//	// Check whether user has entered a command.
	//	if (message[0] == COMMAND_TOKEN) {
	//		mCommandHandler->command(mCharacter, message);
	//		break;
	//	}
	//	mZone->handleSay(mCharacter, message);
	//	break;
	//case ChannelID::Raid:
	//	break;
	//case ChannelID::UCS:
	//	break;
	//	// /emote dances around wildly!
	//case ChannelID::Emote:
	//	mZone->handleEmote(mCharacter, message);
	//	break;
	//default:
	//	StringStream ss;
	//	ss << "[Zone Client Connection] " << __FUNCTION__ << " Got unexpected channel number: " << channel;
	//	Log::error(ss.str());
	//	break;
	//}
}

void ZoneConnection::sendPosition() {
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

void ZoneConnection::sendMessage(const u32 pType, String pMessage) {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_SpecialMesg, sizeof(SpecialMesg_Struct)+pMessage.length());
	auto payload = reinterpret_cast<SpecialMesg_Struct*>(packet->pBuffer);
	payload->header[0] = 0x00; // Header used for #emote style messages..
	payload->header[1] = 0x00; // Play around with these to see other types
	payload->header[2] = 0x00;
	payload->msg_type = pType;
	strcpy(payload->message, pMessage.c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleLogOut(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
	auto payload = reinterpret_cast<CancelTrade_Struct*>(packet->pBuffer);
	payload->fromid = mCharacter->getSpawnID();
	payload->action = groupActUpdate;

	sendPacket(packet);
	delete packet;

	_sendPreLogOutReply();
	_sendLogOutReply();

	mCharacter->setCampComplete(true);

	// NOTE: Zone picks up the dropped connection next update.
	dropConnection();
}

void ZoneConnection::_sendLogOutReply() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_LogoutReply);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendPreLogOutReply() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_PreLogoutReply);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleDeleteSpawn(const EQApplicationPacket* pPacket) {
	_sendLogOutReply();
	mCharacter->setZoningOut();

	// NOTE: Zone picks up the dropped connection next update.
	dropConnection();
}

void ZoneConnection::_handleRequestNewZoneData(const EQApplicationPacket* pPacket) {
	mZoneConnectionStatus = ZoneConnectionStatus::ClientRequestZoneData;
	_sendZoneData();
}

void ZoneConnection::_sendZoneData() {
	EXPECTED(mConnected);

	auto packet = Payload::Zone::ZoneData::create();
	auto payload = Payload::Zone::ZoneData::convert(packet);

	String longName = mZone->getLongName();
	longName += " [" + std::to_string(mZone->getInstanceID()) + "]";

	strcpy(payload->mCharacterName, mCharacter->getName().c_str());
	payload->mZoneID = mZone->getID();
	payload->mInstanceID = mZone->getInstanceID();
	strcpy(payload->mShortName, mZone->getShortName().c_str());
	strcpy(payload->mLongName, longName.c_str()); // NOTE: This affects the zone in message "You have entered ..."
	payload->mGravity = mZone->getGravity();
	payload->underworld = mZone->getMinimumZ();

	payload->mZoneType = mZone->getZoneType();
	payload->mTimeType = mZone->getTimeType();
	payload->mSkyType = mZone->getSkyType();
	payload->mFogDensity = mZone->getFogDensity();
	payload->mMinimumClip = mZone->getMinimumClip();
	payload->mMaximumClip = mZone->getMaximumClip();

	strcpy(payload->mShortName2, mZone->getShortName().c_str());

	// Temp until I decide how I want to represent fog and weather in Zone.
	auto zoneData = ServiceLocator::getZoneDataManager()->getZoneData(mZone->getID());
	EXPECTED(zoneData);

	// Fog.
	for (auto i = 0; i < 4; i++) {
		payload->mFog.mRed[i] = zoneData->mFog[i].mRed;
		payload->mFog.mGreen[i] = zoneData->mFog[i].mGreen;
		payload->mFog.mBlue[i] = zoneData->mFog[i].mBlue;
		
		payload->mFog.mMinimumClip[i] = zoneData->mFog[i].mMinimumClip;
		payload->mFog.mMaximumClip[i] = zoneData->mFog[i].mMaximumClip;
	}

	// Weather.
	for (auto i = 0; i < 4; i++) {
		payload->mWeather.mRainChance[i] = zoneData->mWeather[i].mRainChance;
		payload->mWeather.mRainDuration[i] = zoneData->mWeather[i].mRainDuration;
		payload->mWeather.mSnowChance[i] = zoneData->mWeather[i].mSnowChance;
		payload->mWeather.mSnowDuration[i] = zoneData->mWeather[i].mSnowDuration;
	}

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendAppearance(uint16 pType, uint32 pParameter) {
	EXPECTED(mConnected);
	
	auto packet = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	auto payload = reinterpret_cast<SpawnAppearance_Struct*>(packet->pBuffer);
	payload->spawn_id = mCharacter->getSpawnID();
	payload->type = pType;
	payload->parameter = pParameter;

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleSendAATable(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	// TODO:
}

void ZoneConnection::_handleUpdateAA(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	// TODO:
}

void ZoneConnection::_handleTarget(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Target::sizeCheck(pPacket->size));

	auto payload = Target::convert(pPacket->pBuffer);
	mZone->handleTarget(mCharacter, payload->mSpawnID);
}

void ZoneConnection::_handleTGB(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::sendSimpleMessage(const u32 pType, const u32 pStringID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = SimpleMessage::construct(pType, pStringID);
	sendPacket(packet);
	delete packet;
}

EQApplicationPacket* ZoneConnection::makeSimpleMessage(const u32 pType, const u32 pStringID, String pParameter0, String pParameter1, String pParameter2, String pParameter3, String pParameter4, String pParameter5, String pParameter6, String pParameter7, String pParameter8, String pParameter9) {
	int packetSize = 0;
	packetSize += 4; // Unknown.
	packetSize += 4; // String ID.
	packetSize += 4; // Type.

	String message;

	// Adjust payload size for variable length strings.
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

	auto packet = new EQApplicationPacket(OP_FormattedMessage, packetSize);

	Utility::DynamicStructure ds(packet->pBuffer, packetSize);

	// Write header.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(pStringID); // String ID.
	ds.write<u32>(pType); // Type.

	// Write variable length strings.
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

	EXPECTED_PTR(ds.check());
	return packet;
}

void ZoneConnection::sendSimpleMessage(const u32 pType, const u32 pStringID, String pParameter0, String pParameter1, String pParameter2, String pParameter3, String pParameter4, String pParameter5, String pParameter6, String pParameter7, String pParameter8, String pParameter9) {
	EXPECTED(mConnected);

	auto packet = makeSimpleMessage(pType, pStringID, pParameter0, pParameter1, pParameter2, pParameter3, pParameter4, pParameter5, pParameter6, pParameter7, pParameter8, pParameter9);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendHealthUpdate() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = HealthUpdate::construct(mCharacter->getSpawnID(), mCharacter->getCurrentHP(), mCharacter->getMaximumHP());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendPacket(const EQApplicationPacket* pPacket) {
	EXPECTED(mConnected);
	mStreamInterface->QueuePacket(pPacket);
}

void ZoneConnection::_handleEmote(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Emote::sizeCheck(pPacket));

	auto payload = Emote::convert(pPacket);

	String message = Utility::safeString(payload->mMessage, EmoteLimits::MAX_MESSAGE);
	mZone->handleEmote(mCharacter, message);
}

void ZoneConnection::_handleAnimation(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ActorAnimation::sizeCheck(pPacket));
	
	auto payload = ActorAnimation::convert(pPacket);
	EXPECTED(payload->mSpawnID == mCharacter->getSpawnID());

	mZone->handleAnimation(mCharacter, payload->mAnimation, payload->mSpeed, false);
}

void ZoneConnection::sendExperienceUpdate(const u32 pExperience,  const u32 pAAExperience) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ExperienceUpdate::construct(pExperience, pAAExperience);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendAAExperienceUpdate(const u32 pAAExperience, const u32 pUnspentAA, const u32 pExperienceToAA) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = AAExperienceUpdate::construct(pAAExperience, pUnspentAA, pExperienceToAA);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendLeadershipExperienceUpdate(const double pGroupExperience, const u32 pGroupPoints, const double pRaidExperience, const u32 pRaidPoints) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = LeadershipExperienceUpdate::construct(pGroupExperience, pGroupPoints, pRaidExperience, pRaidPoints);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendLevelUpdate(const u32 pPreviousLevel, const u32 pCurrentLevel, const u32 pExperienceRatio) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = LevelUpdate::construct(pPreviousLevel, pCurrentLevel, pExperienceRatio);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendExperienceMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainExperience); }
void ZoneConnection::sendGroupExperienceMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainGroupExperience); }
void ZoneConnection::sendRaidExperienceMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainRaidExperience); }

void ZoneConnection::sendExperienceLossMessage() {
	EXPECTED(mConnected);
	// There is no StringID for this message apparently.
	sendMessage(MessageType::Yellow, "You have lost experience.");
}

void ZoneConnection::sendLevelGainMessage() {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::Experience, StringID::GainLevel, std::to_string(mCharacter->getLevel()));
}

void ZoneConnection::sendLevelsGainMessage(const u8 pLevels) {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::Experience, StringID::GainLevels, std::to_string(pLevels), std::to_string(mCharacter->getLevel()));
}

void ZoneConnection::sendLevelLostMessage() {
	// NOTE: UF Handles this message itself, no need to send.
	//StringStream ss;
	//ss << mCharacter->getLevel();
	//sendSimpleMessage(MT_Experience, LOSE_LEVEL, ss.str());
}

void ZoneConnection::sendGainGroupLeadershipExperienceMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainGroupLeadershipExperience); }
void ZoneConnection::sendGainGroupLeadershipPointMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainGroupLeadershipPoint); }
void ZoneConnection::sendGainRaidLeadershipExperienceMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainRaidLeadershipExperience); }
void ZoneConnection::sendGainRaidLeadershipPointMessage() { EXPECTED(mConnected); sendSimpleMessage(MessageType::Experience, StringID::GainRaidLeadershipPoint); }

void ZoneConnection::sendAAPointGainMessage(const u32 pUnspentAAPoints) {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::Experience, StringID::GainAAPoint, std::to_string(pUnspentAAPoints));
}

void ZoneConnection::sendAAExperienceOnMessage() {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::White, StringID::AAOn);
}

void ZoneConnection::sendAAExperienceOffMessage() {
	EXPECTED(mConnected);
	sendSimpleMessage(MessageType::White, StringID::AAOff);
}

void ZoneConnection::sendLevelAppearance(const u32 pParameter1) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = LevelAppearance::create();
	auto payload = LevelAppearance::convert(packet);
	payload->mSpawnID = mCharacter->getSpawnID();

	payload->mParameter1 = pParameter1;

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendStats() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_IncreaseStats, sizeof(IncreaseStat_Struct));
	auto payload = (IncreaseStat_Struct*)packet->pBuffer;
	payload->str = 5;
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleWhoRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(WhoRequest::sizeCheck(pPacket));

	auto payload = WhoRequest::convert(pPacket);
	Log::info(payload->_debug());
	EXPECTED(payload->mType == 0 || payload->mType == 3); // who or who all.

	WhoFilter filter;
	filter.mType = payload->mType == 0 ? WhoType::Zone : WhoType::All;
	filter.mText = Utility::safeString(payload->mCharacterName, 64);
	filter.mRace = payload->mRace;
	filter.mClass = payload->mClass;
	filter.mLevelMinimum = payload->mLevelMinimum;
	filter.mLevelMaximum = payload->mLevelMaximum;
	filter.mFlag = payload->mFlag;
	// TODO: GM.

	mZone->handleWhoRequest(mCharacter, filter);
}

void ZoneConnection::sendWhoResponse(const u32 pWhoType, std::list<Character*>& pResults) {
	EXPECTED(mConnected);

	static const u32 MaxResults = 20;
	static const String whoLine("-+-+-+-+----------+-+-+-+-");

	const u32 numResults = pResults.size();
	const bool toGM = mCharacter->isGM();

	static const String accountName = "account";

	enum WhoStartStringID {
		WSS_PlayersLFG = 5000, // Players looking for a group :
		WSS_Players = 5001, // Players in EverQuest :
		WSS_Zones = 5002, // Zones in EverQuest :
	};
	// Determine which start String ID to use.
	u32 startStringID = WSS_Players;
	// TODO: LFG.

	//5030 There are % 1 players in EverQuest.Count % 2, no Active Char % 3
	//5031 There are % 1 players in EverQuest.Count % 2, filtered % 3
	//5032 There are % 1 players in EverQuest.Count % 2, no Active Char % 3, filtered % 4
	//5034 There is % 1 zone server up.
	//5035 There are % 1 zone servers up.

	enum WhoEndStringID {
		WES_Player = 5028, // There is % 1 player in EverQuest.
		WES_None = 5029, // There are no players in EverQuest that match those who filters.
		WES_TooMany = 5033, // Your who request was cut short..too many players.
		WES_Players = 5036, // There are % 1 players in EverQuest.
	};
	// Determine which end String ID to use.
	u32 endStringID = WES_Players;
	// No results.
	if (numResults == 0) endStringID = WES_None;
	// Single result.
	else if (numResults == 1) endStringID = WES_Player;
	// Too many results.
	else if (numResults > MaxResults) endStringID = WES_TooMany;

	//% T1[% 2 % 3] % 4 (% 5) % 6 % 7 % 8 % 9
	// T1=String[2=Level 3=Class]  4=Name (5=Race) 6=GuildName, String + ZoneString.

	// Calculate payload size.
	u32 payloadSize = 0;
	// Fixed header size.
	payloadSize += 64;
	// Fixed results size.
	payloadSize += 48 * pResults.size();
	// Variable result size.
	for (auto i : pResults) {
		payloadSize += i->getName().size() + 1;
		payloadSize += accountName.size() + 1;

		if (i->hasGuild())
			payloadSize += i->getGuildName().size() + 3; // +1 for null terminator and + 2 for brackets.
		else
			payloadSize += 1; // null terminator.
	}

	auto packet = new EQApplicationPacket(OP_WhoAllResponse, payloadSize);
	Utility::DynamicStructure ds(packet->pBuffer, payloadSize);

	// Write header.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(startStringID); // String ID.
	ds.writeString(whoLine);
	ds.write<u8>(0x0a); // Unknown.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(endStringID); // String ID.
	ds.write<u32>(0); // Filtered.
	ds.write<u32>(0); // No Active Characters.
	ds.write<u32>(numResults); // Count.
	ds.write<u32>(numResults); // Players.
	ds.write<u32>(numResults); // Number of results.

	// Write results.
	for (auto i : pResults) {
		enum FormatStringID {
			FS_GM_ANONYMOUS = 5022, // % T1[ANON(% 2 % 3)] % 4 (% 5) % 6 % 7 % 8
			FS_ROLEPLAY = 5023, // % T1[ANONYMOUS] % 2 % 3 % 4
			FS_ANONYMOUS = 5024, //% T1[ANONYMOUS] % 2 % 3
			FS_DEFAULT = 5025, //% T1[% 2 % 3] % 4 (% 5) % 6 % 7 % 8 % 9
		};
		// Determine which format String ID to use.
		u32 formatString = FS_DEFAULT;
		// Character is anonymous.
		if (i->isAnonymous()) {
			formatString = FS_ANONYMOUS;
			// GM override.
			if (toGM) formatString = FS_GM_ANONYMOUS;
		}
		// Character is roleplaying.
		if (i->isRoleplaying()) {
			formatString = FS_ROLEPLAY;
			// GM override.
			if (toGM) formatString = FS_GM_ANONYMOUS;
		}

		enum WhoFlags {
			WF_None = 0,
			WF_AFK = 1,
			WF_LD = 2 << 2,
			WF_Trader = 2 << 3,
			WF_Buyer = 2 << 4,
			WF_RIP = 2 << 5,
		};
		// Determine Character flags.
		u32 flags = WF_None;
		if (i->isAFK()) flags |= WF_AFK;
		if (i->isLinkDead()) flags |= WF_LD;
		if (i->isTrader()) flags |= WF_Trader;
		if (i->isBuyer()) flags |= WF_Buyer;
		if (i->isDead()) flags |= WF_RIP;
		
		u32 zoneStringID = 0xFFFFFFFF;
		if (pWhoType == WhoType::All) zoneStringID = 5006; // 5006 = "ZONE: %1"

		// Determine Zone ID.
		u16 zoneID = 0;
		if (pWhoType == WhoType::All) {
			// Character is not currently zoning.
			if (i->isZoning() == false)
				zoneID = i->getZone()->getID();
		}
		String guildName = "";
		if (i->hasGuild())
			guildName = "<" + i->getGuildName() + ">";

		ds.write<u32>(formatString); // String ID.
		ds.write<u32>(flags); // Flags.
		ds.write<u32>(0xFFFFFFFF); // Unknown.
		ds.writeString(i->getName()); // Character Name. // TODO: This is going to come up as X's corpse if the Character is dead.
		ds.write<u32>(0xFFFFFFFF); // String ID. (Rank String ID) *GM Impossible* etc.
		ds.writeString(guildName); // Guild Name
		ds.write<u32>(0); // Unknown.
		ds.write<u32>(0xFFFFFFFF); // String ID.
		ds.write<u32>(zoneStringID); // String ID.
		ds.write<u16>(zoneID); // Zone ID. Parameter for zoneStringID String.
		ds.write<u16>(0); // Unknown (Tested, not part of Zone ID.)
		ds.write<u32>(i->getClass()); // Class.
		ds.write<u32>(i->getLevel()); // Level.
		ds.write<u32>(i->getRace()); // Race.
		ds.writeString(accountName); // Account Name // TODO: This does not work.
		ds.write<u32>(0); // Unknown.
	}

	sendPacket(packet);
	delete packet;
	EXPECTED(ds.check());
}

EQApplicationPacket* ZoneConnection::makeChannelMessage(const u32 pChannel, const String& pSenderName, const String& pMessage) {
	u32 payloadSize = 0;
	payloadSize += 36; // Fixed.
	payloadSize += pSenderName.size() + 1;
	payloadSize += pMessage.size() + 1;
	payloadSize += 1; // target null term

	auto packet = new EQApplicationPacket(OP_ChannelMessage, payloadSize);
	Utility::DynamicStructure ds(packet->pBuffer, payloadSize);

	ds.writeString(pSenderName); // Sender Name.
	ds.write<u8>(0); // Target Name.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(0); // Language ID.
	ds.write<u32>(pChannel); // Channel ID.
	ds.write<u32>(0); // Unknown.
	ds.write<u8>(0); // Unknown.
	ds.write<u32>(0); // Language Skill.
	ds.writeString(pMessage); // Message.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(0); // Unknown.
	ds.write<u16>(0); // Unknown.
	ds.write<u8>(0); // Unknown.

	return packet;
}

void ZoneConnection::sendChannelMessage(const u32 pChannel, const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);

	auto packet = makeChannelMessage(pChannel, pSenderName, pMessage);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendTell(const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);
	sendChannelMessage(ChannelID::Tell, pSenderName, pMessage);
}

void ZoneConnection::sendGroupMessage(const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);
	sendChannelMessage(ChannelID::Group, pSenderName, pMessage);
}

void ZoneConnection::sendGuildMessage(const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);
	sendChannelMessage(ChannelID::Guild, pSenderName, pMessage);
}

// NOTE: This occurs when the player presses 'Invite' on the group window.
void ZoneConnection::_handleGroupInvite(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupInvite_Struct));

	auto payload = reinterpret_cast<GroupInvite_Struct*>(pPacket->pBuffer);

	const String inviterName = Utility::safeString(payload->inviter_name, Limits::Character::MAX_NAME_LENGTH);
	const String inviteeName = Utility::safeString(payload->invitee_name, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(Limits::Character::nameLength(inviteeName));
	EXPECTED(inviterName == mCharacter->getName()); // Check: Spoofing
	EXPECTED(inviteeName != mCharacter->getName()); // Check: Not inviting ourself
	
	mGroupManager->handleInviteSent(mCharacter, inviteeName);
}

void ZoneConnection::sendGroupInvite(const String pFromCharacterName) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = GroupInvite::construct(pFromCharacterName, mCharacter->getName());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleGroupFollow(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupGeneric_Struct));

	auto payload = reinterpret_cast<GroupGeneric_Struct*>(pPacket->pBuffer);
	
	String inviterName = Utility::safeString(payload->name1, Limits::Character::MAX_NAME_LENGTH); // Character who invited.
	String inviteeName = Utility::safeString(payload->name2, Limits::Character::MAX_NAME_LENGTH); // Character accepting invite.
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(Limits::Character::nameLength(inviteeName));
	EXPECTED(inviteeName == mCharacter->getName()); // Check: Sanity

	// TODO: This can be spoofed to join groups...

	mGroupManager->handleAcceptInvite(mCharacter, inviterName);
}

void ZoneConnection::_handleGroupCanelInvite(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupCancel_Struct));

	auto payload = reinterpret_cast<GroupCancel_Struct*>(pPacket->pBuffer);
	String inviterName = Utility::safeString(payload->name1, Limits::Character::MAX_NAME_LENGTH);
	String inviteeName = Utility::safeString(payload->name2, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(inviterName));
	EXPECTED(Limits::Character::nameLength(inviteeName));
	EXPECTED(inviteeName == mCharacter->getName()); // Check: Sanity

	mGroupManager->handleDeclineInvite(mCharacter, inviterName);
}

void ZoneConnection::sendGroupCreate() {
	EXPECTED(mConnected);

	int packetSize = 31 + mCharacter->getName().length() + 1; // Magic number due to no packet structure.
	auto packet = new EQApplicationPacket(OP_GroupUpdateB, packetSize);

	Utility::DynamicStructure ds(packet->pBuffer, packetSize);
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

	sendPacket(packet);
	delete packet;
	EXPECTED(ds.check());
}

void ZoneConnection::sendGroupLeaderChange(const String pCharacterName) {
	EXPECTED(mConnected);

	// Configure.
	auto payload = reinterpret_cast<GroupLeaderChange_Struct*>(mGroupLeaderChangePacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	strcpy(payload->LeaderName, pCharacterName.c_str());

	sendPacket(mGroupLeaderChangePacket);
}

void ZoneConnection::sendGroupAcknowledge() {
	EXPECTED(mConnected);

	static const auto PACKET_SIZE = 4;
	auto packet = new EQApplicationPacket(OP_GroupAcknowledge, PACKET_SIZE);

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendGroupFollow(const String& pLeaderCharacterName, const String& pMemberCharacterName) {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_GroupFollow, sizeof(GroupGeneric_Struct));
	auto payload = reinterpret_cast<GroupGeneric_Struct*>(packet->pBuffer);
	strcpy(payload->name1, pLeaderCharacterName.c_str());
	strcpy(payload->name2, pMemberCharacterName.c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendGroupJoin(const String& pCharacterName) {
	EXPECTED(mConnected);

	// Configure.
	auto payload = reinterpret_cast<GroupJoin_Struct*>(mGroupJoinPacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActJoin;
	strcpy(payload->membername, pCharacterName.c_str());
	strcpy(payload->yourname, mCharacter->getName().c_str());

	sendPacket(mGroupJoinPacket);
}

void ZoneConnection::sendGroupUpdate(std::list<String>& pGroupMemberNames) {
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
	
	sendPacket(mGroupUpdateMembersPacket);
}

void ZoneConnection::_handleGroupDisband(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupGeneric_Struct));
	
	auto payload = reinterpret_cast<GroupGeneric_Struct*>(pPacket->pBuffer);

	String removeCharacterName = Utility::safeString(payload->name1, Limits::Character::MAX_NAME_LENGTH);
	String myCharacterName = Utility::safeString(payload->name2, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(removeCharacterName));
	EXPECTED(Limits::Character::nameLength(myCharacterName));
	EXPECTED(myCharacterName == mCharacter->getName()); // Check: Sanity
	EXPECTED(mCharacter->hasGroup());

	mGroupManager->handleDisband(mCharacter, removeCharacterName);
}

void ZoneConnection::sendGroupLeave(const String& pLeavingCharacterName) {
	EXPECTED(mConnected);

	auto payload = reinterpret_cast<GroupJoin_Struct*>(mGroupLeavePacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActLeave;
	strcpy(payload->yourname, mCharacter->getName().c_str());
	strcpy(payload->membername, pLeavingCharacterName.c_str());

	sendPacket(mGroupLeavePacket);
}

void ZoneConnection::sendGroupDisband() {
	EXPECTED(mConnected);

	auto payload = reinterpret_cast<GroupUpdate_Struct*>(mGroupDisbandPacket->pBuffer);
	*payload = { 0 }; // Clear memory.
	payload->action = groupActDisband;
	strcpy(payload->yourname, mCharacter->getName().c_str());

	sendPacket(mGroupDisbandPacket);
}

void ZoneConnection::_handleGroupMakeLeader(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GroupMakeLeader_Struct));
	EXPECTED(mCharacter->hasGroup());

	auto payload = reinterpret_cast<GroupMakeLeader_Struct*>(pPacket->pBuffer);

	String currentLeader = Utility::safeString(payload->CurrentLeader, Limits::Character::MAX_NAME_LENGTH);
	String newLeader = Utility::safeString(payload->NewLeader, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(currentLeader));
	EXPECTED(Limits::Character::nameLength(newLeader));
	EXPECTED(currentLeader == mCharacter->getName());

	mGroupManager->handleMakeLeader(mCharacter, newLeader);
}

void ZoneConnection::sendRequestZoneChange(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = RequestZoneChange::construct(pZoneID, pInstanceID, pPosition);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendZoneChange(const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition, const int32 pSuccess) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ZoneChange::construct(mCharacter->getName(), pZoneID, pInstanceID, pPosition, pSuccess);
	Log::info("sendZoneChange: " + pPosition.toString());
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneConnection::_handleZoneChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ZoneChange::sizeCheck(pPacket));

	auto payload = ZoneChange::convert(pPacket);
	mZone->handleZoneChange(mCharacter, payload->mZoneID, payload->mInstanceID, Vector3(payload->mX, payload->mY, payload->mZ));
}

void ZoneConnection::_handleGuildCreate(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild() == false);
	EXPECTED(pPacket->size == Limits::Guild::MAX_NAME_LENGTH);
	
	const String guildName = Utility::safeString(reinterpret_cast<char*>(pPacket->pBuffer), Limits::Guild::MAX_NAME_LENGTH);
	EXPECTED(Limits::Guild::nameLength(guildName));

	mGuildManager->handleCreate(mCharacter, guildName);
}

void ZoneConnection::_handleGuildDelete(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(mGuildManager->isLeader(mCharacter)); // Check: Permission.

	mGuildManager->handleDelete(mCharacter);
}

void ZoneConnection::sendGuildRank() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));
	auto payload = reinterpret_cast<GuildSetRank_Struct*>(packet->pBuffer);
	payload->Rank = mCharacter->getGuildRank();
	payload->Banker = 0;
	strcpy(payload->MemberName, mCharacter->getName().c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendGuildNames() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_GuildsList);
	packet->size = Limits::Guild::MAX_NAME_LENGTH + (Limits::Guild::MAX_NAME_LENGTH * Limits::Guild::MAX_GUILDS); // TODO: Work out the minimum sized packet UF will accept.
	packet->pBuffer = reinterpret_cast<unsigned char*>(mGuildManager->_getGuildNames());

	sendPacket(packet);
	packet->pBuffer = nullptr; // Important: This prevents the payload from being deleted.
	delete packet;
}

void ZoneConnection::_handleGuildInvite(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild()); // Check: Character has a guild.
	EXPECTED(mGuildManager->isLeader(mCharacter) || mGuildManager->isOfficer(mCharacter)); // Check: Permission.
	EXPECTED(pPacket->size == sizeof(GuildCommand_Struct));

	auto payload = reinterpret_cast<GuildCommand_Struct*>(pPacket->pBuffer);

	EXPECTED(payload->guildeqid == mCharacter->getGuildID()); // Check: Sanity. Why the fuck does the client send this?

	String toCharacterName = Utility::safeString(payload->othername, Limits::Character::MAX_NAME_LENGTH);
	String fromCharacterName = Utility::safeString(payload->myname, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(toCharacterName)); // NOTE: Client does not check this..
	EXPECTED(Limits::Character::nameLength(fromCharacterName));
	EXPECTED(fromCharacterName == mCharacter->getName()); // Check: Sanity.

	if (payload->officer == 0)
		mGuildManager->handleInviteSent(mCharacter, toCharacterName);
	if (payload->officer == 1)
		mGuildManager->handlePromote(mCharacter, toCharacterName);

	// NOTE: UF requires that the character being promoted is targetted we can verify this later.
}

void ZoneConnection::_handleGuildRemove(const EQApplicationPacket* pPacket) {
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
		EXPECTED(mGuildManager->isLeader(mCharacter) || mGuildManager->isOfficer(mCharacter));

	mGuildManager->handleRemove(mCharacter, toCharacterName);
}

void ZoneConnection::sendGuildInvite(String pInviterName, GuildID pGuildID) {
	EXPECTED(mConnected);
	EXPECTED(pGuildID != NO_GUILD);
	EXPECTED(mCharacter->hasGuild() == false);
	EXPECTED(mCharacter->getPendingGuildInviteID() == pGuildID);

	auto packet = new EQApplicationPacket(OP_GuildInvite, sizeof(GuildCommand_Struct));
	auto payload = reinterpret_cast<GuildCommand_Struct*>(packet->pBuffer);

	payload->guildeqid = pGuildID;
	// NOTE: myname/othername were poor choices for variable names.
	strcpy(payload->othername, mCharacter->getName().c_str());
	strcpy(payload->myname, pInviterName.c_str());
	
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleGuildInviteAccept(const EQApplicationPacket* pPacket) {
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
		mGuildManager->handleInviteAccept(mCharacter, inviterName);
		return;
	}
	// Character is declining the invite.
	if (payload->response == Decline) {
		mGuildManager->handleInviteDecline(mCharacter, inviterName);
		return;
	}

	Log::error("[Zone Client Connection] Got unexpected response(" + std::to_string(payload->response) + ") to Guild invite " + Utility::characterLogDetails(mCharacter));
	mCharacter->clearPendingGuildInvite();
}

void ZoneConnection::_handleSetGuildMOTD(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(mGuildManager->isLeader(mCharacter) || mGuildManager->isOfficer(mCharacter)); // Only leader or officers can set the MOTD.
	EXPECTED(pPacket->size == sizeof(GuildMOTD_Struct));

	auto payload = reinterpret_cast<GuildMOTD_Struct*>(pPacket->pBuffer);

	String characterName = Utility::safeString(payload->name, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(characterName == mCharacter->getName()); // Check: Sanity.
	String motd = Utility::safeString(payload->motd, Limits::Guild::MAX_MOTD_LENGTH);

	mGuildManager->handleSetMOTD(mCharacter, motd);
}

void ZoneConnection::sendGuildMOTD(const String& pMOTD, const String& pMOTDSetByName) {
	EXPECTED(mConnected);
	//EXPECTED(mCharacter->hasGuild());

	auto packet = new EQApplicationPacket(OP_GuildMOTD, sizeof(GuildMOTD_Struct));
	auto payload = reinterpret_cast<GuildMOTD_Struct*>(packet->pBuffer);
	payload->unknown0 = 0;
	strcpy(payload->name, mCharacter->getName().c_str());
	strcpy(payload->setby_name, pMOTDSetByName.c_str());
	strcpy(payload->motd, pMOTD.c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendGuildMOTDReply(const String& pMOTD, const String& pMOTDSetByName) {
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild());

	auto packet = new EQApplicationPacket(OP_GetGuildMOTDReply, sizeof(GuildMOTD_Struct));
	auto payload = reinterpret_cast<GuildMOTD_Struct*>(packet->pBuffer);
	payload->unknown0 = 0;
	strcpy(payload->name, mCharacter->getName().c_str());
	strcpy(payload->setby_name, pMOTDSetByName.c_str());
	strcpy(payload->motd, pMOTD.c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleGetGuildMOTD(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());

	mGuildManager->handleGetMOTD(mCharacter);
}

void ZoneConnection::sendGuildMembers(const std::list<GuildMember*>& pGuildMembers) {
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

	auto packet = new EQApplicationPacket(OP_GuildMemberList, payloadSize);
	
	Utility::DynamicStructure ds(packet->pBuffer, payloadSize);
	
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

	sendPacket(packet);
	delete packet;
	EXPECTED(ds.check());
}

void ZoneConnection::sendGuildURL(const String& pURL) {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(packet->pBuffer);
	payload->mAction = Payload::Guild::GuildUpdate::GUILD_URL;
	strcpy(&payload->mText[0], pURL.c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendGuildChannel(const String& pChannel) {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(packet->pBuffer);
	payload->mAction = Payload::Guild::GuildUpdate::GUILD_CHANNEL;
	strcpy(&payload->mText[0], pChannel.c_str());

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleSetGuildURLOrChannel(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(mGuildManager->isLeader(mCharacter)); // Only a Guild leader can perform this operation.
	EXPECTED(pPacket->size == sizeof(Payload::Guild::GuildUpdate));

	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(pPacket->pBuffer);
	if (payload->mAction == GUILD_URL) {
		String url = Utility::safeString(payload->mText, Limits::Guild::MAX_URL_LENGTH);
		mGuildManager->handleSetURL(mCharacter, url);
	}
	else if (payload->mAction == GUILD_CHANNEL) {
		String channel = Utility::safeString(payload->mText, Limits::Guild::MAX_CHANNEL_LENGTH);
		mGuildManager->handleSetChannel(mCharacter, channel);
	}
	else {
		StringStream ss; ss << "[Zone Client Connection]Got unknown action value(" << payload->mAction << ") in _handleSetGuildURLOrChannel from " << Utility::characterLogDetails(mCharacter);
		Log::error(ss.str());
	}
}

void ZoneConnection::_handleSetGuildPublicNote(const EQApplicationPacket* pPacket) {
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
		EXPECTED(mGuildManager->isLeader(mCharacter) || mGuildManager->isOfficer(mCharacter));

	mGuildManager->handleSetPublicNote(mCharacter, targetName, note);
}

void ZoneConnection::_handleGetGuildStatus(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(pPacket->size == sizeof(GuildStatus_Struct));

	auto payload = reinterpret_cast<GuildStatus_Struct*>(pPacket->pBuffer);

	String targetName = Utility::safeString(payload->Name, Limits::Character::MAX_NAME_LENGTH);
	
	// NOTE: UF does not prevent the player from sending smaller than possible names.
	if (Limits::Character::nameLength(targetName) == false)
		return;
	
	mGuildManager->handleStatusRequest(mCharacter, targetName);
}

void ZoneConnection::_handleGuildDemote(const EQApplicationPacket* pPacket) {
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
		EXPECTED(mGuildManager->isLeader(mCharacter));

	// Check: Leader can not self-demote.
	if (characterName == demoteName)
		EXPECTED(mGuildManager->isLeader(mCharacter) == false);

	mGuildManager->handleDemote(mCharacter, demoteName);
}

void ZoneConnection::_handleGuildBanker(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(BankerAltStatus::sizeCheck(pPacket->size));

	auto payload = BankerAltStatus::convert(pPacket->pBuffer);

	// NOTE: UF does not send BankerAltStatus::mCharacterName like other packets. /shrug
	String otherName = Utility::safeString(payload->mOtherName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(otherName));

	bool banker = (payload->mStatus & 0x01) > 1;
	bool alt = (payload->mStatus & 0x02) > 1;

	mGuildManager->handleSetBanker(mCharacter, otherName, banker);
	mGuildManager->handleSetAlt(mCharacter, otherName, alt);
}

void ZoneConnection::_handleGuildMakeLeader(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	EXPECTED(pPacket);
	EXPECTED(mCharacter->hasGuild());
	EXPECTED(mGuildManager->isLeader(mCharacter));
	EXPECTED(MakeLeader::sizeCheck(pPacket->size));

	auto payload = MakeLeader::convert(pPacket->pBuffer);

	String characterName = Utility::safeString(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(characterName));
	EXPECTED(mCharacter->getName() == characterName); // Check: Sanity.
	String leaderName = Utility::safeString(payload->mLeaderName, Limits::Character::MAX_NAME_LENGTH);
	EXPECTED(Limits::Character::nameLength(leaderName));

	mGuildManager->handleMakeLeader(mCharacter, leaderName);
}

void ZoneConnection::_unimplementedFeature(String pOpCodeName)
{
}

void ZoneConnection::_handleFaceChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(FaceChange::sizeCheck(pPacket->size));

	auto payload = FaceChange::convert(pPacket->pBuffer);
	Log::info(payload->_debug());

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

void ZoneConnection::sendWearChange(const uint16 pSpawnID, const uint32 pMaterialID, const uint32 pEliteMaterialID, const uint32 pColour, const uint8 pSlotID) {
	EXPECTED(mConnected);
	using namespace Payload::Zone;

	auto packet = WearChange::construct(pSpawnID, pMaterialID, pEliteMaterialID, pColour, pSlotID);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleAutoAttack(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AutoAttack::sizeCheck(pPacket));

	auto payload = AutoAttack::convert(pPacket->pBuffer);
	mCharacter->setAutoAttack(payload->mAttacking);
}

void ZoneConnection::_handleMemoriseSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(MemoriseSpell::sizeCheck(pPacket));
	EXPECTED(mCharacter->isCaster());

	auto payload = MemoriseSpell::convert(pPacket->pBuffer);

	switch (payload->mAction){
		// Character is scribing a spell into the SpellBook.
	case MemoriseSpell::SCRIBE:
		EXPECTED(mCharacter->handleScribeSpell(payload->mSlot, payload->mSpellID));
		break;
		// Character is adding a spell to the SpellBar.
	case MemoriseSpell::MEMORISE:
		EXPECTED(mCharacter->handleMemoriseSpell(payload->mSlot, payload->mSpellID));
		break;
		// Character is removing a spell from the SpellBar.
	case MemoriseSpell::UNMEMORISE:
		EXPECTED(mCharacter->handleUnmemoriseSpell(payload->mSlot));
		break;
	default:
		Log::error("Unknown action in _handleMemoriseSpell from " + Utility::characterLogDetails(mCharacter));
		break;
	}
}

void ZoneConnection::_handleDeleteSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(DeleteSpell::sizeCheck(pPacket));
	EXPECTED(mCharacter->isCaster()); // Sanity.

	auto payload = DeleteSpell::convert(pPacket);
	const bool success = mCharacter->handleDeleteSpell(payload->mSlot);
	sendDeleteSpellDelete(payload->mSlot, success);
}

void ZoneConnection::_handleLoadSpellSet(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(LoadSpellSet::sizeCheck(pPacket));
	EXPECTED(mCharacter->isCaster()); // Sanity.

	auto payload = LoadSpellSet::convert(pPacket->pBuffer);
}

void ZoneConnection::_handleSwapSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(SwapSpell::sizeCheck(pPacket));
	EXPECTED(mCharacter->isCaster()); // Sanity.

	auto payload = SwapSpell::convert(pPacket->pBuffer);
	EXPECTED(mCharacter->handleSwapSpells(payload->mFrom, payload->mTo));

	// Client requires a reply.
	sendPacket(pPacket);
}

void ZoneConnection::_handleCastSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(CastSpell::sizeCheck(pPacket));
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

void ZoneConnection::_handleCombatAbility(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(CombatAbility::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->hasTarget());

	auto payload = CombatAbility::convert(pPacket->pBuffer);

	EXPECTED(payload->mTargetID == mCharacter->getTarget()->getSpawnID());
}

void ZoneConnection::_handleTaunt(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Taunt::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->hasTarget());
	EXPECTED(mCharacter->getTarget()->isNPC());
	EXPECTED(mCharacter->canTaunt());

	auto payload = Taunt::convert(pPacket->pBuffer);

	EXPECTED(payload->mSpawnID == mCharacter->getTarget()->getSpawnID());
}

void ZoneConnection::_handleConsider(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Consider::sizeCheck(pPacket));
	
	auto payload = Consider::convert(pPacket);
	Log::info(payload->_debug());
	mZone->handleConsider(mCharacter, payload->mTargetSpawnID);
}

void ZoneConnection::_handleConsiderCorpse(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Consider::sizeCheck(pPacket));

	auto payload = Consider::convert(pPacket);
	Log::info(payload->_debug());
	mZone->handleConsiderCorpse(mCharacter, payload->mTargetSpawnID);
}

void ZoneConnection::_handleSurname(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::sendSurnameApproval(const bool pSuccess) {
	// NOTE: This packet notifies the client that their surname was approved.
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = Surname::construct(pSuccess ? 1 : 0, mCharacter->getName(), mCharacter->getLastName());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleGMLastName(const EQApplicationPacket* pPacket) {
	mCharacter->notify("Please use the command system.");
}

void ZoneConnection::_handleClearSurname(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	// Update Character.
	mCharacter->setLastName("");
	// Update Zone.
	mZone->handleSurnameChange(mCharacter);
}

void ZoneConnection::_handleSetTitle(const EQApplicationPacket* pPacket) {
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
			prefix = ServiceLocator::getTitleManager()->getPrefix(payload->mTitleID);

		// Update Character.
		mCharacter->setTitle(prefix);
	}

	// Suffix changing.
	if (payload->mOption == SetTitle::SET_SUFFIX) {
		String suffix = "";

		// NOTE: Where mTitleID = 0 the player has pressed the 'Clear Suffix' button.
		if (payload->mTitleID != 0)
			suffix = ServiceLocator::getTitleManager()->getSuffix(payload->mTitleID);

		// Update Character.
		mCharacter->setSuffix(suffix);
	}

	// Update Zone.
	const uint32 t = payload->mOption == SetTitle::SET_TITLE ? TitleOption::Title : TitleOption::Suffix;
	mZone->handleTitleChanged(mCharacter, t);
	
}

void ZoneConnection::_handleRequestTitles(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	auto availableTitles = ServiceLocator::getTitleManager()->getTitles(mCharacter);
	if (availableTitles.empty())
		return;

	int payloadSize = sizeof(uint32); // 4 bytes: store the number of titles.
	for (auto i : availableTitles) {
		payloadSize += sizeof(uint32); // Title ID
		payloadSize += i->mPrefix.length() + 1; // +1 for null terminator.
		payloadSize += i->mSuffix.length() + 1; // +1 for null terminator.
	}

	auto packet = new EQApplicationPacket(OP_SendTitleList, payloadSize);
	Utility::DynamicStructure ds(packet->pBuffer, payloadSize);
	
	ds.write<uint32>(availableTitles.size());
	for (auto i : availableTitles) {
		ds.write<uint32>(i->mID);
		ds.writeString(i->mPrefix);
		ds.writeString(i->mSuffix);
	}

	sendPacket(packet);
	delete packet;
	EXPECTED(ds.check());
}

void ZoneConnection::sendDeleteSpellDelete(const uint16 pSlot, const bool pSuccess) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = DeleteSpell::construct(pSlot, pSuccess ? 1 : 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID, const uint32 pAction) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_MemorizeSpell, MemoriseSpell::size());
	auto payload = MemoriseSpell::convert(packet->pBuffer);
	payload->mAction = static_cast<MemoriseSpell::Action>(pAction);
	payload->mSlot = pSlot;
	payload->mSpellID = pSpellID;

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendScribeSpell(const u16 pSlot, const u32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpell::SCRIBE);
}

void ZoneConnection::sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpell::MEMORISE);
}

void ZoneConnection::sendUnmemoriseSpell(const uint16 pSlot) {
	_sendMemoriseSpell(pSlot, 0, Payload::Zone::MemoriseSpell::UNMEMORISE);
}

void ZoneConnection::sendInterruptCast() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_InterruptCast, InterruptCast::size());
	auto payload = InterruptCast::convert(packet->pBuffer);
	payload->mSpawnID = mCharacter->getSpawnID();
	payload->mMessageID = static_cast<uint32>(StringID::INTERRUPT_SPELL);

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendRefreshSpellBar(const uint16 pSlot, const uint32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpell::SPELLBAR_REFRESH);
}

void ZoneConnection::sendEnableSpellBar(const uint32 pSpellID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ManaChange::construct(mCharacter->getCurrentMana(), mCharacter->getCurrentEndurance(), pSpellID);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendSkillValue(const uint32 pSkillID, const uint32 pValue) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_SkillUpdate, SkillUpdate::size());
	auto payload = SkillUpdate::convert(packet->pBuffer);
	payload->mID = pSkillID;
	payload->mValue = pValue;

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleBeginLootRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(LootBeginRequest::sizeCheck(pPacket->size));
	EXPECTED(mCharacter->isLooting() == false);

	auto payload = LootBeginRequest::convert(pPacket->pBuffer);
	const uint32 corpseSpawnID = payload->mSpawnID;
	mZone->handleBeginLootRequest(mCharacter, corpseSpawnID);
}

void ZoneConnection::_handleEndLootRequest(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	EXPECTED(mCharacter->isLooting());

	mZone->handleEndLootRequest(mCharacter);
}

void ZoneConnection::sendLootComplete() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_LootComplete, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendLootResponse(uint8 pResponse, uint32 pPlatinum, uint32 pGold, uint32 pSilver, uint32 pCopper) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_MoneyOnCorpse, LootResponse::size());
	auto payload = LootResponse::convert(packet->pBuffer);
	payload->mResponse = pResponse;
	payload->mPlatinum = pPlatinum;
	payload->mGold = pGold;
	payload->mSilver = pSilver;
	payload->mCopper = pCopper;

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendConsiderResponse(const uint32 pSpawnID, const uint32 pMessage) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = Consider::construct(mCharacter->getSpawnID(), pSpawnID, pMessage);
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneConnection::_handleLootItem(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::_handleMoveItem(const EQApplicationPacket* pPacket) {
	if (!_handleMoveItemImpl(pPacket)) {
		inventoryError();
	}
}

const bool ZoneConnection::_handleMoveItemImpl(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::_handleConsume(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::sendStamina(const uint32 pHunger, const uint32 pThirst) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	Stamina payload;
	payload.mHunger = pHunger;
	payload.mThirst = pThirst;

	auto packet = Stamina::create(payload);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handlePotionBelt(const EQApplicationPacket* pPacket) {
}

void ZoneConnection::_handleItemRightClick(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ItemRightClick::sizeCheck(pPacket->size));

	auto payload = ItemRightClick::convert(pPacket->pBuffer);

	sendItemRightClickResponse(payload->mSlot, payload->mTargetSpawnID);
}

void ZoneConnection::sendItemRightClickResponse(const int32 pSlot, const uint32 pTargetSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	ItemRightClickResponse payload;
	payload.mSlot = pSlot;
	payload.mTargetSpawnID = pTargetSpawnID;
	payload.mSpellID = 0;
	
	auto packet = ItemRightClickResponse::create(payload);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::inventoryError(){
	sendMessage(MessageType::Red, "Inventory Error. Please inform a GM and relog to prevent loss of items!");
	Log::error("Inventory Error");
}

void ZoneConnection::_handleOpenContainer(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(OpenContainer::sizeCheck(pPacket->size));

	auto payload = OpenContainer::convert(pPacket->pBuffer);
	Log::info("Open Container: " + std::to_string(payload->mSlot));
}

void ZoneConnection::_handleTradeRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeRequest::sizeCheck(pPacket));

	auto payload = TradeRequest::convert(pPacket);

	EXPECTED(payload->mFromSpawnID == mCharacter->getSpawnID());

	// OP_TradeRequestAck

	// NOTE: Underfoot does appear to require a response.

	mZone->handleTradeRequest(mCharacter, payload->mToSpawnID);
}

void ZoneConnection::_handleTradeRequestAck(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeRequest::sizeCheck(pPacket));

	auto payload = TradeRequest::convert(pPacket);
}

void ZoneConnection::sendTradeRequest(const uint32 pFromSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	TradeRequest payload;
	payload.mToSpawnID = mCharacter->getSpawnID();
	payload.mFromSpawnID = pFromSpawnID;

	auto packet = TradeRequest::create(payload);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleCancelTrade(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::_handleAcceptTrade(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::_handleTradeBusy(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(TradeBusy::sizeCheck(pPacket->size));

	auto payload = TradeBusy::convert(pPacket->pBuffer);
}

void ZoneConnection::_handleSetServerFiler(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ServerFilter::sizeCheck(pPacket));

	auto payload = ServerFilter::convert(pPacket->pBuffer);

	mCharacter->setFilters(payload->mFilters);
}

void ZoneConnection::_handleItemLinkClick(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ItemLink::sizeCheck(pPacket));

	auto payload = ItemLink::convert(pPacket);

	// Retrieve base ItemData
	auto itemData = ServiceLocator::getItemDataStore()->get(payload->mItemID);
	EXPECTED(itemData);

	Item* item = new Item(itemData);

	item->setEvolvingLevel(payload->mCurrentEvolvingLevel);
	item->setOrnamentationIcon(payload->mOrnamentationIcon);
	
	// NOTE: Untested!
	// Add required augmentations.
	for (auto i = 0; i < 5; i++) {
		if (payload->mAugments[i] != 0) {
			// Retrieve augmentation ItemData.
			auto augmentationItemData = ServiceLocator::getItemDataStore()->get(payload->mAugments[i]);
			EXPECTED(augmentationItemData);

			// NOTE: Memory is freed when item is deleted.
			item->setAugmentation(i, new Item(augmentationItemData));
		}
	}

	sendItemView(item);
	delete item;
}

void ZoneConnection::_handleItemView(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	Log::info("Got OP_ItemViewUnknown. Size=" + std::to_string(pPacket->size));
}

void ZoneConnection::_handleMoveCoin(const EQApplicationPacket* pPacket) {
	if (!_handleMoveCoinImpl(pPacket)) {
		inventoryError();
	}
}

const bool ZoneConnection::_handleMoveCoinImpl(const EQApplicationPacket* pPacket) {
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
	EXPECTED_BOOL(CurrencySlot::isTrade(payload->mFromSlot) == false); // UF does not move currency from trade slots.
	EXPECTED_BOOL(payload->mAmount > 0); // There is no natural way for UF to send 0/negative values.

	auto isBankingSlot = [](const uint32 pSlot) { return pSlot == CurrencySlot::Bank || pSlot == CurrencySlot::SharedBank; };
	const bool isConversion = payload->mFromType != payload->mToType;
	const bool isBankSlot = isBankingSlot(payload->mFromSlot) || isBankingSlot(payload->mToSlot);
	const bool bankRequired = isConversion || isBankSlot;

	// Check: Currency moving into Shared Bank is platinum only.
	if (CurrencySlot::isSharedBank(payload->mToSlot)) {
		EXPECTED_BOOL(payload->mToType == CurrencyType::Platinum);
	}
	// Check: Currency moving from Shared Bank is platinum only.
	if (CurrencySlot::isSharedBank(payload->mFromSlot)) {
		EXPECTED_BOOL(payload->mToType == CurrencyType::Platinum);
	}

	// Check: Character is in range of a bank.
	if (bankRequired && !mZone->canBank(mCharacter)) {
		// TODO: Cheater.
		mCharacter->notify("Cheat Detected! Expect to be contacted shortly.");
	}

	// Check: Character is trading.
	if (CurrencySlot::isTrade(payload->mToSlot)) {
		EXPECTED_BOOL(mCharacter->isTrading());
	}

	// Move.
	EXPECTED_BOOL(mCharacter->getInventory()->moveCurrency(payload->mFromSlot, payload->mToSlot, payload->mFromType, payload->mToType, payload->mAmount));

	return true;
}

void ZoneConnection::sendCurrencyUpdate() {
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

void ZoneConnection::_handleCrystalCreate(const EQApplicationPacket* pPacket) {
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

	auto item = ServiceLocator::getItemFactory()->make(itemID, stacks);
	EXPECTED(item);
	mCharacter->getInventory()->pushCursor(item);
	sendItemSummon(item);

	// Notify user.
	mCharacter->notify("You have created (" + std::to_string(stacks) + ") " + crystalName);

	// Update client.
	sendCrystals();
}

void ZoneConnection::_handleCrystalReclaim(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::sendCrystals() {
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

void ZoneConnection::_handleUnknown(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	Log::info("Unknown Packet, size=" + std::to_string(pPacket->size));
	auto raw = static_cast<EQRawApplicationPacket*>(const_cast<EQApplicationPacket*>(pPacket));
	Log::info("OpCode= " + std::to_string(raw->GetRawOpcode()));
}

void ZoneConnection::_handleEnvironmentalDamage(const EQApplicationPacket* pPacket)
{
	sendHealthUpdate();
}

void ZoneConnection::sendPopup(const String& pTitle, const String& pText) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = PopupWindow::construct(pTitle, pText);
	sendPacket(packet);
	safe_delete(packet);
}

void ZoneConnection::_handlePopupResponse(const EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
}

void ZoneConnection::_handleClaimRequest(const EQApplicationPacket* pPacket)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void ZoneConnection::sendItemView(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketViewLink);

	auto packet = new EQApplicationPacket(OP_ItemLinkResponse, data, payloadSize);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendItemSummon(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketSummonItem);

	auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendItemTrade(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketTrade);

	auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendItemShop(Item* pItem) {
	EXPECTED(pItem);
	EXPECTED(mConnected);

	uint32 payloadSize = 0;
	const unsigned char* data = pItem->copyData(payloadSize, Payload::ItemPacketMerchant);

	auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
	sendPacket(packet);
	delete packet;
}


void ZoneConnection::_handleAugmentItem(const EQApplicationPacket* pPacket) {
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

void ZoneConnection::sendDeleteItem(const uint32 pSlot, const uint32 pStacks, const uint32 pToSlot) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = DeleteItem::construct(pSlot, pToSlot, pStacks);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendMoveItem(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStacks) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = MoveItem::construct(pFromSlot, pToSlot, pStacks);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendTradeRequestAcknowledge(const uint32 pToSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = TradeRequestAcknowledge::construct(mCharacter->getSpawnID(), pToSpawnID);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendTradeFinished() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishTrade, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendTradeCancel(const uint32 pToSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	//auto packet = TradeCancel::construct(mCharacter->getSpawnID(), pToSpawnID);
	auto packet = TradeCancel::construct(mCharacter->getSpawnID(), mCharacter->getSpawnID());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendFinishWindow() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishWindow, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendFinishWindow2() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishWindow, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleAugmentInfo(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AugmentInformation::sizeCheck(pPacket));

	auto payload = AugmentInformation::convert(pPacket);
	Log::info(payload->_debug());

	//sendReadBook(payload->mWindow, 0, 2, "You must use a <c \"#FF0000\">monster cock</c> to remove this augment safely.");
}

void ZoneConnection::sendReadBook(const uint32 pWindow, const uint32 pSlot, const uint32 pType, const String& pText) {
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

void ZoneConnection::_handleReadBook(const EQApplicationPacket* pPacket) {
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
		Log::info("Found <u>Item</u> " + item->getName());
		String text = "\x07 This item is ";
		String texts[4] = {
			"Common",
			"<c \"#336699\">Magic</c>",
			"<c \"#FFCC00\">Rare</c>",
			"an <c \"#FF6600\">Artifact</c>",
		};
		text += texts[Random::make(3)] + " \x07";
		sendReadBook(payload->mWindow, payload->mSlot, payload->mType, text);
		// Check: Found Item is not in a worn slot.
		if (SlotID::isWorn(item->getSlot()) == false) {
			const uint32 slots = item->getSlots();

			//// Hardcode.
			//if (slots == EquipSlots::Chest) {
			//	auto currentChestItem = mCharacter->getInventory()->getItem(SlotID::CHEST);
			//	if (currentChestItem) {
			//		String comparison;
			//		currentChestItem->compare(item, comparison);
			//		sendReadBook(payload->mWindow, payload->mSlot, payload->mType, comparison);
			//	}
			//}
		}
	}
}

void ZoneConnection::_handleCombine(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(Combine::sizeCheck(pPacket));

	auto payload = Combine::convert(pPacket);
	Log::info(payload->_debug());

	mZone->onCombine(mCharacter, payload->mSlot);
}

void ZoneConnection::sendCombineReply() {
	EXPECTED(mConnected);
	auto packet = new EQApplicationPacket(OP_TradeSkillCombine, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleShopRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ShopRequest::sizeCheck(pPacket));

	// Check: Character is in a state that allows for shopping.
	EXPECTED(mCharacter->canShop());

	auto payload = ShopRequest::convert(pPacket);
	Log::info(payload->_debug());

	EXPECTED(mCharacter->getSpawnID() == payload->mCharacterSpawnID); // Sanity.

	mZone->handleShopRequest(mCharacter, payload->mNPCSpawnID);
}

void ZoneConnection::sendShopRequestReply(const uint32 pNPCSpawnID, const uint32 pAction, const float pRate) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ShopRequest::construct(pNPCSpawnID, mCharacter->getSpawnID(), pAction, pRate);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleShopEnd(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ShopEnd::sizeCheck(pPacket));

	auto payload = ShopEnd::convert(pPacket);

	EXPECTED(payload->mCharacterSpawnID == mCharacter->getSpawnID()); // Sanity.
	mZone->handleShopEnd(mCharacter, payload->mNPCSpawnID);
}

void ZoneConnection::sendShopEndReply() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_ShopEndConfirm, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleShopSell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ShopSell::sizeCheck(pPacket));
	EXPECTED(mCharacter->isShopping());

	auto payload = ShopSell::convert(pPacket);
	Log::info(payload->_debug());

	mZone->handleShopSell(mCharacter, payload->mNPCSpawnID, payload->mSlotID, payload->mStacks);
}

void ZoneConnection::sendShopSellReply(const uint32 pSpawnID, const uint32 pSlotID, const uint32 pStacks, const uint32 pPrice) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ShopSell::construct(pSpawnID, pSlotID, pStacks, pPrice);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleShopBuy(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(ShopBuy::sizeCheck(pPacket));
	EXPECTED(mCharacter->isShopping());

	auto payload = ShopBuy::convert(pPacket);
	Log::info(payload->_debug());

	EXPECTED(payload->mCharacterSpawnID == mCharacter->getSpawnID()); // Sanity.
	EXPECTED(payload->mStacks >= 1); // Sanity.

	mZone->handleShopBuy(mCharacter, payload->mNPCSpawnID, payload->mItemInstanceID, payload->mStacks);
}

void ZoneConnection::sendShopBuyReply(const uint32 pSpawnID, const uint32 pItemInstanceID, const uint32 pStacks, const uint64 pPrice, const uint32 pResponse) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ShopBuy::construct(mCharacter->getSpawnID(), pSpawnID, pItemInstanceID, pSpawnID, pPrice, pResponse);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendShopDeleteItem(const uint32 pSpawnID, const uint32 pItemInstanceID, const int32 pUnknown) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ShopDeleteItem::construct(mCharacter->getSpawnID(), pSpawnID, pItemInstanceID, pUnknown);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendAddNimbus(const uint32 pSpawnID, const uint32 pEffectID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = AddNimbus::create();
	auto payload = AddNimbus::convert(packet);

	payload->mNimbusID = pEffectID;
	payload->mSpawnID = pSpawnID;
	payload->mSpawnID2 = pSpawnID;

	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendRemoveNimbus(const uint32 pNimbusID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = RemoveNimbus::create();
	auto payload = RemoveNimbus::convert(packet);

	payload->mNimbusID = pNimbusID;
	payload->mSpawnID = mCharacter->getSpawnID();

	sendPacket(packet);
	delete packet;
}

#pragma pack(1)
struct Header {
	enum Action : uint32 { Update = 7, Populate = 8, };
	uint32 mAction = 0;
};

struct PopulateEntry {
	uint32 mCurrencyID = 0; // This value affects the 'name' column in the 'Alt Currency' tab. X^18^ where X = mCurrencyID.
	uint32 mUnknown0 = 1; // Copied.
	uint32 mCurrencyID2 = 0; // = mCurrencyID
	uint32 mItemID = 0;
	uint32 mIcon = 0;
	uint32 mMaxStacks = 0;
	uint8 mUnknown1 = 0;
};

struct PopulateAlternateCurrencies {
	Header mHeader;
	uint32 mCount = 0;
	// 0..n PopulateEntry
};

#pragma pack()

void ZoneConnection::sendAlternateCurrencies() {
	EXPECTED(mConnected);

	auto currencies = ServiceLocator::getAlternateCurrencyManager()->getCurrencies();

	// Calculate payload size.
	PopulateAlternateCurrencies populate;
	populate.mHeader.mAction = Header::Populate;
	populate.mCount = currencies.size();

	uint32 size = 0;
	size += sizeof(PopulateAlternateCurrencies);
	size += sizeof(PopulateEntry) * populate.mCount;

	// Allocate memory
	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);

	ds.write<PopulateAlternateCurrencies>(populate);

	for (auto i : currencies) {
		PopulateEntry entry;
		entry.mCurrencyID = entry.mCurrencyID2 = i->mCurrencyID;
		entry.mIcon = i->mIcon;
		entry.mItemID = i->mItemID;
		entry.mMaxStacks = i->mMaxStacks;
		ds.write<PopulateEntry>(entry);
	}

	auto packet = new EQApplicationPacket(OP_AltCurrency, data, size);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendAlternateCurrencyQuantities(const bool pSendZero) {
	EXPECTED(mConnected);

	auto currencies = ServiceLocator::getAlternateCurrencyManager()->getCurrencies();
	for (auto i : currencies) {
		const uint32 quantity = mCharacter->getInventory()->getAlternateCurrencyQuantity(i->mCurrencyID);
		if (quantity > 0 || pSendZero)
			sendAlternateCurrencyQuantity(i->mCurrencyID, quantity);
	}
}

void ZoneConnection::sendAlternateCurrencyQuantity(const uint32 pCurrencyID, const uint32 pQuantity) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = UpdateAlternateCurrency::construct(mCharacter->getName(), pCurrencyID, pQuantity);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendAlternateCurrencyQuantity(const uint32 pCurrencyID) {
	EXPECTED(mConnected);
	sendAlternateCurrencyQuantity(pCurrencyID, mCharacter->getInventory()->getAlternateCurrencyQuantity(pCurrencyID));
}

void ZoneConnection::_handleAlternateCurrencyReclaim(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AlternateCurrencyReclaim::sizeCheck(pPacket));

	auto payload = AlternateCurrencyReclaim::convert(pPacket);
	Log::info(payload->_debug());

	// Handle: Create.
	if (payload->mAction == AlternateCurrencyReclaim::Create) {
		// NOTE: Underfoot will allow currencies to be summoned to the cursor even when there are Items on it.
		if (mCharacter->getInventory()->isCursorEmpty() == false) {
			mCharacter->notify("Please clear your cursor and try again.");
			return;
		}

		// Check: Currency Type.
		const uint32 itemID = ServiceLocator::getAlternateCurrencyManager()->getItemID(payload->mCurrencyID);
		if (itemID == 0) {
			// TODO: Logging.
			return;
		}

		// Check: Stacks.
		const uint32 currentQuantity = mCharacter->getInventory()->getAlternateCurrencyQuantity(payload->mCurrencyID);
		if (currentQuantity < payload->mStacks) {
			// TODO: Logging.
			return;
		}

		// Update alternate currency.
		mCharacter->getInventory()->removeAlternateCurrency(payload->mCurrencyID, payload->mStacks);

		// Create Item
		auto item = ServiceLocator::getItemFactory()->make(itemID, payload->mStacks);
		EXPECTED(item);

		// Add to cursor.
		mCharacter->getInventory()->pushCursor(item);
		sendItemSummon(item);

		// Notify user.
		mCharacter->notify("You have created (" + std::to_string(item->getStacks()) + ") " + item->getName()+"s");

		// Update client.
		sendAlternateCurrencyQuantity(payload->mCurrencyID);
	}
	// Handle: Reclaim.
	else if (payload->mAction == AlternateCurrencyReclaim::Reclaim) {
		// Check: Character has an Item on the cursor.
		Item* item = mCharacter->getInventory()->peekCursor();
		if (!item) {
			mCharacter->notify("Please place an alternate currency on your cursor and try again.");
			return;
		}

		// Check: Item on cursor is alternate currency Item.
		const uint32 currencyID = ServiceLocator::getAlternateCurrencyManager()->getCurrencyID(item->getID());
		if (currencyID == 0) {
			mCharacter->notify("Please place an alternate currency on your cursor and try again.");
			return;
		}

		const uint32 stacks = item->getStacks();
		String currencyName = item->getName();

		// Update alternate currency.
		mCharacter->getInventory()->addAlternateCurrency(currencyID, stacks);
		
		// Consume from Inventory.
		EXPECTED(mCharacter->getInventory()->consume(SlotID::CURSOR, stacks));

		// Clear cursor.
		sendMoveItem(SlotID::CURSOR, SlotID::SLOT_DELETE);

		// Notify user.
		mCharacter->notify("You have reclaimed (" + std::to_string(stacks) + ") " + currencyName+"s");

		// Update client.
		sendAlternateCurrencyQuantity(currencyID);
	}
	else {
		// Unknown Action.
		// TODO: Logging.
		return;
	}
}

void ZoneConnection::sendMOTD(const String& pMOTD) {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_MOTD, reinterpret_cast<const unsigned char*>(pMOTD.c_str()), pMOTD.size() + 1);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleRandomRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(RandomRequest::sizeCheck(pPacket));

	auto payload = RandomRequest::convert(pPacket);

	mZone->handleRandomRequest(mCharacter, payload->mLow, payload->mHigh);
}

void ZoneConnection::_handleDropItem(const EQApplicationPacket* pPacket) {
	mZone->handleDropItem(mCharacter);
}

void ZoneConnection::sendObject(Object* pObject) {
	EXPECTED(mConnected);
	EXPECTED(pObject);

	u32 payloadSize = 0;
	payloadSize += pObject->getAsset().length() + 1 + 57;

	char* data = new char[payloadSize];

	Utility::DynamicStructure ds(data, payloadSize);

	ds.write<u32>(1); // Drop ID.
	ds.writeString(pObject->getAsset());
	ds.write<u16>(mZone->getID());
	ds.write<u16>(mZone->getInstanceID());
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(0); // Unknown.
	ds.write<float>(pObject->getHeading());
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(0); // Unknown.
	ds.write<float>(pObject->getSize());
	ds.write<float>(pObject->getPosition().y);
	ds.write<float>(pObject->getPosition().x);
	ds.write<float>(pObject->getPosition().z);
	ds.write<u32>(pObject->getType());
	ds.write<u32>(0xFFFFFFFF); // Unknown.
	ds.write<u32>(0); // Unknown.
	ds.write<u8>(0); // Unknown.

	if (ds.check() == false) {
		Log::error("[ObjectSpawn] Bad Write: Written: " + std::to_string(ds.getBytesWritten()) + " Size: " + std::to_string(ds.getSize()));
	}

	auto packet = new EQApplicationPacket(OP_GroundSpawn, reinterpret_cast<const unsigned char*>(data), payloadSize);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendManaUpdate() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ManaUpdate::construct(mCharacter->getSpawnID(), mCharacter->getCurrentMana(), mCharacter->getMaximumMana());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendEnduranceUpdate() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = EnduranceUpdate::construct(mCharacter->getSpawnID(), mCharacter->getCurrentEndurance(), mCharacter->getMaximumEndurance());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_handleXTargetAutoAddHaters(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone::ExtendedTarget;
	EXPECTED(pPacket);
	EXPECTED(AutoAddHaters::sizeCheck(pPacket));

	auto payload = AutoAddHaters::convert(pPacket);
	mCharacter->getXTargetController()->setAutoAddHaters(payload->mAction == 1 ? true : false);
}

void ZoneConnection::sendRespawnWindow() {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	const auto respawnOptions = mCharacter->getRespawnOptions();
	EXPECTED(respawnOptions);
	const auto options = respawnOptions->getOptions();

	// Calculate payload size.
	u32 payloadSize = 16; // Fixed header size.
	payloadSize += 26 * respawnOptions->getNumOptions(); // Fixed option size.
	for (auto i : options)
		payloadSize += i.mName.length() + 1; // Variable option name size.
	
	auto packet = new EQApplicationPacket(OP_RespawnWindow, payloadSize);
	Utility::DynamicStructure ds(packet->pBuffer, payloadSize);

	// Write header.
	ds.write<u32>(0); // Default selection.
	ds.write<u32>(30000); // Timer.
	ds.write<u32>(0); // Unknown.
	ds.write<u32>(respawnOptions->getNumOptions());

	// Write options.
	for (auto i : options) {
		ds.write<u32>(i.mID);
		ds.write<u16>(i.mZoneID);
		ds.write<u16>(i.mInstanceID);
		ds.write<float>(i.mPosition.x);
		ds.write<float>(i.mPosition.y);
		ds.write<float>(i.mPosition.z);
		ds.write<float>(i.mHeading);
		ds.writeString(i.mName);
		ds.write<u8>(i.mType);
	}

	sendPacket(packet);
	delete packet;
	EXPECTED(ds.check());
}

void ZoneConnection::_handleRespawnWindowSelect(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(RespawnWindowSelect::sizeCheck(pPacket));
	EXPECTED(mCharacter->getRespawnOptions()->isActive()); // Sanity.

	auto payload = RespawnWindowSelect::convert(pPacket);
	Log::info(payload->_debug());

	mZone->handleRespawnSelection(mCharacter, payload->mSelection);
}

void ZoneConnection::_handleAAAction(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(AAAction::sizeCheck(pPacket));

	auto payload = AAAction::convert(pPacket);
	Log::info(payload->_debug());

	EXPECTED(Utility::inRange<u32>(payload->mExperienceToAA, 0, 100)); // Sanity.

	auto controller = mCharacter->getExperienceController();

	// TODO: The client blocks this until 51 iirc.
	if (payload->mAction == 1) {
		if (controller->isAAOn() == false)
			// " Alternate Experience is *ON*."
			sendAAExperienceOnMessage();

		controller->setExperienceToAA(payload->mExperienceToAA);
		mCharacter->notify("Experience to AA set to " + std::to_string(controller->getExperienceToAA()) + "%");
	}
	else if (payload->mAction == 2) {
		// " Alternate Experience is *OFF*."
		sendAAExperienceOffMessage();
		mCharacter->getExperienceController()->setExperienceToAA(0);
	}
}

void ZoneConnection::_handleLeadershipExperienceToggle(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	EXPECTED(pPacket);
	EXPECTED(LeadershipExperienceToggle::sizeCheck(pPacket));

	auto payload = LeadershipExperienceToggle::convert(pPacket);
	Log::info(payload->_debug());

	EXPECTED(payload->mValue == 1 || payload->mValue == 0); // Sanity.

	auto controller = mCharacter->getExperienceController();

	// Turning leadership experience on.
	if (payload->mValue == 1) {
		controller->setLeadershipExperience(true);
		sendSimpleMessage(MessageType::Experience, StringID::LeadershipOn);
	}
	// Turning leadership experience off.
	else {
		controller->setLeadershipExperience(false);
		sendSimpleMessage(MessageType::Experience, StringID::LeadershipOff);
	}	
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
