#include "ZoneConnection.h"
#include "ServiceLocator.h"
#include "GuildManager.h"
#include "Guild.h"
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
#include "Account.h"

#include "../common/MiscFunctions.h"
#include "../common/packet_dump_file.h"

ZoneConnection::ZoneConnection() {
	mForceSendPositionTimer.Disable();
}

ZoneConnection::~ZoneConnection() {
	dropConnection();
	if (mStreamInterface)
		mStreamInterface->ReleaseFromUse();
	// NOTE: mStreamInterface is intentionally not deleted here.

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

const bool ZoneConnection::initialise(EQStreamInterface* pStreamInterface, ILogFactory* pLogFactory, Zone* pZone, GuildManager* pGuildManager) {
	if (mInitialised) return false;
	if (!pStreamInterface) return false;
	if (!pLogFactory) return false;
	if (!pZone) return false;
	if (!pGuildManager) return false;

	mStreamInterface = pStreamInterface;
	mLog = pLogFactory->make();
	mZone = pZone;
	mGuildManager = pGuildManager;
	mConnected = true;
	mConnectTime = Utility::Time::now();

	updateLogContext();

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

void ZoneConnection::updateLogContext() {
	StringStream context;
	context << "[ZoneConnection (IP: " << mStreamInterface->GetRemoteIP() << " Port: " << mStreamInterface->GetRemotePort();

	if (mCharacter) {
		auto account = mCharacter->getAccount();
		context << " LSID: " << account->getLoginServerID() << " LSAID: " << account->getLoginAccountID() << " LSAN: " << account->getLoginAccountName();
	}

	context << ")]";
	mLog->setContext(context.str());
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
		ret = handlePacket(packet);
		delete packet;
	}
}

void ZoneConnection::dropConnection() {
	mConnected = false;
	if (mStreamInterface)
		mStreamInterface->Close();
}

bool ZoneConnection::handlePacket(const EQApplicationPacket* pPacket) {
	if (!pPacket) return false;

	if (!mStreamInterface->CheckState(ESTABLISHED)) return false;

	const EmuOpcode opcode = pPacket->GetOpcode();

	// Not fully connected.
	if (mConnectingStatus < ZCStatus::Complete) {
		switch (opcode) {
		case OP_ZoneEntry:
			return handleZoneEntry(pPacket);
		case OP_ReqClientSpawn:
			return handleRequestClientSpawn(pPacket);
		case OP_ClientReady:
			return handleClientReady(pPacket);
		case OP_ReqNewZone:
			// UF sends this but still works when there is no reply.
			return handleRequestNewZoneData(pPacket);
		default:
			break;
		}
		return true;
	}

	if (/*opcode == 0 || */opcode == OP_FloatListThing) return true;

	switch (opcode) {
	case OP_Unknown:
		//handleUnknown(pPacket);
		break;
	case OP_AckPacket:
		// Ignore.
		break;
	case OP_SetServerFilter:
		handleSetServerFiler(pPacket);
		break;
	case OP_SendAATable:
		handleSendAATable(pPacket);
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
	case OP_SpawnAppearance:
		handleSpawnAppearance(pPacket);
		break;
	case OP_WearChange:
		handleWearChange(pPacket);
		break;
	case OP_ClientUpdate:
		// NOTE: Sent when a Character moves
		// NOTE: Sent automatically every X seconds by the client.
		handleClientUpdate(pPacket);
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
		handleTGB(pPacket);
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
	case OP_UpdateAA:
		handleUpdateAA(pPacket);
		break;
	case OP_BlockedBuffs:
		// Ignore. Not Implemented.
		break;
	case OP_XTargetRequest:
		Utility::print("[UNHANDLED OP_XTargetRequest]");
		break;
	case OP_XTargetAutoAddHaters:
		// NOTE: This occurs when the user clicks the 'Auto Add Hater Targets' from the 'Extended Target' window.
		handleXTargetAutoAddHaters(pPacket);
		break;
	case OP_GetGuildsList:
		Utility::print("[UNHANDLED OP_GetGuildsList]");
		break;
	case OP_TargetMouse:
	case OP_TargetCommand:
		// NOTE: This is sent when the current target dies.
		handleTarget(pPacket);
		break;
	case OP_Camp:
		// Sent when user types /camp or presses the camp button.
		handleCamp(pPacket);
		break;
	case OP_Logout:
		// This occurs 30 seconds after /camp
		handleLogOut(pPacket);
		return false;
	case OP_DeleteSpawn:
		// Client sends this after /camp
		// NOTE: Sent as a Character is about to zone out.
		handleDeleteSpawn(pPacket);
		break;
	case OP_ChannelMessage:
		handleChannelMessage(pPacket);
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
		handleEmote(pPacket);
		break;
	case OP_Animation:
		handleAnimation(pPacket);
		break;
	case OP_Save:
		handleSaveRequest(pPacket);
		break;
	case OP_SaveOnZoneReq:
		Utility::print("[UNHANDLED OP_SaveOnZoneReq]");
		break;
	case OP_FaceChange:
		// NOTE: This occurs when the player presses 'Accept' in the Face interface.
		handleFaceChange(pPacket);
		break;
	case OP_WhoAllRequest:
		handleWhoRequest(pPacket);
		break;
	case OP_GroupInvite:
	case OP_GroupInvite2:
		// NOTE: This occurs when the player presses 'Invite' on the group window.
		// NOTE: This also occurs when the player uses the /invite command.
		// NOTE: This also occurs when using the 'Invite/follow' mapped command (Options->Keys).
		handleGroupInvite(pPacket);
		break;
	case OP_GroupFollow:
	case OP_GroupFollow2:
		// NOTE: This occurs when the player presses 'Follow' on the group window.
		// NOTE: This also occurs when the player uses the /invite command when they have a current group invitation.
		// NOTE: This also occurs when using the 'Invite/follow' mapped command (Options->Keys).
		handleGroupAcceptInvite(pPacket);
		break;
	case OP_GroupCancelInvite:
		// NOTE: This occurs when the player presses 'Decline' on the group window.
		// NOTE: This also occurs when the player uses the /disband command when they have a current group invitation.
		// NOTE: This also occurs when using the 'Disband' mapped command (Options->Keys).
		handleGroupDeclineInvite(pPacket);
		break;
	case OP_GroupDisband:
		// NOTE: This occurs when the player presses 'Disband' on group window.
		// NOTE: This occurs when the player uses the /disband command.
		handleGroupDisband(pPacket);
		break;
	case OP_GroupMakeLeader:
		// NOTE: This occurs when the player uses the /makeleader command.
		// NOTE: This occurs when the player uses the context menu on the group window (Roles->Leader).
		handleGroupMakeLeader(pPacket);
		break;
	case OP_GuildCreate:
		// NOTE: This occurs when the player uses the /guildcreate command.
		handleGuildCreate(pPacket);
		break;
	case OP_GuildDelete:
		// Note: This occurs when the player uses the /guilddelete command.
		handleGuildDelete(pPacket);
		break;
	case OP_GuildInvite:
		// NOTE: This occurs when a player uses the /guildinvite command.
		handleGuildInvite(pPacket);
		break;
	case OP_GuildInviteAccept:
		// NOTE: This occurs when a player presses 'Yes' or 'No' on the 'guild invite window'.
		// NOTE: UF automatically sends a decline after about 10 seconds if the user has not responded.
		handleGuildInviteResponse(pPacket);
		break;
	case OP_GuildRemove:
		// NOTE: This occurs when the player uses the /guildremove command.
		handleGuildRemove(pPacket);
		break;
	case OP_SetGuildMOTD:
		// NOTE: This occurs when the player uses the /guildmotd command.
		handleGuildSetMOTD(pPacket);
		break;
	case OP_GetGuildMOTD:
		// NOTE: This occurs when the player uses the /getguildmotd command.
		handleGuildMOTDRequest(pPacket);
		break;
	case OP_GuildUpdateURLAndChannel:
		// NOTE: This occurs via the guild window.
		handleSetGuildURLOrChannel(pPacket);
		break;
	case OP_GuildPublicNote:
		// NOTE: This occurs via the guild window.
		handleSetGuildPublicNote(pPacket);
		break;
	case OP_GuildStatus:
		// NOTE: This occurs when the player uses the /guildstatus command.
		handleGetGuildStatus(pPacket);
		break;
	case OP_GuildDemote:
		handleGuildDemote(pPacket);
		break;
	case OP_GuildManageBanker:
		handleGuildSetFlags(pPacket);
		break;
	case OP_GuildLeader:
		handleGuildMakeLeader(pPacket);
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
		handleZoneChange(pPacket);
		break;
	case OP_AutoAttack:
		handleAutoAttack(pPacket);
		break;
	case OP_AutoAttack2:
		// Ignore. This sent directly after OP_AutoAttack. I don't know why.
		break;
		// Spells Begin
	case OP_MemorizeSpell:
		handleMemoriseSpell(pPacket);
		break;
	case OP_DeleteSpell:
		handleDeleteSpell(pPacket);
		break;
	case OP_LoadSpellSet:
		handleLoadSpellSet(pPacket);
		break;
	case OP_SwapSpell:
		handleSwapSpell(pPacket);
		break;
	case OP_CastSpell:
		handleCastSpell(pPacket);
		break;
		// Spells End
	case OP_CombatAbility:
		handleCombatAbility(pPacket);
		break;
	case OP_Taunt:
		handleTaunt(pPacket);
		break;
	case OP_Consider:
		handleConsider(pPacket);
		break;
	case OP_ConsiderCorpse:
		handleConsiderCorpse(pPacket);
		break;
	case OP_Surname:
		// NOTE: This occurs when the player uses the /surname command.
		handleSurname(pPacket);
		break;
	case OP_ClearSurname:
		// NOTE: This occurs when the player uses the /surname command with no text
		handleClearSurname(pPacket);
		break;
	case OP_GMLastName:
		// NOTE: This occurs when the player uses the /lastname command.
		handleGMLastName(pPacket);
		break;
	case OP_SetTitle:
		// NOTE: This occurs when the player presses the 'Change Title', 'Clear Title', 'Change Suffix' or 'Clear Suffix' buttons.
		handleSetTitle(pPacket);
		break;
	case OP_RequestTitles:
		// NOTE: This occurs when the player opens the title window.
		handleRequestTitles(pPacket);
		break;
	case OP_LootRequest:
		handleBeginLootRequest(pPacket);
		break;
	case OP_EndLootRequest:
		handleEndLootRequest(pPacket);
		break;
	case OP_LootItem:
		handleLootItem(pPacket);
		break;
	case OP_MoveItem:
		handleMoveItem(pPacket);
		break;
	case OP_Consume:
		handleConsume(pPacket);
		break;
	case OP_ItemVerifyRequest:
		// NOTE: UF Requires a reply or the UI will lock up.
		// NOTE: This occurs when the player right clicks on any Item (except containers).
		handleItemRightClick(pPacket);
		break;
	case OP_PotionBelt:
		handlePotionBelt(pPacket);
		break;
	case OP_OpenContainer:
		// NOTE: This occurs when the player opens a container.
		handleOpenContainer(pPacket);
		break;
	case OP_TradeRequest:
		// NOTE: This occurs when a player left clicks on an NPC with an Item on the cursor.
		handleTradeRequest(pPacket);
		break;
	case OP_TradeRequestAck:
		handleTradeRequestAck(pPacket);
		break;
	case OP_CancelTrade:
		// NOTE: This occurs when a player presses the 'Cancel' button whilst trading with an NPC.
		handleCancelTrade(pPacket);
		break;
	case OP_TradeAcceptClick:
		// NOTE: This occurs when a player presses 'Give' button whilst trading with an NPC.
		// NOTE: This occurs when a player presses the 'Trade' in the 'Trade Window'.
		handleAcceptTrade(pPacket);
		break;
	case OP_TradeBusy:
		handleTradeBusy(pPacket);
		break;
	case OP_ItemLinkClick:
		// Note: This occurs when a player clicks on an Item Link.
		handleItemLinkClick(pPacket);
		break;
	case OP_ItemViewUnknown:
		handleItemView(pPacket);
		break;
	case OP_MoveCoin:
		handleMoveCurrency(pPacket);
		break;
	case OP_CrystalCreate:
		// NOTE: This occurs when a player selects Radiant or Ebon Crystals from within the inventory window.
		handleCrystalCreate(pPacket);
		break;
	case OP_CrystalReclaim:
		// NOTE: This occurs when a player clicks the 'Reclaim' button in the inventory window.
		// NOTE: No payload is sent (Size=0)
		handleCrystalReclaim(pPacket);
		break;
	case OP_EvolvingItem:
		Log::error("OP_EvolvingItem");
		break;
	case OP_EnvDamage:
		handleEnvironmentDamage(pPacket);
		break;
	case OP_PopupResponse:
		handlePopupResponse(pPacket);
		break;
	case OP_VetClaimRequest:
		// NOTE: This occurs when a player clicks the 'Refresh' button in the Claim Window.
		handleClaimRequest(pPacket);
		break;
	case OP_AugmentItem:
		// NOTE: This occurs when the player presses 'Insert' on the Augmentation Sealer.
		handleAugmentItem(pPacket);
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
		handleAugmentInfo(pPacket);
		break;
	case OP_ReadBook:
		handleReadBook(pPacket);
		break;
	case OP_TradeSkillCombine:
		// NOTE: This occurs when a player presses the 'Combine' button on an opened container.
		handleCombine(pPacket);
		break;
	case OP_ShopRequest:
		// NOTE: This occurs when a player right clicks on a merchant.
		handleShopRequest(pPacket);
		break;
	case OP_ShopEnd:
		// NOTE: This occurs when a player presses the 'Done' button on the merchant window.
		// NOTE: This occurs when a player presses 'escape' whilst a merchant window is open.
		// NOTE: This occurs when a player moves their character too far away from a merchant.
		handleShopEnd(pPacket);
		break;
	case OP_ShopPlayerSell:
		// NOTE: This occurs when a player tries to sell an Item to a merchant.
		handleShopSell(pPacket);
		break;
	case OP_ShopPlayerBuy:
		// NOTE: This occurs when a player tries to buy an Item from a merchant.
		handleShopBuy(pPacket);
		break;
	case OP_AltCurrencyReclaim:
		// NOTE: This occurs when a player presses the 'Reclaim' button in the 'Alt. Currency' tab (Inventory window).
		// NOTE: This occurs when a player presses the 'Create' button in the 'Alt. Currency' tab (Inventory window).
		handleAlternateCurrencyReclaim(pPacket);
		break;
	case OP_RandomReq:
		// NOTE: This occurs when a player enters the /random command.
		handleRandomRequest(pPacket);
		break;
	case OP_GroundSpawn:
		// NOTE: This occurs when a player drops an Item on the ground.
		handleDropItem(pPacket);
		break;
	case OP_RespawnWindow:
		// NOTE: This occurs when a player selects a respawn option from the 'Respawn Window'.
		handleRespawnWindowSelect(pPacket);
		break;
	case OP_AAAction:
		// NOTE: This occurs when a player adjusts the 'Exp to AA' setting in the 'Alternate Advancement Window'.
		handleAAAction(pPacket);
		break;
	case OP_LeadershipExpToggle:
		// NOTE: This occurs when a player uses the 'Leadership Exp' toggle in the 'Leadership Window'
		handleLeadershipExperienceToggle(pPacket);
		break;
	case OP_PetCommands:
		handlePetCommand(pPacket);
		break;
	case OP_ApplyPoison:
		handleApplyPoison(pPacket);
		break;
	case OP_RaidInvite:
		handleRaidInvite(pPacket);
		break;
	case OP_FindPersonRequest:
		handleFindPersonRequest(pPacket);
		break;
	case OP_InspectRequest:
		handleInspectRequest(pPacket);
		break;
	case OP_InspectMessageUpdate:
		handleSetInspectMessage(pPacket);
		break;
	case OP_BuffRemoveRequest:
		handleRemoveBuffRequest(pPacket);
		break;
	case OP_GroupRoles:
		handleGroupRoleChange(pPacket);
		break;
	default:
		//StringStream ss;
		//ss << "Unknown Packet: " << opcode;
		//Utility::print(ss.str());z
		break;
	}
	return true;
}

const bool ZoneConnection::handleZoneEntry(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(ZoneEntry::sizeCheck(pPacket));
	
	auto payload = ZoneEntry::convert(pPacket);
	
	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	String characterName(payload->mCharacterName);

	// Check: Connecting packet sequence.
	if (mConnectingStatus != ZCStatus::None) {
		mLog->error("Got unexpected OP_ZoneEntry, dropping connection.");
		dropConnection();
		return false;
	}
	// Update connecting status.
	mConnectingStatus = ZCStatus::ZoneEntryReceived;

	// Check: Name is of valid length.
	if (!Limits::Character::nameLength(characterName)) {
		mLog->error("Invalid name length.");
		dropConnection();
		return false;
	}

	mLog->info("Got OP_ZoneEntry from " + characterName);

	// Retrieve Character
	mCharacter = ServiceLocator::getZoneManager()->getZoningCharacter(characterName);
	if (!mCharacter) {
		mLog->error("Unable to find zoning Character.");
		dropConnection();
		return false;
	}

	updateLogContext();
	mLog->info("Identified.");

	// Check: Character authenticated for this Zone.
	EXPECTED_BOOL(mZone->checkAuthentication(mCharacter));
	// Clear ZoneChange
	mCharacter->clearZoneChange();

	mCharacter->setZone(mZone);
	mCharacter->setSpawnID(mZone->getNextSpawnID());
	mCharacter->setConnection(this);

	// REPLY
	_sendGuildNames();
	_sendPlayerProfile();
	mConnectingStatus = ZCStatus::PlayerProfileSent;
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

	return true;
}

const bool ZoneConnection::handleRequestNewZoneData(const EQApplicationPacket* pPacket) {
	if (!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Check: Connecting packet sequence.
	if (mConnectingStatus != ZCStatus::PlayerProfileSent) {
		mLog->error("Got unexpected OP_ReqNewZone, dropping connection.");
		dropConnection();
		return false;
	}

	mConnectingStatus = ZCStatus::ClientRequestZoneData;
	_sendZoneData();

	mConnectingStatus = ZCStatus::ZoneInformationSent;
	return true;
}

const bool ZoneConnection::handleRequestClientSpawn(const EQApplicationPacket* pPacket) {
	if (!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Check: Connecting packet sequence.
	if (mConnectingStatus != ZCStatus::ZoneInformationSent) {
		mLog->error("Got unexpected OP_ReqClientSpawn, dropping connection.");
		dropConnection();
		return false;
	}

	_sendDoors();
	_sendObjects();
	_sendAAStats();
	sendZonePoints();
	_sendZoneServerReady();
	_sendExpZoneIn();
	_sendWorldObjectsSent();

	// Alternate Currency
	sendAlternateCurrencies();
	sendAlternateCurrencyQuantities(false);

	sendMOTD("Welcome to Fading Light.");

	mConnectingStatus = ZCStatus::ClientRequestSpawn;
	return true;
}

const bool ZoneConnection::handleClientReady(const EQApplicationPacket* pPacket) {
	if (!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Check: Connecting packet sequence.
	if (mConnectingStatus != ZCStatus::ClientRequestSpawn) {
		mLog->error("Got unexpected OP_ClientReady, dropping connection.");
		dropConnection();
		return false;
	}

	mForceSendPositionTimer.Start(4000);

	mConnectingStatus = ZCStatus::Complete;
	return true;
}

void ZoneConnection::_sendTimeOfDay() {
	EXPECTED(mConnected);

	auto packet = Payload::Zone::Time::construct(0, 0, 0, 0, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendPlayerProfile() {
	auto packet = Payload::makeCharacterProfile(mCharacter);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendZoneEntry() {
	EXPECTED(mConnected);

	mCharacter->_syncPosition();

	uint32 size = mCharacter->getDataSize();
	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);
	EXPECTED(mCharacter->copyData(writer));
	
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
	using namespace Payload::Zone;
	EXPECTED(mConnected);
	
	auto packet = TributeUpdate::create();
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

void ZoneConnection::sendZonePoints() {
	auto packet = Payload::makeZonePoints(mZone->getZonePoints());
	sendPacket(packet);
	delete packet;
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

const bool ZoneConnection::handleClientUpdate(const EQApplicationPacket* pPacket) {
	using namespace Payload;
	if (!pPacket) return false;
	SIZE_CHECK(PositionUpdate::sizeCheck(pPacket) || pPacket->size == PositionUpdate::size() + 1); // Payload has an extra byte from time to time.

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

		return true;
}

const bool ZoneConnection::handleSpawnAppearance(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(SpawnAppearance::sizeCheck(pPacket));

	auto payload = SpawnAppearance::convert(pPacket);
	const uint16 actionType = payload->mType;
	const uint32 actionParameter = payload->mParameter;

	// Ignore if spawn id does not match this characters ID.
	if (payload->mSpawnID != mCharacter->getSpawnID()) {
		// Note: UF client sends spawn ID (0) and action type (51) every few seconds. Not sure why.
		return true;
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

	return true;
}

const bool ZoneConnection::handleCamp(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(Camp::sizeCheck(pPacket));

	// Instant camp for GM.
	if (mCharacter->isGM()) {
		_sendPreLogOutReply();
		_sendLogOutReply();

		mCharacter->setCampComplete(true);

		dropConnection();
		return true;;
	}

	mCharacter->startCamp();
	return true;
}

struct ChannelMessage {
	String mSender;
	String mTarget;
	u32 mUnknown0 = 0;
	u32 mLanguage = 0;
	u32 mChannel = 0;
	u32 mUnknown1 = 0;
	u8 mUnknown2 = 0;
	u32 mSkill = 0;
	String mMessage;
	String _debug() const {
		StringStream ss;
		ss << "{ChannelMessage} ";
		PRINT_MEMBER(mSender);
		PRINT_MEMBER(mTarget);
		PRINT_MEMBER(mUnknown0);
		PRINT_MEMBER(mLanguage);
		PRINT_MEMBER(mChannel);
		PRINT_MEMBER(mUnknown1);
		PRINT_MEMBER((int)mUnknown2);
		PRINT_MEMBER(mSkill);
		PRINT_MEMBER(mMessage);
		return ss.str();
	}
};
#define CHECKED_READ(pRead) if(!(pRead)) { StringStream ss; ss << "[CHECKED_READ] (" << ARG_STR(pRead) << ") Failed in " << __FUNCTION__; mLog->error(ss.str()); return false; }

const bool ZoneConnection::handleChannelMessage(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;

	Utility::MemoryReader reader(pPacket->pBuffer, pPacket->size);
	static ChannelMessage m;

	CHECKED_READ(reader.readString(m.mSender, 64));
	CHECKED_READ(reader.readString(m.mTarget, 64));
	CHECKED_READ(reader.read<u32>(m.mUnknown0));
	CHECKED_READ(reader.read<u32>(m.mLanguage));
	CHECKED_READ(reader.read<u32>(m.mChannel));
	CHECKED_READ(reader.read<u32>(m.mUnknown1));
	CHECKED_READ(reader.read<u8>(m.mUnknown2));
	CHECKED_READ(reader.read<u32>(m.mSkill));
	CHECKED_READ(reader.readString(m.mMessage, 1500)); // This is just a estimate.

	/* NOTE:
		- This payload consistently has another 15 bytes after the message.
		- Sometimes those values are non-zero and cause the payload size to increase.
		- It is odd and I suspect that it is a client error, therefore we just read the important stuff and ignore the variation.
	*/

	// Notify Zone.
	mZone->onChannelMessage(mCharacter, m.mChannel, m.mSender, m.mTarget, m.mMessage);
	return true;
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

EQApplicationPacket* ZoneConnection::makeMessage(const u32 pType, const String& pMessage, const String& pSenderName) {
	// Calculate size.
	u32 size = 0;

	size += sizeof(u8); // Unknown.
	size += sizeof(u8); // Unknown.
	size += sizeof(u8); // Unknown.
	size += sizeof(u32); // Type.
	size += sizeof(u32); // SpawnID.
	size += sizeof(u32)* 3; // Unknowns.

	size += pMessage.size() + 1;
	size += pSenderName.size() + 1;

	unsigned char * data = new unsigned char[size];
	Utility::MemoryWriter writer(data, size);

	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u32>(pType); // Type.
	writer.write<u32>(0); // SpawnID
	writer.writeString(pSenderName); // Sender name.
	writer.write<u32>(0); // Unknown
	writer.write<u32>(0); // Unknown
	writer.write<u32>(0); // Unknown
	writer.writeString(pMessage); // Sender name.

	return new EQApplicationPacket(OP_SpecialMesg, data, size);
}

void ZoneConnection::sendMessage(const u32 pType, const String& pMessage, const String& pSenderName) {
	EXPECTED(mConnected);

	auto packet = makeMessage(pType, pMessage, pSenderName);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleLogOut(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// TODO: Determine whether this is actually required when the Character is not trading.
	auto packet = TradeCancel::construct(mCharacter->getSpawnID(), 0);
	sendPacket(packet);
	delete packet;

	_sendPreLogOutReply();
	_sendLogOutReply();

	mCharacter->setCampComplete(true);

	// NOTE: Zone picks up the dropped connection next update.
	dropConnection();
	return true;
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

const bool ZoneConnection::handleDeleteSpawn(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(DeleteSpawn::sizeCheck(pPacket));

	_sendLogOutReply();
	mCharacter->setZoningOut();

	// NOTE: Zone picks up the dropped connection next update.
	dropConnection();
	return true;
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
	auto zoneData = ServiceLocator::getZoneDataManager()->getData(mZone->getID());
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

void ZoneConnection::sendAppearance(const u16 pType, const u32 pParameter) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = SpawnAppearance::construct(mCharacter->getSpawnID(), pType, pParameter);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleSendAATable(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);
	// TODO:
	return true;
}

const bool ZoneConnection::handleUpdateAA(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// TODO:
	return true;
}

const bool ZoneConnection::handleTarget(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Target::sizeCheck(pPacket));

	auto payload = Target::convert(pPacket);

	// Notify Zone.
	mZone->onTargetChange(mCharacter, payload->mSpawnID);
	return true;
}

const bool ZoneConnection::handleTGB(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(TGB::sizeCheck(pPacket)) return false;

	auto payload = TGB::convert(pPacket);

	if (payload->mAction == 0 || payload->mAction == 1) {
		mCharacter->setTGB(payload->mAction == 1);
		sendSimpleMessage(MessageType::White, mCharacter->getTGB() ? StringID::TGB_ON : StringID::TGB_OFF);
	}
	// Ignore anything else, including the extra 2 packet UF sends.

	return true;
}

void ZoneConnection::sendSimpleMessage(const u32 pType, const u32 pStringID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = SimpleMessage::construct(pType, pStringID);
	sendPacket(packet);
	delete packet;
}

EQApplicationPacket* ZoneConnection::makeSimpleMessage(const u32 pType, const u32 pStringID, const String& pParameter0, const String& pParameter1, const String& pParameter2, const String& pParameter3, const String& pParameter4, const String& pParameter5, const String& pParameter6, const String& pParameter7, const String& pParameter8, const String& pParameter9) {
	int packetSize = 0;
	packetSize += 4; // Unknown.
	packetSize += 4; // String ID.
	packetSize += 4; // Type.

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

	Utility::MemoryWriter writer(packet->pBuffer, packetSize);

	// Write header.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(pStringID); // String ID.
	writer.write<u32>(pType); // Type.

	// Write variable length strings.
	if (pParameter0.length() != 0) writer.writeString(pParameter0);
	if (pParameter1.length() != 0) writer.writeString(pParameter1);
	if (pParameter2.length() != 0) writer.writeString(pParameter2);
	if (pParameter3.length() != 0) writer.writeString(pParameter3);
	if (pParameter4.length() != 0) writer.writeString(pParameter4);
	if (pParameter5.length() != 0) writer.writeString(pParameter5);
	if (pParameter6.length() != 0) writer.writeString(pParameter6);
	if (pParameter7.length() != 0) writer.writeString(pParameter7);
	if (pParameter8.length() != 0) writer.writeString(pParameter8);
	if (pParameter9.length() != 0) writer.writeString(pParameter9);

	EXPECTED_PTR(writer.check());
	return packet;
}

void ZoneConnection::sendSimpleMessage(const u32 pType, const u32 pStringID, const String& pParameter0, const String& pParameter1, const String& pParameter2, const String& pParameter3, const String& pParameter4, const String& pParameter5, const String& pParameter6, const String& pParameter7, const String& pParameter8, const String& pParameter9) {
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

const bool ZoneConnection::handleEmote(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Emote::sizeCheck(pPacket));

	auto payload = Emote::convert(pPacket);

	STRING_CHECK(payload->mMessage, EmoteLimits::MAX_MESSAGE);
	String message(payload->mMessage);

	// Notify Zone.
	mZone->onEmote(mCharacter, message);
	return true;
}

const bool ZoneConnection::handleAnimation(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ActorAnimation::sizeCheck(pPacket));
	
	auto payload = ActorAnimation::convert(pPacket);

	// Notify Zone.
	mZone->onAnimationChange(mCharacter, payload->mAnimation, payload->mSpeed, false);
	return true;
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

const bool ZoneConnection::handleWhoRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(WhoRequest::sizeCheck(pPacket));

	auto payload = WhoRequest::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	const String characterName(payload->mCharacterName);

	Log::info(payload->_debug());
	if (payload->mType != 0 && payload->mType != 3) return false; // who or who all.

	WhoFilter filter;
	filter.mType = payload->mType == 0 ? WhoType::Zone : WhoType::All;
	filter.mText = characterName;
	filter.mRace = payload->mRace;
	filter.mClass = payload->mClass;
	filter.mLevelMinimum = payload->mLevelMinimum;
	filter.mLevelMaximum = payload->mLevelMaximum;
	filter.mFlag = payload->mFlag;
	// TODO: GM.

	mZone->handleWhoRequest(mCharacter, filter);
	return true;
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
			payloadSize += i->getGuild()->getName().size() + 3; // +1 for null terminator and + 2 for brackets.
		else
			payloadSize += 1; // null terminator.
	}

	auto packet = new EQApplicationPacket(OP_WhoAllResponse, payloadSize);
	Utility::MemoryWriter writer(packet->pBuffer, payloadSize);

	// Write header.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(startStringID); // String ID.
	writer.writeString(whoLine);
	writer.write<u8>(0x0a); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(endStringID); // String ID.
	writer.write<u32>(0); // Filtered.
	writer.write<u32>(0); // No Active Characters.
	writer.write<u32>(numResults); // Count.
	writer.write<u32>(numResults); // Players.
	writer.write<u32>(numResults); // Number of results.

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
			guildName = "<" + i->getGuild()->getName() + ">";

		writer.write<u32>(formatString); // String ID.
		writer.write<u32>(flags); // Flags.
		writer.write<u32>(0xFFFFFFFF); // Unknown.
		writer.writeString(i->getName()); // Character Name. // TODO: This is going to come up as X's corpse if the Character is dead.
		writer.write<u32>(0xFFFFFFFF); // String ID. (Rank String ID) *GM Impossible* etc.
		writer.writeString(guildName); // Guild Name
		writer.write<u32>(0); // Unknown.
		writer.write<u32>(0xFFFFFFFF); // String ID.
		writer.write<u32>(zoneStringID); // String ID.
		writer.write<u16>(zoneID); // Zone ID. Parameter for zoneStringID String.
		writer.write<u16>(0); // Unknown (Tested, not part of Zone ID.)
		writer.write<u32>(i->getClass()); // Class.
		writer.write<u32>(i->getLevel()); // Level.
		writer.write<u32>(i->getRace()); // Race.
		writer.writeString(accountName); // Account Name // TODO: This does not work.
		writer.write<u32>(0); // Unknown.
	}

	sendPacket(packet);
	delete packet;
	EXPECTED(writer.check());
}

EQApplicationPacket* ZoneConnection::makeChannelMessage(const u32 pChannel, const String& pSenderName, const String& pMessage) {
	u32 payloadSize = 0;
	payloadSize += 36; // Fixed.
	payloadSize += pSenderName.size() + 1;
	payloadSize += pMessage.size() + 1;
	payloadSize += 1; // target null term

	auto packet = new EQApplicationPacket(OP_ChannelMessage, payloadSize);
	Utility::MemoryWriter writer(packet->pBuffer, payloadSize);

	writer.writeString(pSenderName); // Sender Name.
	writer.write<u8>(0); // Target Name.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(0); // Language ID.
	writer.write<u32>(pChannel); // Channel ID.
	writer.write<u32>(0); // Unknown.
	writer.write<u8>(0); // Unknown.
	writer.write<u32>(0); // Language Skill.
	writer.writeString(pMessage); // Message.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<u16>(0); // Unknown.
	writer.write<u8>(0); // Unknown.

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

const bool ZoneConnection::handleGroupInvite(const EQApplicationPacket* pPacket) {
	using namespace Payload::Group;
	if(!pPacket) return false;
	SIZE_CHECK(Invite::sizeCheck(pPacket));

	auto payload = Invite::convert(pPacket);

	STRING_CHECK(payload->mFrom, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mTo, Limits::Character::MAX_NAME_LENGTH);
	const String inviteeName(payload->mTo);
	
	// Notify Zone.
	mZone->onGroupInvite(mCharacter, inviteeName);
	return true;
}

void ZoneConnection::sendGroupInvite(const String pFromCharacterName) {
	using namespace Payload::Group;
	EXPECTED(mConnected);

	auto packet = Invite::construct(pFromCharacterName, mCharacter->getName());
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleGroupAcceptInvite(const EQApplicationPacket* pPacket) {
	using namespace Payload::Group;
	if(!pPacket) return false;
	SIZE_CHECK(AcceptInvite::sizeCheck(pPacket));

	auto payload = AcceptInvite::convert(pPacket);

	STRING_CHECK(payload->mName1, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mName2, Limits::Character::MAX_NAME_LENGTH);

	// Notify Zone.
	mZone->onGroupInviteAccept(mCharacter);
	return true;
}

const bool ZoneConnection::handleGroupDeclineInvite(const EQApplicationPacket* pPacket) {
	using namespace Payload::Group;
	if(!pPacket) return false;
	SIZE_CHECK(DeclineInvite::sizeCheck(pPacket));

	auto payload = DeclineInvite::convert(pPacket);

	STRING_CHECK(payload->mName1, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mName2, Limits::Character::MAX_NAME_LENGTH);

	// Notify Zone.
	mZone->onGroupInviteDecline(mCharacter);
	return true;
}

const bool ZoneConnection::handleGroupDisband(const EQApplicationPacket* pPacket) {
	using namespace Payload::Group;
	if(!pPacket) return false;
	SIZE_CHECK(Disband::sizeCheck(pPacket));
	
	auto payload = Disband::convert(pPacket);

	STRING_CHECK(payload->mName1, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mName2, Limits::Character::MAX_NAME_LENGTH);
	const String removeCharacterName(payload->mName1);

	if (mCharacter->getName() == removeCharacterName) {
		mZone->onGroupLeave(mCharacter);
	}
	else {
		mZone->onGroupRemove(mCharacter, removeCharacterName);
	}
	return true;
}

const bool ZoneConnection::handleGroupMakeLeader(const EQApplicationPacket* pPacket) {
	using namespace Payload::Group;
	if(!pPacket) return false;
	SIZE_CHECK(MakeLeader::sizeCheck(pPacket));

	auto payload = MakeLeader::convert(pPacket);

	STRING_CHECK(payload->mCurrentLeader, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mNewLeader, Limits::Character::MAX_NAME_LENGTH);
	const String newLeader(payload->mNewLeader);

	// Notify Zone.
	mZone->onGroupMakeLeader(mCharacter, newLeader);
	return true;
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

const bool ZoneConnection::handleZoneChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ZoneChange::sizeCheck(pPacket));

	auto payload = ZoneChange::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	
	mZone->handleZoneChange(mCharacter, payload->mZoneID, payload->mInstanceID, Vector3(payload->mX, payload->mY, payload->mZ));
	return true;
}

const bool ZoneConnection::handleGuildCreate(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(Create::sizeCheck(pPacket));
	
	auto payload = Create::convert(pPacket);

	STRING_CHECK(payload->mName, Limits::Guild::MAX_NAME_LENGTH);
	const String guildName(payload->mName);

	// Notify Zone.
	mZone->onGuildCreate(mCharacter, guildName);
	return true;
}

const bool ZoneConnection::handleGuildDelete(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if(!pPacket) return false;
	SIZE_CHECK(Delete::sizeCheck(pPacket));

	auto payload = Delete::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	
	// Notify Zone.
	mZone->onGuildDelete(mCharacter);
	return true;
}

void ZoneConnection::sendGuildRank(const u32 pRank) {
	using namespace Payload::Guild;
	EXPECTED(mConnected);

	auto packet = RankUpdate::construct(pRank, mCharacter->getName());
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendGuildNames() {
	auto packet = Payload::makeGuildNameList(mGuildManager->getGuilds());
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleGuildInvite(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(Invite::sizeCheck(pPacket));

	auto payload = Invite::convert(pPacket);

	STRING_CHECK(payload->mToCharacter, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mFromCharacter, Limits::Character::MAX_NAME_LENGTH);
	const String characterName(payload->mToCharacter);

	// Check: String length.
	if (!Limits::Character::nameLength(characterName)) return false;

	// Handle: Inviting another player to guild.
	if (payload->mAction == CommandAction::Invite) {
		mZone->onGuildInvite(mCharacter, characterName);
	}
	// Handle: Promoting a member within guild.
	else if (payload->mAction == CommandAction::Promote) {
		mZone->onGuildPromote(mCharacter, characterName);
	}
	else {
		// TODO: Log (unknown action sent).
	}

	return true;
}

const bool ZoneConnection::handleGuildRemove(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(Remove::sizeCheck(pPacket));

	auto payload = Remove::convert(pPacket);

	STRING_CHECK(payload->mToCharacter, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mFromCharacter, Limits::Character::MAX_NAME_LENGTH);
	const String toCharacterName(payload->mToCharacter);
	
	// Check: String length.
	if (!Limits::Character::nameLength(toCharacterName)) return false;

	// Handle: Character is removing themself.
	if (mCharacter->getName() == toCharacterName) {
		mZone->onGuildLeave(mCharacter);
	}
	// Handle: Character is removing another Character.
	else {
		mZone->onGuildRemove(mCharacter, toCharacterName);
	}

	return true;
}

void ZoneConnection::sendGuildInvite(const String& pInviterName, const u32 pGuildID) {
	using namespace Payload::Guild;
	EXPECTED(mConnected);

	auto packet = Invite::create();
	auto payload = Invite::convert(packet);
	payload->mGuildID = pGuildID;
	strcpy(payload->mToCharacter, mCharacter->getName().c_str());
	strcpy(payload->mFromCharacter, pInviterName.c_str());

	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleGuildInviteResponse(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(InviteResponse::sizeCheck(pPacket));

	auto payload = InviteResponse::convert(pPacket);

	STRING_CHECK(payload->mInviter, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mNewMember, Limits::Character::MAX_NAME_LENGTH);

	switch (payload->mResponse) {
	case InviteResponseType::Accept:
		mZone->onGuildInviteAccept(mCharacter);
		break;
	case InviteResponseType::Decline:
		mZone->onGuildInviteDecline(mCharacter);
		break;
	default:
		mLog->error("Unknown invite response type: " + toString(payload->mResponse) + " in " + __FUNCTION__);
		break;
	}

	return true;
}

const bool ZoneConnection::handleGuildSetMOTD(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(MOTD::sizeCheck(pPacket));

	auto payload = MOTD::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mSetByName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mMOTD, Limits::Guild::MAX_MOTD_LENGTH);
	const String motd(payload->mMOTD);

	// Notify Zone.
	mZone->onGuildSetMOTD(mCharacter, motd);
	return true;
}

void ZoneConnection::sendGuildMOTDResponse(const String& pMOTD, const String& pMOTDSetByName) {
	using namespace Payload::Guild;
	EXPECTED(mConnected);

	auto packet = MOTDResponse::construct(pMOTDSetByName, pMOTD);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleGuildMOTDRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(MOTDRequest::sizeCheck(pPacket));

	// Notify Zone.
	mZone->onGuildMOTDRequest(mCharacter);
	return true;
}

const bool ZoneConnection::handleSetGuildURLOrChannel(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(GuildUpdate::sizeCheck(pPacket));

	auto payload = GuildUpdate::convert(pPacket);

	STRING_CHECK(payload->mText, 512);
	const String text(payload->mText);

	// Notify Zone.
	switch (payload->mAction) {
	case GuildUpdateAction::URL:
		mZone->onGuildSetURL(mCharacter, text);
		break;
	case GuildUpdateAction::Channel:
		mZone->onGuildSetChannel(mCharacter, text);
		break;
	default:
		mLog->error("Unknown action: " + toString(payload->mAction) + " in " + __FUNCTION__);
		break;
	}

	return true;
}

const bool ZoneConnection::handleSetGuildPublicNote(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(PublicNote::sizeCheck(pPacket));

	auto payload = PublicNote::convert(pPacket);

	STRING_CHECK(payload->mSenderName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mTargetName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mNote, Limits::Guild::MAX_PUBLIC_NOTE_LENGTH);
	const String targetName(payload->mTargetName);
	const String note(payload->mNote);

	// Notify Zone.
	mZone->onGuildSetPublicNote(mCharacter, targetName, note);
	return true;
}

const bool ZoneConnection::handleGetGuildStatus(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(StatusRequest::sizeCheck(pPacket));

	auto payload = StatusRequest::convert(pPacket);

	STRING_CHECK(payload->mName, Limits::Character::MAX_NAME_LENGTH);
	String targetName(payload->mName);
	
	// Notify Zone.
	mCharacter->notify("Feature not implemented."); // This feature is lame.
	return true;
}

const bool ZoneConnection::handleGuildDemote(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(Demote::sizeCheck(pPacket));

	auto payload = Demote::convert(pPacket->pBuffer);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mDemoteName, Limits::Character::MAX_NAME_LENGTH);
	const String demoteeName(payload->mDemoteName);

	// Check: String length.
	if (!Limits::Character::nameLength(demoteeName)) return false;

	// Notify Zone.
	mZone->onGuildDemote(mCharacter, demoteeName);
	return true;
}

const bool ZoneConnection::handleGuildSetFlags(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(FlagsUpdate::sizeCheck(pPacket));

	auto payload = FlagsUpdate::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mOtherName, Limits::Character::MAX_NAME_LENGTH);
	const String otherName(payload->mOtherName);

	// Notify Zone.
	mZone->onGuildSetFlags(mCharacter, otherName, payload->mFlags);
	return true;
}

const bool ZoneConnection::handleGuildMakeLeader(const EQApplicationPacket* pPacket) {
	using namespace Payload::Guild;
	if (!pPacket) return false;
	SIZE_CHECK(MakeLeader::sizeCheck(pPacket));

	auto payload = MakeLeader::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mLeaderName, Limits::Character::MAX_NAME_LENGTH);
	const String leaderName(payload->mLeaderName);

	// Check: String length.
	if (!Limits::Character::nameLength(leaderName)) return false;

	// Notify Zone.
	mZone->onGuildMakeLeader(mCharacter, leaderName);
	return true;
}

void ZoneConnection::_unimplementedFeature(String pOpCodeName)
{
}

const bool ZoneConnection::handleFaceChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(FaceChange::sizeCheck(pPacket));

	auto payload = FaceChange::convert(pPacket);
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
	return true;
}

void ZoneConnection::sendWearChange(const uint16 pSpawnID, const uint32 pMaterialID, const uint32 pEliteMaterialID, const uint32 pColour, const uint8 pSlotID) {
	EXPECTED(mConnected);
	using namespace Payload::Zone;

	auto packet = WearChange::construct(pSpawnID, pMaterialID, pEliteMaterialID, pColour, pSlotID);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleAutoAttack(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(AutoAttack::sizeCheck(pPacket));

	auto payload = AutoAttack::convert(pPacket);
	mCharacter->setAutoAttack(payload->mAttacking);
	return true;
}

const bool ZoneConnection::handleMemoriseSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(MemoriseSpell::sizeCheck(pPacket));

	auto payload = MemoriseSpell::convert(pPacket);

	switch (payload->mAction){
		// Character is scribing a spell into the SpellBook.
	case MemoriseSpellAction::Scribe:
		mCharacter->handleScribeSpell(payload->mSlot, payload->mSpellID);
		break;
		// Character is adding a spell to the SpellBar.
	case MemoriseSpellAction::Memorise:
		mCharacter->handleMemoriseSpell(payload->mSlot, payload->mSpellID);
		break;
		// Character is removing a spell from the SpellBar.
	case MemoriseSpellAction::Unmemorise:
		mCharacter->handleUnmemoriseSpell(payload->mSlot);
		break;
	default:
		mLog->error("Unknown action: " + toString(payload->mAction) + " in" + String(__FUNCTION__));
		break;
	}

	return true;
}

const bool ZoneConnection::handleDeleteSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(DeleteSpell::sizeCheck(pPacket));

	auto payload = DeleteSpell::convert(pPacket);
	const bool success = mCharacter->handleDeleteSpell(payload->mSlot);
	sendDeleteSpellDelete(payload->mSlot, success);

	return true;
}

const bool ZoneConnection::handleLoadSpellSet(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(LoadSpellSet::sizeCheck(pPacket));

	auto payload = LoadSpellSet::convert(pPacket);

	// TODO:
	return true;
}

const bool ZoneConnection::handleSwapSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(SwapSpell::sizeCheck(pPacket));

	auto payload = SwapSpell::convert(pPacket);
	mCharacter->handleSwapSpells(payload->mFrom, payload->mTo);

	// Client requires a reply.
	sendPacket(pPacket);
	return true;
}

const bool ZoneConnection::handleCastSpell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(CastSpell::sizeCheck(pPacket));
	
	auto payload = CastSpell::convert(pPacket);

	if (payload->mInventorySlot == 0xFFFF)
		EXPECTED_BOOL(mCharacter->isCaster());

	// Check: Target validity
	if (mCharacter->hasTarget()) {
		// Match spawn IDs
		EXPECTED_BOOL(payload->mTargetID == mCharacter->getTarget()->getSpawnID());
	}
	else {
		// Expect both zero.
		EXPECTED_BOOL(payload->mTargetID == 0);
	}

	EXPECTED_BOOL(Limits::SpellBar::slotValid(payload->mSlot));
	EXPECTED_BOOL(Limits::SpellBar::spellIDValid(payload->mSpellID));

	// Casting from Spell Bar.
	mZone->handleCastingBegin(mCharacter, payload->mSlot, payload->mSpellID);
	return true;
}

const bool ZoneConnection::handleCombatAbility(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(CombatAbility::sizeCheck(pPacket));

	auto payload = CombatAbility::convert(pPacket);

	// TODO:
	return true;
}

const bool ZoneConnection::handleTaunt(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Taunt::sizeCheck(pPacket));

	auto payload = Taunt::convert(pPacket);
	// TODO:
	return true;
}

const bool ZoneConnection::handleConsider(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Consider::sizeCheck(pPacket));
	
	auto payload = Consider::convert(pPacket);
	Log::info(payload->_debug());
	mZone->handleConsider(mCharacter, payload->mTargetSpawnID);
	return true;
}

const bool ZoneConnection::handleConsiderCorpse(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Consider::sizeCheck(pPacket));

	auto payload = Consider::convert(pPacket);
	Log::info(payload->_debug());
	mZone->handleConsiderCorpse(mCharacter, payload->mTargetSpawnID);
	return true;
}

const bool ZoneConnection::handleSurname(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Surname::sizeCheck(pPacket));

	auto payload = Surname::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mLastName, Limits::Character::MAX_LAST_NAME_LENGTH);
	const String lastName(payload->mLastName);

	if (!Limits::Character::surnameLengthClient(lastName)) return false;

	// TODO: Check for special characters / Captialisation.

	// Update Character.
	mCharacter->setLastName(lastName);
	sendSurnameApproval(true);
	// Update Zone.
	mZone->handleSurnameChange(mCharacter);
	return true;
}

