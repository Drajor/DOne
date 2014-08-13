#include "ZoneClientConnection.h"
#include "GuildManager.h"
#include "GroupManager.h"
#include "RaidManager.h"
#include "Zone.h"
#include "ZoneData.h"
#include "Character.h"
#include "LogSystem.h"
#include "DataStore.h"
#include "CommandHandler.h"
#include "../common/EQStreamIntf.h"
#include "../common/emu_opcodes.h"
#include "../common/EQPacket.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"
#include "Utility.h"
#include "Limits.h"

#include "../common/MiscFunctions.h"

/*
Known GM Commands
/goto
/kill
/summon
/emotezone
*/
//ZoneClientConnection::mGroupUpdatePacket = nullptr;

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
	mGroupJoinPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	mGroupLeavePacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	mGroupDisbandPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));
	mGroupLeaderChangePacket = new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));
	mGroupUpdateMembersPacket = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
}

void ZoneClientConnection::deinitialise() {
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
		mZone->notifyCharacterPositionChanged(mCharacter);
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
	ARG_PTR_CHECK_BOOL(pPacket);

	if (!mStreamInterface->CheckState(ESTABLISHED)) return false;

	EmuOpcode opcode = pPacket->GetOpcode();
	if (opcode == 0 || opcode == OP_FloatListThing) return true;

	switch (opcode) {
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
		Utility::print("[UNHANDLED OP_SetServerFilter]");
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
	case OP_PotionBelt:
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
	case OP_ZoneChange:
		Utility::print("[GOT OP_ZoneChange]");
		_handleZoneChange(pPacket);
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
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mConnected);

	// Check that this packet was expected.
	if (mZoneConnectionStatus != ZoneConnectionStatus::NONE) {
		Log::error("[Zone Client Connection] Received unexpected OP_ZoneEntry, dropping connection.");
		dropConnection();
		return;
	}
	// Check packet is the correct size.
	static const auto EXPECTED_SIZE = sizeof(ClientZoneEntry_Struct);
	if (pPacket->size != EXPECTED_SIZE) {
		Log::error("[Zone Client Connection] Received wrong sized ClientZoneEntry_Struct, dropping connection.");
		dropConnection();
		return;
	}

	auto payload = reinterpret_cast<ClientZoneEntry_Struct*>(pPacket->pBuffer);
	String characterName = Utility::safeString(payload->char_name, 64);

	// Check that this Zone is expecting this client.
	if (!mZone->checkAuthentication(characterName)) {
		Log::error("[Zone Client Connection] Client not expected in Zone, dropping connection.");
		dropConnection();
		return;
	}
	// Retrieve Authentication.
	ClientAuthentication authentication;
	if (!mZone->getAuthentication(characterName, authentication)) {
		Log::error("[Zone Client Connection] Client authentication not found, dropping connection.");
		dropConnection();
		return;
	}

	mZone->removeAuthentication(characterName); // Character has arrived so we can stop expecting them.
	mZoneConnectionStatus = ZoneConnectionStatus::ZoneEntryReceived;

	// Determine whether Character is zoning or logging in.
	Character* character = mZone->getZoningCharacter(characterName);

	// Character is coming from another zone.
	if (character) {
		mConnectionOrigin = ConnectionOrigin::Zone;
		mCharacter = character;
	}
	// Character is coming from character select.
	else {
		mConnectionOrigin = ConnectionOrigin::Character_Select;

		// Load Character. (Character becomes responsible for this memory AFTER Character::initialise)
		auto profile = new PlayerProfile_Struct();
		memset(profile, 0, sizeof(PlayerProfile_Struct));
		auto extendedProfile = new ExtendedProfile_Struct();
		memset(extendedProfile, 0, sizeof(ExtendedProfile_Struct));
		uint32 characterID = 0;
		if (!DataStore::getInstance().loadCharacter(characterName, characterID, profile, extendedProfile)) {
			Log::error("[Zone Client Connection] Failed to load character, dropping connection.");
			dropConnection();
			safe_delete(profile);
			safe_delete(extendedProfile);
			return;
		}

		// We will load this up every time for now but soon this data can be passed between Zones.
		// Initialise Character.
		mCharacter = new Character(characterID, authentication);
		if (!mCharacter->initialise(profile, extendedProfile)) {
			Log::error("[Zone Client Connection] Initialising Character failed, dropping connection.");
			dropConnection();
			safe_delete(mCharacter);
			return;
		}
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
	// The entityid field in the Player Profile is used by the Client in relation to Group Leadership AA // TODO: How?
	//mCharacter->getProfile()->entityid = mCharacter->getSpawnID();
	mCharacter->getProfile()->zone_id = mZone->getID();
	mCharacter->getProfile()->zoneInstance = mZone->getInstanceID();
	memcpy(outPacket->pBuffer, mCharacter->getProfile(), outPacket->size);
	outPacket->priority = 6;
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendZoneEntry() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	auto payload = (ServerZoneEntry_Struct*)outPacket->pBuffer;

	strcpy(payload->player.spawn.name, mCharacter->getName().c_str());
	payload->player.spawn.heading = 0;//FloatToEQ19(heading);
	payload->player.spawn.x = 0; //FloatToEQ19(x_pos);//((int32)x_pos)<<3;
	payload->player.spawn.y = 0; // FloatToEQ19(y_pos);//((int32)y_pos)<<3;
	payload->player.spawn.z = 0; // FloatToEQ19(z_pos);//((int32)z_pos)<<3;
	payload->player.spawn.spawnId = mCharacter->getSpawnID();
	payload->player.spawn.curHp = 50;// static_cast<uint8>(GetHPRatio());
	payload->player.spawn.max_hp = 100;		//this field needs a better name
	payload->player.spawn.race = mCharacter->getRace();
	payload->player.spawn.runspeed = mCharacter->getRunSpeed();
	payload->player.spawn.walkspeed = mCharacter->getWalkSpeed();
	payload->player.spawn.class_ = mCharacter->getClass();
	payload->player.spawn.gender = mCharacter->getGender();
	payload->player.spawn.level = mCharacter->getLevel();
	payload->player.spawn.deity = mCharacter->getDeity();
	payload->player.spawn.animation = 0;
	payload->player.spawn.findable = 0;
	payload->player.spawn.light = 1;
	payload->player.spawn.showhelm = mCharacter->getShowHelm();

	payload->player.spawn.invis = 0; // (invisible || hidden) ? 1 : 0;	// TODO: load this before spawning players
	payload->player.spawn.NPC = 0; // isClient() ? 0 : 1;
	payload->player.spawn.IsMercenary = 0;

	payload->player.spawn.petOwnerId = 0;// ownerid;

	payload->player.spawn.haircolor = 0; // haircolor;
	payload->player.spawn.beardcolor = 0; // beardcolor;
	payload->player.spawn.eyecolor1 = 0; //eyecolor1;
	payload->player.spawn.eyecolor2 = 0; // eyecolor2;
	payload->player.spawn.hairstyle = 0; // hairstyle;
	payload->player.spawn.face = 0; // luclinface;
	payload->player.spawn.beard = 0; // beard;
	payload->player.spawn.StandState = mCharacter->getAppearance();
	payload->player.spawn.drakkin_heritage = 0; // drakkin_heritage;
	payload->player.spawn.drakkin_tattoo = 0; // drakkin_tattoo;
	payload->player.spawn.drakkin_details = 0; // drakkin_details;
	payload->player.spawn.equip_chest2 = 0; // texture;
	payload->player.spawn.helm = 0;//helmtexture;
	payload->player.spawn.guildrank = mCharacter->getGuildRank();
	payload->player.spawn.size = mCharacter->getSize();
	payload->player.spawn.bodytype = BT_Humanoid;
	payload->player.spawn.flymode = 0;// FindType(SE_Levitate) ? 2 : 0;
	//payload->player.spawn.lastName[0] = '\0';
	strcpy(payload->player.spawn.lastName, mCharacter->getLastName().c_str());
	memset(payload->player.spawn.set_to_0xFF, 0xFF, sizeof(payload->player.spawn.set_to_0xFF));
	payload->player.spawn.afk = 0;// AFK;
	payload->player.spawn.lfg = 0;// LFG; // afk and lfg are cleared on zoning on live
	payload->player.spawn.anon = mCharacter->getAnonymous();
	payload->player.spawn.gm = mCharacter->getGM();
	payload->player.spawn.guildID = mCharacter->getGuildID();
	payload->player.spawn.is_pet = 0;
	payload->player.spawn.curHp = 1;
	payload->player.spawn.NPC = 0;
	payload->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.
	outPacket->priority = 6;
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendZoneSpawns() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ZoneSpawns, 0, 0);
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendTributeUpdate() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_TributeUpdate, sizeof(TributeInfo_Struct));
	auto payload = (TributeInfo_Struct *)outPacket->pBuffer;
	mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendInventory() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_CharInventory, 0);
	mStreamInterface->QueuePacket(outPacket);
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
	ARG_PTR_CHECK(pPacket);
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
	ARG_PTR_CHECK(pPacket);

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
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendZonepoints, 0);
	//mStreamInterface->QueuePacket(outPacket);
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
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(PlayerPositionUpdateClient_Struct);

	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE || pPacket->size == EXPECTED_PAYLOAD_SIZE + 1);

	auto payload = reinterpret_cast<PlayerPositionUpdateClient_Struct*>(pPacket->pBuffer);

	if (mCharacter->getX() != payload->x_pos || mCharacter->getY() != payload->y_pos || mCharacter->getZ() != payload->z_pos || FloatToEQ19(mCharacter->getHeading()) != payload->heading || mCharacter->getAnimation() != payload->animation) {
		mCharacter->setPosition(payload->x_pos, payload->y_pos, payload->z_pos, EQ19toFloat(payload->heading));
		mCharacter->setAnimation(payload->animation);
		mCharacter->setPositionDeltas(payload->delta_x, payload->delta_y, payload->delta_z, payload->delta_heading);
		mZone->notifyCharacterPositionChanged(mCharacter);

		// Restart the force send timer.
		mForceSendPositionTimer.Start();
	}
}

