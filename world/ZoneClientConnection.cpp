#include "ZoneClientConnection.h"
#include "../common/EQStreamIntf.h"
#include "../common/emu_opcodes.h"
#include "../common/EQPacket.h"
#include "../common/eq_packet_structs.h"

#include "Utility.h"
#include <sstream>

ZoneClientConnection::ZoneClientConnection(EQStreamInterface* pStreamInterface, Zone* pZone) :
mStreamInterface(pStreamInterface),
mZone(pZone)
{

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

bool ZoneClientConnection::_handlePacket(const EQApplicationPacket* pPacket) {	
	EmuOpcode opcode = pPacket->GetOpcode();
	//std::stringstream ss;
	//ss << "Packet: " << opcode;
	//Utility::print(ss.str());
	switch (opcode) {
	case OP_AckPacket:
		Utility::print("OP_AckPacket");
		break;
	case OP_ZoneEntry:
		Utility::print("OP_ZoneEntry");
		_handleZoneEntry(pPacket);
		break;
	case OP_SetServerFilter:
		Utility::print("OP_SetServerFilter");
		break;
	case OP_SendAATable:
		Utility::print("OP_SendAATable");
		break;
	case OP_ReqClientSpawn:
		Utility::print("OP_ReqClientSpawn");
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
		Utility::print("OP_SpawnAppearance");
		break;
	case OP_WearChange:
		Utility::print("OP_WearChange");
		break;
	case OP_ClientUpdate:
		Utility::print("OP_ClientUpdate");
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
	default:
		Utility::print("UNKNOWN PACKET");
		break;
	}
	return true;
}

void ZoneClientConnection::_handleZoneEntry(const EQApplicationPacket* pPacket) {
	if (pPacket->size != sizeof(ClientZoneEntry_Struct)) {

		return;
	}
		
	ClientZoneEntry_Struct* payload = (ClientZoneEntry_Struct*)pPacket->pBuffer;
}
