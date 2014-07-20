#include "ZoneClientConnection.h"
#include "Constants.h"
#include "Zone.h"
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
#include <sstream>

#include "../common/MiscFunctions.h"

ZoneClientConnection::ZoneClientConnection(EQStreamInterface* pStreamInterface, DataStore* pDataStore, Zone* pZone) :
mStreamInterface(pStreamInterface),
mZone(pZone),
mDataStore(pDataStore),
mCharacter(0),
mCommandHandler(0),
mZoneConnectionStatus(ZoneConnectionStatus::NONE)
{
	mCommandHandler = new CommandHandler();
}

ZoneClientConnection::~ZoneClientConnection() { }

void ZoneClientConnection::update() {
	if (!mStreamInterface->CheckState(ESTABLISHED)) {
		return;
	}

	// Handle packets.
	EQApplicationPacket* packet = 0;
	bool ret = true;
	while (ret && (packet = (EQApplicationPacket*)mStreamInterface->PopPacket())) {
		ret = _handlePacket(packet);
		delete packet;
	}
}

void ZoneClientConnection::dropConnection()
{

}

bool ZoneClientConnection::_handlePacket(const EQApplicationPacket* pPacket) {	
	EmuOpcode opcode = pPacket->GetOpcode();
	if (opcode == 0 || opcode == OP_FloatListThing) return true;

	switch (opcode) {
	case OP_AckPacket:
		Utility::print("OP_AckPacket");
		break;
	case OP_ZoneEntry:
		Utility::print("OP_ZoneEntry");
		_handleZoneEntry(pPacket);
		break;
	case OP_ReqClientSpawn:
		Utility::print("OP_ReqClientSpawn");
		_handleRequestClientSpawn(pPacket);
		break;
	case OP_SetServerFilter:
		Utility::print("OP_SetServerFilter");
		break;
	case OP_SendAATable:
		Utility::print("OP_SendAATable");
		break;
	case OP_SendExpZonein:
		Utility::print("OP_SendExpZonein");
		break;
	case OP_WorldObjectsSent:
		Utility::print("OP_WorldObjectsSent");
		break;
	case OP_ZoneComplete:
		Utility::print("OP_ZoneComplete");
		break;
	case OP_ReqNewZone:
		Utility::print("OP_ReqNewZone");
		break;
	case OP_SpawnAppearance:
		_handleSpawnAppearance(pPacket);
		break;
	case OP_WearChange:
		Utility::print("OP_WearChange");
		break;
	case OP_ClientUpdate:
		_handleClientUpdate(pPacket);
		break;
	case OP_ClientError:
		Utility::print("OP_ClientError");
		break;
	case OP_ApproveZone:
		Utility::print("OP_ApproveZone");
		break;
	case OP_TGB:
		Utility::print("OP_TGB");
		break;
	case OP_SendTributes:
		Utility::print("OP_SendTributes");
		break;
	case OP_SendGuildTributes:
		Utility::print("OP_SendGuildTributes");
		break;
	case OP_SendAAStats:
		Utility::print("OP_SendAAStats");
		break;
	case OP_ClientReady:
		Utility::print("OP_ClientReady");
		break;
	case OP_UpdateAA:
		Utility::print("OP_UpdateAA");
		break;
	case OP_BlockedBuffs:
		Utility::print("OP_BlockedBuffs");
		break;
	case OP_XTargetRequest:
		Utility::print("OP_XTargetRequest");
		break;
	case OP_XTargetAutoAddHaters:
		Utility::print("OP_XTargetAutoAddHaters");
		break;
	case OP_GetGuildsList:
		Utility::print("OP_GetGuildsList");
		break;
	case OP_TargetMouse:
		Utility::print("OP_TargetMouse");
		break;
	case OP_Camp:
		// Sent when user types /camp or presses the camp button.
		_handleCamp(pPacket);
		break;
	case OP_Logout:
		// This occurs 30 seconds after /camp
		_handleLogOut(pPacket);
		break;
	case OP_DeleteSpawn:
		// Client sends this after /camp
		_handleDeleteSpawn(pPacket);
		break;
	case OP_ChannelMessage:
		_handleChannelMessage(pPacket);
		break;
	default:
		//Utility::print("UNKNOWN PACKET");
		std::stringstream ss;
		ss << "Unknown Packet: " << opcode;
		Utility::print(ss.str());
		break;
	}
	return true;
}