void ZoneClientConnection::_handleSpawnAppearance(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(SpawnAppearance_Struct);
	
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE || pPacket->size == EXPECTED_PAYLOAD_SIZE + 1);

	auto payload = reinterpret_cast<SpawnAppearance_Struct*>(pPacket->pBuffer);
	const uint16 actionType = payload->type;
	const uint32 actionParameter = payload->parameter;

	// Ignore if spawn id does not match this characters ID.
	if (payload->spawn_id != mCharacter->getSpawnID()) {
		// Note: UF client sends spawn ID (0) and action type (51) every few seconds. Not sure why.
		return;
	}

	switch (actionType) {
		// Handle animation.
	case SpawnAppearanceType::Animation:
		switch (actionParameter) {
		case SpawnAppearanceAnimation::Standing:
			mCharacter->setStanding(true);
			mZone->notifyCharacterStanding(mCharacter);
			break;
		case SpawnAppearanceAnimation::Freeze:
			break;
		case SpawnAppearanceAnimation::Looting:
			break;
		case SpawnAppearanceAnimation::Sitting:
			mCharacter->setStanding(false);
			mZone->notifyCharacterSitting(mCharacter);
			break;
		case SpawnAppearanceAnimation::Crouch:
			// Crouch or Jump triggers this.
			mZone->notifyCharacterCrouching(mCharacter);
			mCharacter->_setAppearance(Crouch); // TODO: May eventually need to do something like mCharacter->setCrouching(true); 
			break;
		case SpawnAppearanceAnimation::Death:
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
			mZone->notifyCharacterAnonymous(mCharacter);
		}
		// Anything else is ignored.
		break;
		// Handle AFK
	case SpawnAppearanceType::AFK:
		if (actionParameter == 0) {
			// Update character and notify zone.
			mCharacter->setAFK(false);
			mZone->notifyCharacterAFK(mCharacter);
		}
		else if (actionParameter == 1) {
			// Update character and notify zone.
			mCharacter->setAFK(true);
			mZone->notifyCharacterAFK(mCharacter);
		}
		// Anything else is ignored.
		break;
	case SpawnAppearanceType::ShowHelm:
		if (actionParameter == 0) {
			// Update character and notify zone.
			mCharacter->setShowHelm(false);
			mZone->notifyCharacterShowHelm(mCharacter);
		}
		else if (actionParameter == 1) {
			// Update character and notify zone.
			mCharacter->setShowHelm(true);
			mZone->notifyCharacterShowHelm(mCharacter);
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
	case SpawnAppearanceType::Sneak:
		break;
	case SpawnAppearanceType::Invisible:
		break;
	case SpawnAppearanceType::Size:
		break;
	case SpawnAppearanceType::Light:
		break;
	default:
		StringStream ss;
		ss << "[Zone Client Connection] Got unexpected SpawnAppearanceTypes : " << actionType;
		Log::error(ss.str());
		break;
	}
}