void ZoneConnection::sendSurnameApproval(const bool pSuccess) {
	// NOTE: This packet notifies the client that their surname was approved.
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = Surname::construct(pSuccess ? 1 : 0, mCharacter->getName(), mCharacter->getLastName());
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleGMLastName(const EQApplicationPacket* pPacket) {
	mCharacter->notify("Please use the command system.");
	return true;
}

const bool ZoneConnection::handleClearSurname(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Update Character.
	mCharacter->setLastName("");
	// Update Zone.
	mZone->handleSurnameChange(mCharacter);
	return true;
}

const bool ZoneConnection::handleSetTitle(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(SetTitle::sizeCheck(pPacket));
	
	auto payload = SetTitle::convert(pPacket);

	// Handle: Title change.
	if (payload->mAction == SetTitleAction::Title) {
		mZone->onSetTitle(mCharacter, payload->mID);
	}
	// Handle: Suffix change.
	else if (payload->mAction == SetTitleAction::Suffix) {
		mZone->onSetSuffix(mCharacter, payload->mID);
	}
	else {
		mLog->error("Unknown action in " + String(__FUNCTION__));
	}

	return true;
}

const bool ZoneConnection::handleRequestTitles(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Notify Zone.
	mZone->onRequestTitles(mCharacter);
	return true;
}

void ZoneConnection::sendDeleteSpellDelete(const uint16 pSlot, const bool pSuccess) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = DeleteSpell::construct(pSlot, pSuccess ? 1 : 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::_sendMemoriseSpell(const u16 pSlot, const u32 pSpellID, const u32 pAction) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = MemoriseSpell::construct(pSlot, pSpellID, pAction);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendScribeSpell(const u16 pSlot, const u32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpellAction::Scribe);
}

void ZoneConnection::sendMemoriseSpell(const uint16 pSlot, const uint32 pSpellID) {
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpellAction::Memorise);
}