void ZoneClientConnection::_handleZoneEntry(const EQApplicationPacket* pPacket) {
	// Check that this packet was expected.
	if (mZoneConnectionStatus != ZoneConnectionStatus::NONE) {
		Log::error("[Zone Client Connection] Received unexpected OP_ZoneEntry, dropping connection.");
		dropConnection();
		return;
	}
	// Check packet is the correct size.
	static const std::size_t EXPECTED_SIZE = sizeof(ClientZoneEntry_Struct);
	if (pPacket->size != EXPECTED_SIZE) {
		Log::error("[Zone Client Connection] Received wrong sized ClientZoneEntry_Struct, dropping connection.");
		dropConnection();
		return;
	}

	ClientZoneEntry_Struct* payload = (ClientZoneEntry_Struct*)pPacket->pBuffer;

	// Check character name is valid.
	if (strlen(payload->char_name) > 63) { // TODO: Remove magic number.
		Log::error("[Zone Client Connection] Received wrong sized character name in ClientZoneEntry_Struct, dropping connection.");
		dropConnection();
		return;
	}

	std::string characterName = payload->char_name;

	// Check that this Zone is expecting this client.
	if (!mZone->checkAuthentication(characterName)) {
		Log::error("[Zone Client Connection] Client not expected in Zone, dropping connection.");
		dropConnection();
	}

	mZone->removeAuthentication(characterName); // Character has arrived so we can stop expecting them.
	mZoneConnectionStatus = ZoneConnectionStatus::ZoneEntryReceived;

	// Load Character. (Character becomes responsible for this memory AFTER Character::initialise)
	PlayerProfile_Struct* profile = new PlayerProfile_Struct();
	memset(profile, 0, sizeof(PlayerProfile_Struct));
	ExtendedProfile_Struct* extendedProfile = new ExtendedProfile_Struct();
	memset(extendedProfile, 0, sizeof(ExtendedProfile_Struct));
	uint32 characterID = 0;
	if(!mDataStore->loadCharacter(characterName, characterID, profile, extendedProfile)) {
		Log::error("[Zone Client Connection] Failed to load character, dropping connection.");
		dropConnection();
		safe_delete(profile);
		safe_delete(extendedProfile);
		return;
	}

	// Initialise Character.
	mCharacter = new Character(characterID);
	if (!mCharacter->initialise(profile, extendedProfile)) {
		Log::error("[Zone Client Connection] Initialising Character failed, dropping connection.");
		dropConnection();
		safe_delete(mCharacter);
		return;
	}
	mCharacter->setZone(mZone);
	mCharacter->setSpawnID(mZone->getNextSpawnID());
	mCharacter->setConnection(this);

	// We will load this up every time for now but soon this data can be passed between Zones.

	// REPLY
	// OP_PlayerProfile
	_sendPlayerProfile();
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
}