void ZoneClientConnection::_handleCamp(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);

	mCharacter->startCamp();
}

void ZoneClientConnection::_handleChannelMessage(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_SIZE = sizeof(ChannelMessage_Struct); // NOTE: This packet size increases with message size.
	static const auto MAXIMUM_SIZE = 661; // This is the absolute largest (513 characters + 148 bytes for the rest of the contents).

	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size <= MAXIMUM_SIZE);
	PACKET_SIZE_CHECK(pPacket->size >= EXPECTED_SIZE);

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
		break;
	case ChannelID::CH_GROUP:
		GroupManager::getInstance().handleMessage(mCharacter, message);
		break;
	case ChannelID::CH_SHOUT:
		mZone->notifyCharacterChatShout(mCharacter, message);
		break;
	case ChannelID::CH_AUCTION:
		mZone->notifyCharacterChatAuction(mCharacter, message);
		break;
	case ChannelID::CH_OOC:
		mZone->notifyCharacterChatOOC(mCharacter, message);
		break;
	case ChannelID::CH_GMSAY:
	case ChannelID::CH_BROADCAST:
		// GM_SAY / CH_BROADAST are unused as far as I know.
		break;
	case ChannelID::CH_TELL:
		if (senderName.length() > 0 && targetName.length() > 0) {
			mZone->notifyCharacterChatTell(mCharacter, targetName, message);
		}
		break;
	case ChannelID::CH_SAY:
		// Check whether user has entered a command.
		if (message[0] == COMMAND_TOKEN) {
			mCommandHandler->command(mCharacter, message);
			break;
		}
		mZone->notifyCharacterChatSay(mCharacter, message);
		break;
	case ChannelID::CH_RAID:
		break;
	case ChannelID::CH_UCS:
		break;
		// /emote dances around wildly!
	case ChannelID::CH_EMOTE:
		mZone->notifyCharacterEmote(mCharacter, message);
		break;
	default:
		StringStream ss;
		ss << "[Zone Client Connection] " << __FUNCTION__ << " Got unexpected channel number: " << channel;
		Log::error(ss.str());
		break;
	}
}

