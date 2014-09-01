#pragma once

#include "Constants.h"

class EQStream;
class EQApplicationPacket;

class UCSClientConnection {
public:
	UCSClientConnection(EQStream* pStream);
	void update();
private:
	void _handlePacket(EQApplicationPacket* pPacket);
	void _handleMailLogin(EQApplicationPacket* pPacket);
	void _handleMail(EQApplicationPacket* pPacket);
	EQStream* mStream = nullptr;
};