void ZoneClientConnection::_sendTimeOfDay() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
	TimeOfDay_Struct* payload = (TimeOfDay_Struct*)outPacket->pBuffer;
	memset(payload, 0, sizeof(TimeOfDay_Struct)); // TODO:
	outPacket->priority = 6; // TODO: Look into this.
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendPlayerProfile() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));
	// The entityid field in the Player Profile is used by the Client in relation to Group Leadership AA // TODO: How?
	//m_pp.entityid = getID(); // TODO:
	memcpy(outPacket->pBuffer, mCharacter->getProfile(), outPacket->size);
	outPacket->priority = 6;
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendZoneEntry() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	ServerZoneEntry_Struct* payload = (ServerZoneEntry_Struct*)outPacket->pBuffer;

	//FillSpawnStruct(&outPacket->player, castToMOB());
	/////////////////////////
	strcpy(payload->player.spawn.name, mCharacter->getName().c_str());
	//if (isClient()) {
	//	strn0cpy(ns->spawn.lastName, lastname, sizeof(ns->spawn.lastName));
	//}

	payload->player.spawn.heading = 0;//FloatToEQ19(heading);
	payload->player.spawn.x = 0; //FloatToEQ19(x_pos);//((int32)x_pos)<<3;
	payload->player.spawn.y = 0; // FloatToEQ19(y_pos);//((int32)y_pos)<<3;
	payload->player.spawn.z = 0; // FloatToEQ19(z_pos);//((int32)z_pos)<<3;
	payload->player.spawn.spawnId = mCharacter->getSpawnID();
	payload->player.spawn.curHp = 50;// static_cast<uint8>(GetHPRatio());
	payload->player.spawn.max_hp = 100;		//this field needs a better name
	payload->player.spawn.race = 1; //race;
	payload->player.spawn.runspeed = 2.7; // runspeed;
	payload->player.spawn.walkspeed = 1.35; // runspeed * 0.5f;
	payload->player.spawn.class_ = 1;// class_;
	payload->player.spawn.gender = 1; // gender;
	payload->player.spawn.level = 1;// level;
	payload->player.spawn.deity = 396; // deity;
	payload->player.spawn.animation = 0;
	payload->player.spawn.findable = 0; // findable ? 1 : 0;
	payload->player.spawn.light = 1; // light;
	payload->player.spawn.showhelm = 1;

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
	payload->player.spawn.StandState = 0; // GetAppearanceValue(_appearance);
	payload->player.spawn.drakkin_heritage = 0; // drakkin_heritage;
	payload->player.spawn.drakkin_tattoo = 0; // drakkin_tattoo;
	payload->player.spawn.drakkin_details = 0; // drakkin_details;
	payload->player.spawn.equip_chest2 = 0; // texture;

	payload->player.spawn.helm = 0;//helmtexture;
	payload->player.spawn.helm = 0;
	payload->player.spawn.guildrank = 0xFF;
	payload->player.spawn.size = 10;//size;
	payload->player.spawn.bodytype = 0; // bodytype;
	payload->player.spawn.flymode = 0;// FindType(SE_Levitate) ? 2 : 0;
	payload->player.spawn.lastName[0] = '\0';
	memset(payload->player.spawn.set_to_0xFF, 0xFF, sizeof(payload->player.spawn.set_to_0xFF));
	/////////////////////////
	payload->player.spawn.afk = 0;// AFK;
	payload->player.spawn.lfg = 0;// LFG; // afk and lfg are cleared on zoning on live
	payload->player.spawn.anon = 0;// m_pp.anon;
	payload->player.spawn.gm = 0;// GetGM() ? 1 : 0;
	payload->player.spawn.guildID = 0;// GuildID();
	payload->player.spawn.is_pet = 0;
	/////////////////////////

	payload->player.spawn.curHp = 1;
	payload->player.spawn.NPC = 0;
	payload->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.
	outPacket->priority = 6;
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendZoneSpawns() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ZoneSpawns, 0, 0);
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendTributeUpdate()
{
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_TributeUpdate, sizeof(TributeInfo_Struct));
	TributeInfo_Struct* payload = (TributeInfo_Struct *)outPacket->pBuffer;
	mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendInventory()
{
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_CharInventory, 0);
	mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendWeather() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_Weather, 12);
	Weather_Struct* payload = (Weather_Struct*)outPacket->pBuffer;
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
	_sendDoors();
	_sendObjects();
	_sendAAStats();
	_sendZonePoints();
	_sendZoneServerReady();
	_sendExpZoneIn();
	_sendWorldObjectsSent();
}

void ZoneClientConnection::_sendDoors() {
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpawnDoor, 0);
	//mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendObjects() {
	return;
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_GroundSpawn, 0);
	//mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendZonePoints() {
	//EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendZonepoints, 0);
	//mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendAAStats() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendAAStats, 0);
	mStreamInterface->QueuePacket(outPacket);
}

void ZoneClientConnection::_sendZoneServerReady() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ZoneServerReady, 0);
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendExpZoneIn() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SendExpZonein, 0);
	mStreamInterface->FastQueuePacket(&outPacket);
}

void ZoneClientConnection::_sendWorldObjectsSent() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleClientUpdate(const EQApplicationPacket* pPacket) {
	// Check packet size.
	static const std::size_t EXPECTED_SIZE = sizeof(PlayerPositionUpdateClient_Struct);
	if (pPacket->size != EXPECTED_SIZE && pPacket->size != EXPECTED_SIZE + 1) {
		Log::error("[Zone Client Connection] Wrong sized PlayerPositionUpdateClient_Struct, dropping connection.");
		// TODO: Drop connection!
		return;
	}

	PlayerPositionUpdateClient_Struct* payload = reinterpret_cast<PlayerPositionUpdateClient_Struct*>(pPacket->pBuffer);
	mZone->updateCharacterPosition(mCharacter, payload->x_pos, payload->y_pos, payload->z_pos, payload->heading);
}