void ZoneClientConnection::sendPosition() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	auto payload = reinterpret_cast<PlayerPositionUpdateServer_Struct*>(outPacket->pBuffer);
	memset(payload, 0xff, sizeof(PlayerPositionUpdateServer_Struct));
	payload->spawn_id = mCharacter->getSpawnID();
	payload->x_pos = FloatToEQ19(mCharacter->getX());
	payload->y_pos = FloatToEQ19(mCharacter->getY());
	payload->z_pos = FloatToEQ19(mCharacter->getZ());
	payload->delta_x = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaX()));
	payload->delta_y = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaY()));
	payload->delta_z = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaZ()));
	payload->heading = FloatToEQ19(static_cast<float>(mCharacter->getHeading()));
	payload->animation = 0;
	payload->delta_heading = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaHeading()));
	payload->padding0002 = 0;
	payload->padding0006 = 7;
	payload->padding0014 = 0x7f;
	payload->padding0018 = 0x5df27;

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
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
	payload->msg_type = static_cast<std::uint32_t>(pType);
	strcpy(payload->message, pMessage.c_str());
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleLogOut(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
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
	ARG_PTR_CHECK(pPacket);
	// TODO:
}

void ZoneClientConnection::_handleUpdateAA(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	// TODO:
}

void ZoneClientConnection::_handleTarget(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == sizeof(ClientTarget_Struct));

	auto payload = reinterpret_cast<ClientTarget_Struct*>(pPacket->pBuffer);

	mZone->handleTarget(mCharacter, payload->new_target);
}

void ZoneClientConnection::_handleTGB(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(uint32);

	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE);

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
	payload->color = static_cast<std::uint32_t>(pType);
	payload->string_id = static_cast<std::uint32_t>(pStringID);

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
	payload->type = static_cast<std::uint32_t>(pType);
	payload->string_id = static_cast<std::uint32_t>(pStringID);

	Utility::DynamicStructure dynamicStructure(outPacket->pBuffer, packetSize);
	dynamicStructure.movePointer(sizeof(FormattedMessage_Struct));

	if (pParameter0.length() != 0) dynamicStructure.writeString(pParameter0);
	if (pParameter1.length() != 0) dynamicStructure.writeString(pParameter1);
	if (pParameter2.length() != 0) dynamicStructure.writeString(pParameter2);
	if (pParameter3.length() != 0) dynamicStructure.writeString(pParameter3);
	if (pParameter4.length() != 0) dynamicStructure.writeString(pParameter4);
	if (pParameter5.length() != 0) dynamicStructure.writeString(pParameter5);
	if (pParameter6.length() != 0) dynamicStructure.writeString(pParameter6);
	if (pParameter7.length() != 0) dynamicStructure.writeString(pParameter7);
	if (pParameter8.length() != 0) dynamicStructure.writeString(pParameter8);
	if (pParameter9.length() != 0) dynamicStructure.writeString(pParameter9);

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);

	// Check payload size calculation.
	if (dynamicStructure.getBytesWritten() != packetSize) {
		StringStream ss;
		ss << "[Zone Client Connection] Wrong amount of data written in sendWhoResults. Expected " << packetSize << " Got " << dynamicStructure.getBytesWritten();
		Log::error(ss.str());
	}
}

void ZoneClientConnection::sendHPUpdate() {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_HPUpdate, sizeof(SpawnHPUpdate_Struct));
	auto payload = reinterpret_cast<SpawnHPUpdate_Struct*>(outPacket->pBuffer);
	payload->spawn_id = mCharacter->getSpawnID();
	payload->cur_hp = mCharacter->getCurrentHP();
	payload->max_hp = mCharacter->getMaximumHP();

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

EQApplicationPacket* ZoneClientConnection::makeCharacterSpawnPacket() {
	EXPECTED_BOOL(mConnected);

	auto outPacket = new EQApplicationPacket(OP_NewSpawn, sizeof(NewSpawn_Struct));
	auto payload = reinterpret_cast<NewSpawn_Struct*>(outPacket->pBuffer);
	populateSpawnStruct(payload);
	return outPacket;
}