void ZoneConnection::sendUnmemoriseSpell(const uint16 pSlot) {
	_sendMemoriseSpell(pSlot, 0, Payload::Zone::MemoriseSpellAction::Unmemorise);
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
	_sendMemoriseSpell(pSlot, pSpellID, Payload::Zone::MemoriseSpellAction::SpellBarRefresh);
}

void ZoneConnection::sendEnableSpellBar(const uint32 pSpellID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ManaChange::construct(mCharacter->getCurrentMana(), mCharacter->getCurrentEndurance(), pSpellID);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendSkillValue(const u32 pSkillID, const u32 pValue) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = SkillUpdate::construct(pSkillID, pValue);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleBeginLootRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(LootBeginRequest::sizeCheck(pPacket));

	auto payload = LootBeginRequest::convert(pPacket);

	// Notify Zone.
	mZone->handleBeginLootRequest(mCharacter, payload->mSpawnID);
	return true;
}

const bool ZoneConnection::handleEndLootRequest(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Notify Zone.
	mZone->handleEndLootRequest(mCharacter);
	return true;
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

const bool ZoneConnection::handleLootItem(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(LootItem::sizeCheck(pPacket));
	
	auto payload = LootItem::convert(pPacket);
	Log::info(payload->_debug());

	// Send required reply(echo).
	sendPacket(const_cast<EQApplicationPacket*>(pPacket));

	// Notify Zone.
	mZone->handleLootItem(mCharacter, mCharacter->getLootingCorpse(), payload->mSlotID);
	return true;
}

const bool ZoneConnection::handleMoveItem(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(MoveItem::sizeCheck(pPacket));

	auto payload = MoveItem::convert(pPacket);

	// Every Item move attempt gets logged.
	StringStream ss;
	ss << "[MoveItem] Slot: " << payload->mFromSlot << " to " << payload->mToSlot << " | Stacks: " << payload->mStacks;
	mLog->info(ss.str());

	// Notify Zone.
	mZone->onMoveItem(mCharacter, payload->mFromSlot, payload->mToSlot, payload->mStacks);
	return true;
}

const bool ZoneConnection::handleConsume(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Consume::sizeCheck(pPacket));

	auto payload = Consume::convert(pPacket);

	Log::info("Consume from slot: " + std::to_string(payload->mSlot));
	if (!mCharacter->getInventory()->consume(payload->mSlot, 1)) {
		inventoryError();
	}
	
	sendStamina(0, 0);
	return true;
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

const bool ZoneConnection::handlePotionBelt(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(PotionBelt::sizeCheck(pPacket));

	auto payload = PotionBelt::convert(pPacket);
	// TODO:
	return true;
}

const bool ZoneConnection::handleItemRightClick(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ItemRightClick::sizeCheck(pPacket));

	auto payload = ItemRightClick::convert(pPacket);

	sendItemRightClickResponse(payload->mSlot, payload->mTargetSpawnID);
	return true;
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

const bool ZoneConnection::handleOpenContainer(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(OpenContainer::sizeCheck(pPacket));

	auto payload = OpenContainer::convert(pPacket);
	Log::info("Open Container: " + std::to_string(payload->mSlot));
	return true;
}

const bool ZoneConnection::handleTradeRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Payload::Zone::TradeRequest::sizeCheck(pPacket));

	auto payload = Payload::Zone::TradeRequest::convert(pPacket);

	// OP_TradeRequestAck

	// NOTE: Underfoot does appear to require a response.

	// Notify Zone.
	mZone->onTradeRequest(mCharacter, payload->mToSpawnID);
	return true;
}

