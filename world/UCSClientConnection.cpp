#include "UCSClientConnection.h"
#include "Utility.h"
#include "LogSystem.h"

#include "../common/EQStream.h"

UCSClientConnection::UCSClientConnection(EQStream* pStream) : mStream(pStream) {
	EXPECTED(mStream);
}

void UCSClientConnection::update() {
	// Process incoming packets.
	auto packet = mStream->PopPacket();
	while (packet) {
		_handlePacket(packet);
		safe_delete(packet);
		packet = mStream->PopPacket();
	}
}

void UCSClientConnection::_handlePacket(EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);

	const EmuOpcode opcode = pPacket->GetOpcode();
	if (opcode == 0) { return; } // [UF] Sends this once
	if (opcode == OP_MailLogin) { _handleMailLogin(pPacket); return; }
	if (opcode == OP_Mail) { _handleMail(pPacket); return; }

	Log::error("[UCS Client Connection] Got unknown opcode: " + std::to_string(opcode));
}

void UCSClientConnection::_handleMailLogin(EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	Log::error("Got OP_MailLogin");
}

void UCSClientConnection::_handleMail(EQApplicationPacket* pPacket) {
	EXPECTED(pPacket);
	Log::error("Got OP_Mail");
}