EQApplicationPacket* ZoneClientConnection::makeCharacterPositionUpdate() {
	//ERROR_CONDITION(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	auto payload = reinterpret_cast<PlayerPositionUpdateServer_Struct*>(outPacket->pBuffer);
	payload->spawn_id = mCharacter->getSpawnID();
	payload->x_pos = FloatToEQ19(mCharacter->getX());
	payload->y_pos = FloatToEQ19(mCharacter->getY());
	payload->z_pos = FloatToEQ19(mCharacter->getZ());
	payload->delta_x = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaX()));
	payload->delta_y = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaY()));
	payload->delta_z = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaZ()));
	payload->heading = FloatToEQ19(mCharacter->getHeading());
	payload->padding0002 = 0;
	payload->padding0006 = 7;
	payload->padding0014 = 0x7f;
	payload->padding0018 = 0x5df27;
	payload->animation = mCharacter->getAnimation();
	payload->delta_heading = NewFloatToEQ13(static_cast<float>(mCharacter->getDeltaHeading()));

	return outPacket;
}


void ZoneClientConnection::sendPacket(EQApplicationPacket* pPacket) {
	EXPECTED(mConnected);
	mStreamInterface->QueuePacket(pPacket);
}

void ZoneClientConnection::populateSpawnStruct(NewSpawn_Struct* pSpawn) {
	EXPECTED(mConnected);

	pSpawn->spawn.gm = mCharacter->getGM();
	pSpawn->spawn.anon = mCharacter->getAnonymous();
	pSpawn->spawn.heading = FloatToEQ19(mCharacter->getHeading());
	pSpawn->spawn.x = FloatToEQ19(mCharacter->getX());
	pSpawn->spawn.y = FloatToEQ19(mCharacter->getY());
	pSpawn->spawn.z = FloatToEQ19(mCharacter->getZ());
	pSpawn->spawn.spawnId = mCharacter->getSpawnID();
	pSpawn->spawn.curHp = 10; //static_cast<uint8>(GetHPRatio()); // TODO:
	pSpawn->spawn.max_hp = 100;
	pSpawn->spawn.race = mCharacter->getRace();
	pSpawn->spawn.runspeed = mCharacter->getRunSpeed();
	pSpawn->spawn.walkspeed = mCharacter->getWalkSpeed();
	pSpawn->spawn.class_ = mCharacter->getClass();
	pSpawn->spawn.gender = mCharacter->getGender();
	pSpawn->spawn.level = mCharacter->getLevel();
	pSpawn->spawn.deity = mCharacter->getDeity();
	pSpawn->spawn.animation = mCharacter->getAnimation();
	pSpawn->spawn.findable = 0; // TODO: I don't think PCs are ever findable.
	pSpawn->spawn.light = 1; // TODO: Items
	pSpawn->spawn.showhelm = mCharacter->getShowHelm();
	pSpawn->spawn.invis = 0;	// TODO: GM Hide?
	pSpawn->spawn.NPC = 0;
	pSpawn->spawn.IsMercenary = 0;
	pSpawn->spawn.petOwnerId = 0;
	// TODO: Below Appearances
	pSpawn->spawn.haircolor = 0;
	pSpawn->spawn.beardcolor = 0;
	pSpawn->spawn.eyecolor1 = 0;
	pSpawn->spawn.eyecolor2 = 0;
	pSpawn->spawn.hairstyle = 0;
	pSpawn->spawn.face = 0;
	pSpawn->spawn.beard = 0;
	pSpawn->spawn.StandState = mCharacter->getAppearance();
	pSpawn->spawn.drakkin_heritage = 0;
	pSpawn->spawn.drakkin_tattoo = 0;
	pSpawn->spawn.drakkin_details = 0;
	pSpawn->spawn.equip_chest2 = 0;
	pSpawn->spawn.helm = 0;
	// TODO: Look at old helm stuff when Items
	pSpawn->spawn.guildrank = mCharacter->getGuildRank();
	pSpawn->spawn.size = mCharacter->getSize();
	pSpawn->spawn.bodytype = BT_Humanoid;
	pSpawn->spawn.flymode = 0;
	pSpawn->spawn.guildID = mCharacter->getGuildID();
	strcpy(pSpawn->spawn.name, mCharacter->getName().c_str());
	strcpy(pSpawn->spawn.lastName, mCharacter->getLastName().c_str());
	// TODO: Equipment materials when Items.
	memset(pSpawn->spawn.set_to_0xFF, 0xFF, sizeof(pSpawn->spawn.set_to_0xFF));
}

void ZoneClientConnection::_handleEmote(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(Emote_Struct);

	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE);

	static const unsigned int MAX_EMOTE_SIZE = 1024;
	auto payload = reinterpret_cast<Emote_Struct*>(pPacket->pBuffer);
	String message = Utility::safeString(payload->message, MAX_EMOTE_SIZE);
	mZone->notifyCharacterEmote(mCharacter, message);
}