const bool ZoneConnection::handleTradeRequestAck(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Payload::Zone::TradeRequest::sizeCheck(pPacket));

	auto payload = Payload::Zone::TradeRequest::convert(pPacket);

	// Notify Zone.
	mZone->onTradeResponse(mCharacter, payload->mToSpawnID);
	return true;
}

void ZoneConnection::sendTradeRequest(const uint32 pFromSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	Payload::Zone::TradeRequest payload;
	payload.mToSpawnID = mCharacter->getSpawnID();
	payload.mFromSpawnID = pFromSpawnID;

	auto packet = Payload::Zone::TradeRequest::create(payload);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleCancelTrade(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(TradeCancel::sizeCheck(pPacket));
	
	// NOTE: Underfoot sends this twice when trade is canceled with an NPC
	// TODO: Test whether this occurs when canceling with PC.
	if (mCharacter->isTrading() == false) {
		Log::info("Got OP_CancelTrade while not trading. Ignoring.");
		return true;
	}

	auto payload = TradeCancel::convert(pPacket);

	Log::info(payload->_debug());

	// Notify Zone.
	mZone->onTradeCancel(mCharacter, payload->mToSpawnID);
	return true;
}

const bool ZoneConnection::handleAcceptTrade(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(TradeAccept::sizeCheck(pPacket));

	auto payload = TradeAccept::convert(pPacket);
	Log::info(payload->_debug());

	// Notify Zone.
	mZone->onTradeAccept(mCharacter, payload->mFromSpawnID);
	return true;
}

const bool ZoneConnection::handleTradeBusy(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(TradeBusy::sizeCheck(pPacket));

	auto payload = TradeBusy::convert(pPacket);

	// Notify Zone.
	mZone->onTradeBusy(mCharacter, payload->mToSpawnID);
	return true;
}

const bool ZoneConnection::handleSetServerFiler(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ServerFilter::sizeCheck(pPacket));

	auto payload = ServerFilter::convert(pPacket);

	mCharacter->setFilters(payload->mFilters);
	return true;
}