void ZoneClientConnection::_handleSpawnAppearance(const EQApplicationPacket* pPacket) {
	// Check packet size.
	static const std::size_t EXPECTED_SIZE = sizeof(SpawnAppearance_Struct);
	if (pPacket->size != EXPECTED_SIZE && pPacket->size != EXPECTED_SIZE + 1) {
		Log::error("[Zone Client Connection] Wrong sized SpawnAppearance_Struct, dropping connection.");
		// TODO: Drop connection!
		return;
	}

	SpawnAppearance_Struct* payload = reinterpret_cast<SpawnAppearance_Struct*>(pPacket->pBuffer);
	const uint16 actionType = payload->type;
	const uint32 actionParameter = payload->parameter;

	// Ignore if spawn id does not match this characters ID.
	if (payload->spawn_id != mCharacter->getID()) {
		// Note: UF client sends spawn ID (0) and action type (51) every few seconds. Not sure why.
		return;
	}

	switch (actionType) {
		// Handle animation.
	case SpawnAppearanceTypes::Animation:
		switch (actionParameter) {
		case SpawnAppearanceAnimations::Standing:
			mCharacter->setStanding(true);
			break;
		case SpawnAppearanceAnimations::Freeze:
			break;
		case SpawnAppearanceAnimations::Looting:
			break;
		case SpawnAppearanceAnimations::Sitting:
			mCharacter->setStanding(false);
			break;
		case SpawnAppearanceAnimations::Crouch:
			// Crouch or Jump triggers this.
			break;
		case SpawnAppearanceAnimations::Death:
			break;
		default:
			std::stringstream ss;
			ss << "[Zone Client Connection] Got unexpected SpawnAppearanceTypes::Animation parameter : " << actionParameter;
			Log::info(ss.str());
			break;
		}
		// Handle anonymous / roleplay
	case SpawnAppearanceTypes::Anonymous:
		// TODO: Update other clients.
		// Not anonymous
		if (actionParameter == 0) {
			mCharacter->mProfile->mAnonymous = 0;
		}
		// Anonymous
		else if (actionParameter == 1) {
			mCharacter->mProfile->mAnonymous = 1;
		}
		// Roleplay
		else if (actionParameter == 2) {
			mCharacter->mProfile->mAnonymous = 2;
		}
		// Anything else is ignored.
		break;
		// Handle AFK
	case SpawnAppearanceTypes::AFK:
		// TODO: Update other clients.
		if (actionParameter == 0) {
			mCharacter->setAFK(false);
		}
		else if (actionParameter == 1) {
			mCharacter->setAFK(true);
		}
		// Anything else is ignored.
		break;
	case SpawnAppearanceTypes::ShowHelm:
		// TODO: Update other clients.
		if (actionParameter == 0) {
			mCharacter->setShowHelm(false);
		}
		else if (actionParameter == 1) {
			mCharacter->setShowHelm(true);
		}
		// Anything else is ignored.
		break;
		// Ignore!
	case SpawnAppearanceTypes::HP:
		break;
	case SpawnAppearanceTypes::Split:
		break;
	case SpawnAppearanceTypes::Die:
		break;
	case SpawnAppearanceTypes::DamageState:
		break;
	case SpawnAppearanceTypes::Sneak:
		break;
	case SpawnAppearanceTypes::Invisible:
		break;
	case SpawnAppearanceTypes::Size:
		break;
	case SpawnAppearanceTypes::Light:
		break;
	default:
		std::stringstream ss;
		ss << "[Zone Client Connection] Got unexpected SpawnAppearanceTypes : " << actionType;
		Log::error(ss.str());
		break;
	}
}

void ZoneClientConnection::_handleCamp(const EQApplicationPacket* pPacket) {
	mCharacter->startCamp();
}