void ZoneClientConnection::_handleAnimation(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(Animation_Struct);
	
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE);

	auto payload = reinterpret_cast<Animation_Struct*>(pPacket->pBuffer);
	mZone->notifyCharacterAnimation(mCharacter, payload->action, payload->value, false);
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

	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE);

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

	Utility::DynamicStructure dynamicStructure(outPacket->pBuffer, packetSize);
	dynamicStructure.movePointer(sizeof(WhoAllReturnStruct)); // Move the pointer to where WhoAllPlayer begin.

	ss << "Written 1: " << dynamicStructure.getBytesWritten();
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
	const bool receiverIsGM = mCharacter->getGM() == 1 ? true : false;
	for (auto i : pMatches) {
		// NOTE: The write methods below *MUST* stay in order.

		// Determine Format String ID.
		uint32 formatString = FS_DEFAULT;
		if (i->getAnonymous() != ANON_None) {
			// Player is /roleplay
			if (i->getAnonymous() == ANON_Roleplay) 
				formatString = FS_ROLEPLAY;
			// Player is /anonymous
			if (i->getAnonymous() == ANON_Anonymous)
				formatString = FS_ANONYMOUS;

			// Allows GM to see through anonymous.
			if (receiverIsGM)
				formatString = FS_GM_ANONYMOUS;
		}
		dynamicStructure.write<uint32>(formatString); // formatstring

		dynamicStructure.write<uint32>(0xFFFFFFFF); // pidstring (Not sure what this does).

		dynamicStructure.writeString(i->getName()); // name
		
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
		dynamicStructure.write<uint32>(rankStringID); // rankstring

		dynamicStructure.writeString(FakeGuild); // guild
		//dynamicStructure.write<uint32>(0); // guild
		dynamicStructure.write<uint32>(0xFFFFFFFF); // unknown80[0]
		dynamicStructure.write<uint32>(0xFFFFFFFF); // unknown80[1]
		//dynamicStructure.write<uint32>(0xFFFFFFFF); // zonestring
		dynamicStructure.write<uint32>(i->getZone()->getLongNameStringID()); // zonestring // This is StringID 
		dynamicStructure.write<uint32>(4); // zone (Not sure what this does).
		dynamicStructure.write<uint32>(i->getClass()); // class_
		dynamicStructure.write<uint32>(i->getLevel()); // level
		dynamicStructure.write<uint32>(i->getRace()); // race
		dynamicStructure.writeString(FakeAccount); // account
		//dynamicStructure.write<uint32>(0); // account
		dynamicStructure.write<uint32>(0); // unknown100
		//ss << "Written 4: " << dynamicStructure.getBytesWritten();
		//Log::error(ss.str());
		//ss.str("");
	}

	// Check payload size calculation.
	if (dynamicStructure.getBytesWritten() != packetSize) {
		StringStream ss;
		ss << "[Zone Client Connection] Wrong amount of data written in sendWhoResults. Expected " << packetSize << " Got " << dynamicStructure.getBytesWritten();
		Log::error(ss.str());
	}

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendChannelMessage(const ChannelID pChannel, const String& pSenderName, const String& pMessage) {
	EXPECTED(mConnected);

	auto outPacket = new EQApplicationPacket(OP_ChannelMessage, sizeof(ChannelMessage_Struct)+pMessage.length() + 1);
	auto payload = reinterpret_cast<ChannelMessage_Struct*>(outPacket->pBuffer);
	payload->language = Language::COMMON_TONGUE_LANG;
	payload->skill_in_language = 0;
	payload->chan_num = static_cast<std::uint32_t>(pChannel);
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
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GroupInvite_Struct));

	auto payload = reinterpret_cast<GroupInvite_Struct*>(pPacket->pBuffer);

	const String inviterName = Utility::safeString(payload->inviter_name, MAX_CHARACTER_NAME_LENGTH);
	const String inviteeName = Utility::safeString(payload->invitee_name, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(inviterName));
	EXPECTED(Limits::characterNameLength(inviteeName));
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
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GroupGeneric_Struct));

	auto payload = reinterpret_cast<GroupGeneric_Struct*>(pPacket->pBuffer);
	
	String inviterName = Utility::safeString(payload->name1, MAX_CHARACTER_NAME_LENGTH); // Character who invited.
	String inviteeName = Utility::safeString(payload->name2, MAX_CHARACTER_NAME_LENGTH); // Character accepting invite.
	EXPECTED(Limits::characterNameLength(inviterName));
	EXPECTED(Limits::characterNameLength(inviteeName));
	EXPECTED(inviteeName == mCharacter->getName()); // Check: Sanity

	// TODO: This can be spoofed to join groups...

	GroupManager::getInstance().handleAcceptInvite(mCharacter, inviterName);
}