const bool ZoneConnection::handleItemLinkClick(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ItemLink::sizeCheck(pPacket));

	auto payload = ItemLink::convert(pPacket);

	// Retrieve base ItemData
	auto itemData = ServiceLocator::getItemDataStore()->get(payload->mItemID);
	EXPECTED_BOOL(itemData);

	Item* item = new Item(itemData);

	item->setEvolvingLevel(payload->mCurrentEvolvingLevel);
	item->setOrnamentationIcon(payload->mOrnamentationIcon);
	
	// NOTE: Untested!
	// Add required augmentations.
	for (auto i = 0; i < 5; i++) {
		if (payload->mAugments[i] != 0) {
			// Retrieve augmentation ItemData.
			auto augmentationItemData = ServiceLocator::getItemDataStore()->get(payload->mAugments[i]);
			EXPECTED_BOOL(augmentationItemData);

			// NOTE: Memory is freed when item is deleted.
			item->setAugmentation(i, new Item(augmentationItemData));
		}
	}

	sendItemView(item);
	delete item;
	return true;
}

const bool ZoneConnection::handleItemView(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	Log::info("Got OP_ItemViewUnknown. Size=" + std::to_string(pPacket->size));
	return true;
}

const bool ZoneConnection::handleMoveCurrency(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(MoveCurrency::sizeCheck(pPacket));

	auto payload = MoveCurrency::convert(pPacket);

	// Every currency move attempt gets logged.
	StringStream ss;
	ss << "[MoveCurrency] Slot: " << payload->mFromSlot << " to " << payload->mToSlot << " | Type: " << payload->mFromType << " to " << payload->mToType << " | Amount: " << payload->mAmount;
	mLog->info(ss.str());

	// Notify Zone.
	if (!mZone->onMoveCurrency(mCharacter, payload->mFromSlot, payload->mToSlot, payload->mFromType, payload->mToType, payload->mAmount)){
		inventoryError();
		return false;
	}

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

const bool ZoneConnection::handleCrystalCreate(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(CrystalCreate::sizeCheck(pPacket));

	// NOTE: Underfoot will allow crystals to be summoned to the cursor even when there are Items on it.
	// This is to keep things simple.
	if (mCharacter->getInventory()->isCursorEmpty() == false) {
		mCharacter->notify("Please clear your cursor and try again.");
		return true;
	}

	auto payload = CrystalCreate::convert(pPacket);
	uint32 itemID = 0;

	uint32 stacks = payload->mAmount;
	String crystalName;

	// Creating Radiant Crystals.
	if (payload->mType == CrystalCreateType::Radiant) {
		itemID = ItemID::RadiantCrystal;
		crystalName = "Radiant Crystals.";
		// Adjust stacks to prevent over-stacking.
		stacks = stacks > MaxRadiantCrystalsStacks ? MaxRadiantCrystalsStacks : stacks;
		// Check: Not trying to create more than is possible.
		EXPECTED_BOOL(stacks <= mCharacter->getInventory()->getRadiantCrystals());
		// Remove crystals from Character.
		EXPECTED_BOOL(mCharacter->getInventory()->removeRadiantCrystals(stacks));
	}
	// Creating Ebon Crystals.
	else if (payload->mType == CrystalCreateType::Ebon) {
		itemID = ItemID::EbonCrystal;
		crystalName = "Ebon Crystals.";
		// Adjust stacks to prevent over-stacking.
		stacks = stacks > MaxEbonCrystalsStacks ? MaxEbonCrystalsStacks : stacks;
		// Check: Not trying to create more than is possible.
		EXPECTED_BOOL(stacks <= mCharacter->getInventory()->getEbonCrystals());
		// Remove crystals from Character.
		EXPECTED_BOOL(mCharacter->getInventory()->removeEbonCrystals(stacks));
		
	}
	// Unknown.
	else {
		mLog->error("Unknown crystal type: " + toString(payload->mType) + " in " + String(__FUNCTION__));
		return true;
	}

	auto item = ServiceLocator::getItemFactory()->make(itemID, stacks);
	EXPECTED_BOOL(item);
	mCharacter->getInventory()->pushCursor(item);
	sendItemSummon(item);

	// Notify user.
	mCharacter->notify("You have created (" + std::to_string(stacks) + ") " + crystalName);

	// Update client.
	sendCrystals();
	return true;
}

const bool ZoneConnection::handleCrystalReclaim(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Check: Character has Radiant or Ebon crystals on cursor.
	Item* item = mCharacter->getInventory()->peekCursor();
	if (!item || (item->getID() != ItemID::RadiantCrystal && item->getID() != ItemID::EbonCrystal)) {
		mCharacter->notify("Please place crystals on your cursor and try again.");
		return true;
	}

	const uint32 stacks = item->getStacks();
	String crystalName;

	// Add Radiant Crystals.
	if (item->getID() == ItemID::RadiantCrystal) {
		crystalName = "Radiant Crystals.";
		// Consume from Inventory.
		EXPECTED_BOOL(mCharacter->getInventory()->consume(SlotID::CURSOR, stacks));
		mCharacter->getInventory()->addRadiantCrystals(stacks);
	}
	// Add Ebon Crystals.
	else if (item->getID() == ItemID::EbonCrystal) {
		crystalName = "Ebon Crystals.";
		// Consume from Inventory.
		EXPECTED_BOOL(mCharacter->getInventory()->consume(SlotID::CURSOR, stacks));
		mCharacter->getInventory()->addEbonCrystals(stacks);
	}

	// Clear cursor.
	sendMoveItem(SlotID::CURSOR, SlotID::SLOT_DELETE);

	// Notify user.
	mCharacter->notify("You have reclaimed (" + std::to_string(stacks) + ") " + crystalName);

	// Update client.
	sendCrystals();
	return true;
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

const bool ZoneConnection::handleUnknown(const EQApplicationPacket* pPacket) {
	if(!pPacket) return false;
	Log::info("Unknown Packet, size=" + std::to_string(pPacket->size));
	auto raw = static_cast<EQRawApplicationPacket*>(const_cast<EQApplicationPacket*>(pPacket));
	Log::info("OpCode= " + std::to_string(raw->GetRawOpcode()));
	return true;
}

const bool ZoneConnection::handleEnvironmentDamage(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(EnvironmentDamage::sizeCheck(pPacket));

	// TODO:
	sendHealthUpdate();
	return true;
}

void ZoneConnection::sendPopup(const String& pTitle, const String& pText) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = PopupWindow::construct(pTitle, pText);
	sendPacket(packet);
	safe_delete(packet);
}

const bool ZoneConnection::handlePopupResponse(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(PopupResponse::sizeCheck(pPacket));

	auto payload = PopupResponse::convert(pPacket);
	// TODO:
	return true;
}

const bool ZoneConnection::handleClaimRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(ClaimRequest::sizeCheck(pPacket));

	auto payload = ClaimRequest::convert(pPacket);

	STRING_CHECK(payload->mName, Limits::Character::MAX_NAME_LENGTH);

	// TODO:
	return true;
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

void ZoneConnection::sendItemTradeCharacter(Item* pItem) {
	if (!pItem) return;

	u32 size = 0;
	const unsigned char* data = pItem->copyData(size, Payload::ItemPacketTradeView);

	auto packet = new EQApplicationPacket(OP_ItemPacket, data, size);
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


const bool ZoneConnection::handleAugmentItem(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(AugmentItem::sizeCheck(pPacket));

	auto payload = AugmentItem::convert(pPacket);

	Log::info(payload->_debug());

	auto container = mCharacter->getInventory()->getItem(payload->mContainerSlot);
	EXPECTED_BOOL(container);
	EXPECTED_BOOL(container->getContainerType() == ContainerType::AugmentationSealer);
	EXPECTED_BOOL(container->getContainerSlots() == 2);

	// Insert
	if (payload->mAugmentSlot == -1) {
		auto item0 = container->getContents(0);
		EXPECTED_BOOL(item0);
		auto item1 = container->getContents(1);
		EXPECTED_BOOL(item1);

		Item* item = nullptr;
		Item* augment = nullptr;

		// Determine which item is the augment.

		if (item0->getItemType() == ItemType::Augmentation) {
			augment = item0;
			item = item1;
		}
		else {
			EXPECTED_BOOL(item1->getItemType() == ItemType::Augmentation);
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
		EXPECTED_BOOL(item->insertAugment(augment));
		mCharacter->getInventory()->pushCursor(item);

		uint32 payloadSize = 0;
		const unsigned char* data = item->copyData(payloadSize, Payload::ItemPacketSummonItem);

		auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
		sendPacket(packet);
		delete packet;

		return true;
	}

	// Remove.
	if (payload->mAugmentSlot == 0) {

	}

	return true;
}

void ZoneConnection::sendDeleteItem(const u32 pFromSlot, const u32 pToSlot, const u32 pStacks) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = DeleteItem::construct(pFromSlot, pToSlot, pStacks);
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

void ZoneConnection::sendTradeRequestAcknowledge(const u32 pFromSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = TradeRequestAcknowledge::construct(mCharacter->getSpawnID(), pFromSpawnID);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendTradeFinished() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishTrade, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendTradeCancel(const u32 pToSpawnID, const u32 pFromSpawnID) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = TradeCancel::construct(pToSpawnID, pFromSpawnID);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendCharacterTradeClose() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishWindow, 0);
	sendPacket(packet);
	delete packet;
}

void ZoneConnection::sendFinishWindow2() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_FinishWindow2, 0);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleAugmentInfo(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(AugmentInformation::sizeCheck(pPacket));

	auto payload = AugmentInformation::convert(pPacket);
	Log::info(payload->_debug());

	//sendReadBook(payload->mWindow, 0, 2, "You must use a <c \"#FF0000\">monster cock</c> to remove this augment safely.");
	return true;
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
	Utility::MemoryWriter writer(data, size);

	writer.write<uint32>(pWindow);
	writer.write<uint32>(pSlot);
	writer.write<uint32>(pType);
	writer.write<uint32>(0); // unknown.
	writer.write<uint16>(0); // unknown.
	writer.writeString(pText);

	EXPECTED(writer.check());

	auto packet = new EQApplicationPacket(OP_ReadBook, data, size);
	sendPacket(packet);
	delete packet;

	
}

const bool ZoneConnection::handleReadBook(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(BookRequest::sizeCheck(pPacket));

	auto payload = BookRequest::convert(pPacket);

	STRING_CHECK(payload->mText, 8194);
	const String text(payload->mText);

	Log::info(payload->_debug());

	// Extract Item ID and Instance ID from text.
	uint32 itemID = 0;
	uint32 itemInstanceID = 0;

	
	std::vector<String> elements = Utility::split(text, '|');
	EXPECTED_BOOL(elements.size() == 2);

	EXPECTED_BOOL(Utility::stoSafe(itemID, elements[0]));
	EXPECTED_BOOL(Utility::stoSafe(itemInstanceID, elements[1]));

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
	return true;
}

const bool ZoneConnection::handleCombine(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(Combine::sizeCheck(pPacket));

	auto payload = Combine::convert(pPacket);
	Log::info(payload->_debug());

	// Notify Zone.
	mZone->onCombine(mCharacter, payload->mContainerSlotID);
	return true;
}

void ZoneConnection::sendCombineReply() {
	EXPECTED(mConnected);
	auto packet = new EQApplicationPacket(OP_TradeSkillCombine, 0);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleShopRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ShopRequest::sizeCheck(pPacket));

	auto payload = ShopRequest::convert(pPacket);
	Log::info(payload->_debug());

	// Notify Zone.
	mZone->handleShopRequest(mCharacter, payload->mNPCSpawnID);
	return true;
}

