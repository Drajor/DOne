#ifndef UCS_H
#define UCS_H

#include "../common/types.h"

class EmuTCPConnection;
class ServerPacket;

class UCSConnection {
public:
	UCSConnection();
	void setConnection(EmuTCPConnection* pConnection);
	bool update();
	bool sendPacket(ServerPacket* pPacket);
	void disconnect();
private:
	uint32 getIP() const;
	EmuTCPConnection* mTCPConnection;
	bool mAuthenticated;
};

#endif