void ZoneClientConnection::_handleGroupCanelInvite(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GroupCancel_Struct));

	auto payload = reinterpret_cast<GroupCancel_Struct*>(pPacket->pBuffer);
	String inviterName = Utility::safeString(payload->name1, MAX_CHARACTER_NAME_LENGTH);
	String inviteeName = Utility::safeString(payload->name2, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(inviterName));
	EXPECTED(Limits::characterNameLength(inviteeName));
	EXPECTED(inviteeName == mCharacter->getName()); // Check: Sanity

	GroupManager::getInstance().handleDeclineInvite(mCharacter, inviterName);
}

void ZoneClientConnection::sendGroupCreate() {
	EXPECTED(mConnected);

	int packetSize = 31 + mCharacter->getName().length() + 1; // Magic number due to no packet structure.
	auto outPacket = new EQApplicationPacket(OP_GroupUpdateB, packetSize);

	Utility::DynamicStructure dynamicStructure(outPacket->pBuffer, packetSize);
	dynamicStructure.write<uint32>(0); // 4
	dynamicStructure.write<uint32>(1); // 8
	dynamicStructure.write<uint8>(0); // 9
	dynamicStructure.write<uint32>(0); // 13
	dynamicStructure.writeString(mCharacter->getName()); // dynamic
	dynamicStructure.write<uint8>(0); // 14
	dynamicStructure.write<uint8>(0); // 15
	dynamicStructure.write<uint8>(0); // 16
	dynamicStructure.write<uint32>(mCharacter->getLevel()); // 20
	dynamicStructure.write<uint8>(0); // 21
	dynamicStructure.write<uint32>(0); // 25
	dynamicStructure.write<uint32>(0); // 29
	dynamicStructure.write<uint16>(0); // 31

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);

	// Check payload size calculation.
	if (dynamicStructure.getBytesWritten() != packetSize) {
		StringStream ss;
		ss << "[Zone Client Connection] Wrong amount of data written in sendGroupCreate. Expected " << packetSize << " Got " << dynamicStructure.getBytesWritten();
		Log::error(ss.str());
	}
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
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GroupGeneric_Struct));
	
	auto payload = reinterpret_cast<GroupGeneric_Struct*>(pPacket->pBuffer);

	String removeCharacterName = Utility::safeString(payload->name1, MAX_CHARACTER_NAME_LENGTH);
	String myCharacterName = Utility::safeString(payload->name2, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(removeCharacterName));
	EXPECTED(Limits::characterNameLength(myCharacterName));
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
	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GroupMakeLeader_Struct));
	EXPECTED(mCharacter->hasGroup());

	auto payload = reinterpret_cast<GroupMakeLeader_Struct*>(pPacket->pBuffer);

	String currentLeader = Utility::safeString(payload->CurrentLeader, MAX_CHARACTER_NAME_LENGTH);
	String newLeader = Utility::safeString(payload->NewLeader, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(currentLeader));
	EXPECTED(Limits::characterNameLength(newLeader));
	EXPECTED(currentLeader == mCharacter->getName());

	GroupManager::getInstance().handleMakeLeader(mCharacter, newLeader);
}