void ZoneConnection::sendShopRequestReply(const uint32 pNPCSpawnID, const uint32 pAction, const float pRate) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ShopRequest::construct(pNPCSpawnID, mCharacter->getSpawnID(), pAction, pRate);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleShopEnd(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ShopEnd::sizeCheck(pPacket));

	auto payload = ShopEnd::convert(pPacket);

	// Notify Zone.
	mZone->handleShopEnd(mCharacter, payload->mNPCSpawnID);
	return true;
}

void ZoneConnection::sendShopEndReply() {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_ShopEndConfirm, 0);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleShopSell(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ShopSell::sizeCheck(pPacket));

	auto payload = ShopSell::convert(pPacket);
	Log::info(payload->_debug());

	// Notify Zone.
	mZone->onSellItem(mCharacter, payload->mNPCSpawnID, payload->mSlotID, payload->mStacks);
	return true;
}

void ZoneConnection::sendShopSellReply(const uint32 pSpawnID, const uint32 pSlotID, const uint32 pStacks, const uint32 pPrice) {
	using namespace Payload::Zone;
	EXPECTED(mConnected);

	auto packet = ShopSell::construct(pSpawnID, pSlotID, pStacks, pPrice);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleShopBuy(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(ShopBuy::sizeCheck(pPacket));

	auto payload = ShopBuy::convert(pPacket);
	Log::info(payload->_debug());

	// Notify Zone.
	mZone->onBuyItem(mCharacter, payload->mNPCSpawnID, payload->mItemInstanceID, payload->mStacks);
	return true;
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
	Utility::MemoryWriter writer(data, size);

	writer.write<PopulateAlternateCurrencies>(populate);

	for (auto i : currencies) {
		PopulateEntry entry;
		entry.mCurrencyID = entry.mCurrencyID2 = i->mCurrencyID;
		entry.mIcon = i->mIcon;
		entry.mItemID = i->mItemID;
		entry.mMaxStacks = i->mMaxStacks;
		writer.write<PopulateEntry>(entry);
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

const bool ZoneConnection::handleAlternateCurrencyReclaim(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(AlternateCurrencyReclaim::sizeCheck(pPacket));

	auto payload = AlternateCurrencyReclaim::convert(pPacket);
	Log::info(payload->_debug());

	// Handle: Create.
	if (payload->mAction == AlternateCurrencyReclaim::Create) {
		// NOTE: Underfoot will allow currencies to be summoned to the cursor even when there are Items on it.
		if (mCharacter->getInventory()->isCursorEmpty() == false) {
			mCharacter->notify("Please clear your cursor and try again.");
			return true;
		}

		// Check: Currency Type.
		const uint32 itemID = ServiceLocator::getAlternateCurrencyManager()->getItemID(payload->mCurrencyID);
		if (itemID == 0) {
			// TODO: Logging.
			return true;
		}

		// Check: Stacks.
		const uint32 currentQuantity = mCharacter->getInventory()->getAlternateCurrencyQuantity(payload->mCurrencyID);
		if (currentQuantity < payload->mStacks) {
			// TODO: Logging.
			return true;
		}

		// Update alternate currency.
		mCharacter->getInventory()->removeAlternateCurrency(payload->mCurrencyID, payload->mStacks);

		// Create Item
		auto item = ServiceLocator::getItemFactory()->make(itemID, payload->mStacks);
		EXPECTED_BOOL(item);

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
			return true;
		}

		// Check: Item on cursor is alternate currency Item.
		const uint32 currencyID = ServiceLocator::getAlternateCurrencyManager()->getCurrencyID(item->getID());
		if (currencyID == 0) {
			mCharacter->notify("Please place an alternate currency on your cursor and try again.");
			return true;
		}

		const uint32 stacks = item->getStacks();
		String currencyName = item->getName();

		// Update alternate currency.
		mCharacter->getInventory()->addAlternateCurrency(currencyID, stacks);
		
		// Consume from Inventory.
		EXPECTED_BOOL(mCharacter->getInventory()->consume(SlotID::CURSOR, stacks));

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
		return true;
	}

	return true;
}

void ZoneConnection::sendMOTD(const String& pMOTD) {
	EXPECTED(mConnected);

	auto packet = new EQApplicationPacket(OP_MOTD, reinterpret_cast<const unsigned char*>(pMOTD.c_str()), pMOTD.size() + 1);
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleRandomRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(RandomRequest::sizeCheck(pPacket));

	auto payload = RandomRequest::convert(pPacket);

	// Notify Zone.
	mZone->handleRandomRequest(mCharacter, payload->mLow, payload->mHigh);
	return true;
}

const bool ZoneConnection::handleDropItem(const EQApplicationPacket* pPacket) {
	if (!pPacket) return false;
	SIZE_CHECK(pPacket->size == 0);

	// Notify Zone.
	mZone->handleDropItem(mCharacter);
	return true;
}

void ZoneConnection::sendObject(Object* pObject) {
	EXPECTED(mConnected);
	EXPECTED(pObject);

	u32 payloadSize = 0;
	payloadSize += pObject->getAsset().length() + 1 + 57;

	char* data = new char[payloadSize];

	Utility::MemoryWriter writer(data, payloadSize);

	writer.write<u32>(1); // Drop ID.
	writer.writeString(pObject->getAsset());
	writer.write<u16>(mZone->getID());
	writer.write<u16>(mZone->getInstanceID());
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<float>(pObject->getHeading());
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<float>(pObject->getSize());
	writer.write<float>(pObject->getPosition().y);
	writer.write<float>(pObject->getPosition().x);
	writer.write<float>(pObject->getPosition().z);
	writer.write<u32>(pObject->getType());
	writer.write<u32>(0xFFFFFFFF); // Unknown.
	writer.write<u32>(0); // Unknown.
	writer.write<u8>(0); // Unknown.

	if (writer.check() == false) {
		Log::error("[ObjectSpawn] Bad Write: Written: " + std::to_string(writer.getBytesWritten()) + " Size: " + std::to_string(writer.getSize()));
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

const bool ZoneConnection::handleXTargetAutoAddHaters(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone::ExtendedTarget;
	if(!pPacket) return false;
	SIZE_CHECK(AutoAddHaters::sizeCheck(pPacket));

	auto payload = AutoAddHaters::convert(pPacket);
	mCharacter->getXTargetController()->setAutoAddHaters(payload->mAction == 1 ? true : false);
	return true;
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
	Utility::MemoryWriter writer(packet->pBuffer, payloadSize);

	// Write header.
	writer.write<u32>(0); // Default selection.
	writer.write<u32>(30000); // Timer.
	writer.write<u32>(0); // Unknown.
	writer.write<u32>(respawnOptions->getNumOptions());

	// Write options.
	for (auto i : options) {
		writer.write<u32>(i.mID);
		writer.write<u16>(i.mZoneID);
		writer.write<u16>(i.mInstanceID);
		writer.write<float>(i.mPosition.x);
		writer.write<float>(i.mPosition.y);
		writer.write<float>(i.mPosition.z);
		writer.write<float>(i.mHeading);
		writer.writeString(i.mName);
		writer.write<u8>(i.mType);
	}

	sendPacket(packet);
	delete packet;
	EXPECTED(writer.check());
}

const bool ZoneConnection::handleRespawnWindowSelect(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(RespawnWindowSelect::sizeCheck(pPacket));

	auto payload = RespawnWindowSelect::convert(pPacket);
	Log::info(payload->_debug());

	// Notify Zone.
	mZone->handleRespawnSelection(mCharacter, payload->mSelection);
	return true;
}

const bool ZoneConnection::handleAAAction(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(AAAction::sizeCheck(pPacket));

	auto payload = AAAction::convert(pPacket);
	Log::info(payload->_debug());

	if (!Utility::inRange<u32>(payload->mExperienceToAA, 0, 100)) return false; // Sanity.

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
	else {
		mLog->error("Unknown action: " + toString(payload->mAction) + " in " + __FUNCTION__);
	}

	return true;
}

const bool ZoneConnection::handleLeadershipExperienceToggle(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if(!pPacket) return false;
	SIZE_CHECK(LeadershipExperienceToggle::sizeCheck(pPacket));

	auto payload = LeadershipExperienceToggle::convert(pPacket);
	auto controller = mCharacter->getExperienceController();

	// Turning leadership experience on.
	if (payload->mValue == 1) {
		controller->setLeadershipExperience(true);
		sendSimpleMessage(MessageType::Experience, StringID::LeadershipOn);
	}
	// Turning leadership experience off.
	else if (payload->mValue == 1) {
		controller->setLeadershipExperience(false);
		sendSimpleMessage(MessageType::Experience, StringID::LeadershipOff);
	}
	// Unknown.
	else {
		mLog->error("Unknown value: " + toString(payload->mValue) + " in " + String(__FUNCTION__));
		return true;
	}

	return true;
}

const bool ZoneConnection::handlePetCommand(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(PetCommand::sizeCheck(pPacket));

	auto payload = PetCommand::convert(pPacket);

	// Notify Zone.
	mZone->onPetCommand(mCharacter, payload->mCommand);
	return true;
}

const bool ZoneConnection::handleSaveRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(SaveRequest::sizeCheck(pPacket));

	return true;
}

const bool ZoneConnection::handleApplyPoison(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(ApplyPoison::sizeCheck(pPacket));

	return true;
}

const bool ZoneConnection::handleRaidInvite(const EQApplicationPacket* pPacket) {
	using namespace Payload::Raid;
	if (!pPacket) return false;
	SIZE_CHECK(Invite::sizeCheck(pPacket));

	auto payload = Invite::convert(pPacket);

	STRING_CHECK(payload->mCharacterName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mLeaderName, Limits::Character::MAX_NAME_LENGTH);

	// TODO:
	return true;
}

const bool ZoneConnection::handleFindPersonRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(FindPersonRequest::sizeCheck(pPacket));

	// TODO:
	return true;
}

const bool ZoneConnection::handleInspectRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(InspectRequest::sizeCheck(pPacket));

	// TODO:
	return true;
}

const bool ZoneConnection::handleSetInspectMessage(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(InspectMessage::sizeCheck(pPacket));

	auto payload = InspectMessage::convert(pPacket);

	STRING_CHECK(payload->mMessage, 256);
	const String newMessage(payload->mMessage);

	// TODO:
	return true;
}

const bool ZoneConnection::handleRemoveBuffRequest(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(RemoveBuffRequest::sizeCheck(pPacket));

	auto payload = RemoveBuffRequest::convert(pPacket);

	// TODO:
	return true;
}

const bool ZoneConnection::handleGroupRoleChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Group;
	if (!pPacket) return false;
	SIZE_CHECK(Roles::sizeCheck(pPacket));

	auto payload = Roles::convert(pPacket);

	STRING_CHECK(payload->mTargetName, Limits::Character::MAX_NAME_LENGTH);
	STRING_CHECK(payload->mSetterName, Limits::Character::MAX_NAME_LENGTH);
	String targetName(payload->mTargetName);

	// Notify Zone.
	mZone->onGroupRoleChange(mCharacter, targetName, payload->mRoleID, payload->mToggle);
	return true;
}

void ZoneConnection::sendRejectTarget() {
	using namespace Payload::Zone;

	auto packet = RejectTarget::construct();
	sendPacket(packet);
	delete packet;
}

const bool ZoneConnection::handleWearChange(const EQApplicationPacket* pPacket) {
	using namespace Payload::Zone;
	if (!pPacket) return false;
	SIZE_CHECK(WearChange::sizeCheck(pPacket));

	auto payload = WearChange::convert(pPacket);
	mLog->info(payload->_debug());

	// Notify Zone.
	mZone->onWearChange(mCharacter, payload->mMaterialID, payload->mEliteMaterialID, payload->mColour, payload->mSlot);
	return true;
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