enum CHANNEL : uint32 {
	CH_GUILD = 0, // /gu
	CH_GROUP = 2, // /g
	CH_SHOUT = 3, // /shou
	CH_AUCTION = 4, // /auc
	CH_OOC = 5, // /ooc
	CH_BROADCAST = 6, // ??
	CH_TELL = 7, // /t
	CH_SAY = 8, // /say
	CH_RAID = 15, // /rs
	CH_UCS = 20, // Not sure yet.
	CH_EMOTE = 22 // UF+

};
void ZoneClientConnection::_handleChannelMessage(const EQApplicationPacket* pPacket) {
	// Check packet size.
	// NOTE: This packet size increases with message size.
	static const std::size_t EXPECTED_SIZE = sizeof(ChannelMessage_Struct);
	if (pPacket->size < EXPECTED_SIZE || pPacket->size > 661) { // TODO: Magic number. 661 is biggest UF can send legitimately.
		Log::error("[Zone Client Connection] Wrong sized ChannelMessage_Struct, dropping connection.");
		dropConnection();
		return;
	}

	ChannelMessage_Struct* payload = reinterpret_cast<ChannelMessage_Struct*>(pPacket->pBuffer);
	uint32 channel = payload->chan_num;
	std::string message = payload->message; // TODO: I am sure if this is completely safe.

	// Check message size.
	if (message.empty()) {
		Log::error("[Zone Client Connection] Got empty message, dropping connection.");
		dropConnection();
		return;
	}

	switch (channel) {
	case CHANNEL::CH_GUILD:
		break;
	case CHANNEL::CH_GROUP:
		break;
	case CHANNEL::CH_SHOUT:
		break;
	case CHANNEL::CH_OOC:
		break;
	case CHANNEL::CH_BROADCAST:
		break;
	case CHANNEL::CH_TELL:
		break;
	case CHANNEL::CH_SAY:
		// Check whether user has entered a command.
		if (message[0] == COMMAND_TOKEN) {
			mCommandHandler->command(mCharacter, message);
			break;
		}
		break;
	case CHANNEL::CH_RAID:
		break;
	case CHANNEL::CH_UCS:
		break;
	case CHANNEL::CH_EMOTE:
		break;
	default:
		std::stringstream ss;
		ss << "[Zone Client Connection] " << __FUNCTION__ << " Got unexpected channel number: " << channel;
		Log::error(ss.str());
		break;
	}
}

void ZoneClientConnection::sendPosition() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
	PlayerPositionUpdateServer_Struct* payload = reinterpret_cast<PlayerPositionUpdateServer_Struct*>(outPacket->pBuffer);
	memset(payload, 0xff, sizeof(PlayerPositionUpdateServer_Struct));
	payload->spawn_id = mCharacter->getSpawnID();
	payload->x_pos = FloatToEQ19(mCharacter->getX());
	payload->y_pos = FloatToEQ19(mCharacter->getY());
	payload->z_pos = FloatToEQ19(mCharacter->getZ());
	payload->delta_x = NewFloatToEQ13(0);
	payload->delta_y = NewFloatToEQ13(0);
	payload->delta_z = NewFloatToEQ13(0);
	payload->heading = FloatToEQ19(mCharacter->getHeading());
	payload->animation = 0;
	payload->delta_heading = NewFloatToEQ13(0);
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

void ZoneClientConnection::sendMessage(uint32 pType, std::string pMessage) {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_SpecialMesg, sizeof(SpecialMesg_Struct)+pMessage.length());
	SpecialMesg_Struct* payload = reinterpret_cast<SpecialMesg_Struct*>(outPacket->pBuffer);
	payload->header[0] = 0x00; // Header used for #emote style messages..
	payload->header[1] = 0x00; // Play around with these to see other types
	payload->header[2] = 0x00;
	payload->msg_type = pType;
	strcpy(payload->message, pMessage.c_str());
	mStreamInterface->QueuePacket(outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleLogOut(const EQApplicationPacket* pPacket) {

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
	CancelTrade_Struct* ct = (CancelTrade_Struct*)outapp->pBuffer;
	ct->fromid = mCharacter->getSpawnID();
	ct->action = groupActUpdate;
	mStreamInterface->FastQueuePacket(&outapp);

	_sendPreLogOutReply();
	_sendLogOutReply();

	// Tell Zone.
	mZone->notifyCharacterLogOut(mCharacter);

	// Note: The client will hang until this is closed.
	mStreamInterface->Close();
}

void ZoneClientConnection::_sendLogOutReply() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_LogoutReply);
	mStreamInterface->FastQueuePacket(&outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_sendPreLogOutReply() {
	EQApplicationPacket* outPacket = new EQApplicationPacket(OP_PreLogoutReply);
	mStreamInterface->FastQueuePacket(&outPacket);
	safe_delete(outPacket);
}

void ZoneClientConnection::_handleDeleteSpawn(const EQApplicationPacket* pPacket)
{
	_sendLogOutReply();
}