void ZoneClientConnection::sendRequestZoneChange(uint32 pZoneID, uint16 pInstanceID) {
	auto outPacket = new EQApplicationPacket(OP_RequestClientZoneChange, sizeof(RequestClientZoneChange_Struct));
	auto payload = reinterpret_cast<RequestClientZoneChange_Struct*>(outPacket->pBuffer);

	payload->zone_id = pZoneID;
	payload->instance_id = pInstanceID;
	payload->type = 0x01; // Copied.

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::sendZoneChange(uint32 pZoneID, uint16 pInstanceID) {
	auto outPacket = new EQApplicationPacket(OP_ZoneChange, sizeof(ZoneChange_Struct));
	auto payload = reinterpret_cast<ZoneChange_Struct*>(outPacket->pBuffer);
	strcpy(payload->char_name, mCharacter->getName().c_str());
	payload->zoneID = pZoneID;
	payload->instanceID = pInstanceID;
	payload->success = 1;
	outPacket->priority = 6;
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_handleZoneChange(const EQApplicationPacket* pPacket) {
	static const auto EXPECTED_PAYLOAD_SIZE = sizeof(ZoneChange_Struct);

	ARG_PTR_CHECK(pPacket);
	PACKET_SIZE_CHECK(pPacket->size == EXPECTED_PAYLOAD_SIZE);

	auto x = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
	auto xx = reinterpret_cast<CancelTrade_Struct*>(x->pBuffer);
	xx->fromid = mCharacter->getSpawnID();
	xx->action = groupActUpdate;
	mStreamInterface->FastQueuePacket(&x);

	auto a = new EQApplicationPacket(OP_PreLogoutReply);
	mStreamInterface->FastQueuePacket(&a);

	auto payload = reinterpret_cast<ZoneChange_Struct*>(pPacket->pBuffer);
	sendZoneChange(payload->zoneID, payload->instanceID);

	mZone->notifyCharacterZoneChange(mCharacter, payload->zoneID, payload->instanceID);
}

void ZoneClientConnection::_handleGuildCreate(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mCharacter->hasGuild() == false);
	PACKET_SIZE_CHECK(pPacket->size == MAX_GUILD_NAME_LENGTH);
	
	const String guildName = Utility::safeString(reinterpret_cast<char*>(pPacket->pBuffer), MAX_GUILD_NAME_LENGTH);
	EXPECTED(Limits::guildNameLength(guildName));

	GuildManager::getInstance().handleCreate(mCharacter, guildName);
}

void ZoneClientConnection::_handleGuildDelete(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mCharacter->hasGuild());

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
	outPacket->size = MAX_GUILD_NAME_LENGTH + (MAX_GUILD_NAME_LENGTH * MAX_GUILDS); // TODO: Work out the minimum sized packet UF will accept.
	outPacket->pBuffer = reinterpret_cast<unsigned char*>(GuildManager::getInstance()._getGuildNames());

	mStreamInterface->QueuePacket(outPacket);
	outPacket->pBuffer = nullptr;
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleGuildInvite(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild()); // Check: Character has a guild.
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GuildCommand_Struct));

	auto payload = reinterpret_cast<GuildCommand_Struct*>(pPacket->pBuffer);

	EXPECTED(payload->guildeqid == mCharacter->getGuildID()); // Check: Sanity. Why the fuck does the client send this?

	String toCharacterName = Utility::safeString(payload->othername, MAX_CHARACTER_NAME_LENGTH);
	String fromCharacterName = Utility::safeString(payload->myname, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(toCharacterName)); // NOTE: Client does not check this..
	EXPECTED(Limits::characterNameLength(fromCharacterName));
	EXPECTED(fromCharacterName == mCharacter->getName()); // Check: Sanity.

	GuildManager::getInstance().handleInviteSent(mCharacter, toCharacterName);
}

void ZoneClientConnection::_handleGuildRemove(const EQApplicationPacket* pPacket) {
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild()); // Check: Character has a guild.
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GuildCommand_Struct));

	auto payload = reinterpret_cast<GuildCommand_Struct*>(pPacket->pBuffer);

	String toCharacterName = Utility::safeString(payload->othername, MAX_CHARACTER_NAME_LENGTH);
	String fromCharacterName = Utility::safeString(payload->myname, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(toCharacterName));
	EXPECTED(Limits::characterNameLength(fromCharacterName));
	EXPECTED(fromCharacterName == mCharacter->getName()) // Check: Sanity.

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
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mConnected);
	if (mCharacter->hasGuild()) { return; } // NOTE: UF sends OP_GuildInvite and OP_GuildInviteAccept(response=2,guildid=0) when using /guildinvite .. not sure why.
	EXPECTED(mCharacter->hasPendingGuildInvite()); // Check: This Character has actually been invited to *A* Guild
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GuildInviteAccept_Struct));

	auto payload = reinterpret_cast<GuildInviteAccept_Struct*>(pPacket->pBuffer);
	
	String characterName = Utility::safeString(payload->newmember, MAX_CHARACTER_NAME_LENGTH);
	String inviterName = Utility::safeString(payload->inviter, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(Limits::characterNameLength(characterName));
	EXPECTED(Limits::characterNameLength(inviterName));
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
	ARG_PTR_CHECK(pPacket);
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild());
	PACKET_SIZE_CHECK(pPacket->size == sizeof(GuildMOTD_Struct));

	auto payload = reinterpret_cast<GuildMOTD_Struct*>(pPacket->pBuffer);

	String characterName = Utility::safeString(payload->name, MAX_CHARACTER_NAME_LENGTH);
	EXPECTED(characterName == mCharacter->getName()); // Check: Sanity.
	String motd = Utility::safeString(payload->motd, MAX_GUILD_MOTD_LENGTH);

	GuildManager::getInstance().handleSetMOTD(mCharacter, motd);
}

void ZoneClientConnection::sendGuildMOTD(const String& pMOTD, const String& pMOTDSetByName) {
	EXPECTED(mConnected);
	EXPECTED(mCharacter->hasGuild());

	auto outPacket = new EQApplicationPacket(OP_GuildMOTD, sizeof(GuildMOTD_Struct));
	auto payload = reinterpret_cast<GuildMOTD_Struct*>(outPacket->pBuffer);
	payload->unknown0 = 0;
	strcpy(payload->name, mCharacter->getName().c_str());
	strcpy(payload->setby_name, pMOTDSetByName.c_str());
	strcpy(payload->motd, pMOTD.c_str());

	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}
