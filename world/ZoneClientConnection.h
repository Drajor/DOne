#pragma once

class EQStreamInterface;
class EQApplicationPacket;
class Zone;

class ZoneClientConnection {
public:
	ZoneClientConnection(EQStreamInterface* pStreamInterface, Zone* pZone);
	~ZoneClientConnection();
	void update();
	bool _handlePacket(const EQApplicationPacket* pPacket);
	void _handleZoneEntry(const EQApplicationPacket* pPacket);
private:
	EQStreamInterface* mStreamInterface;
	Zone* mZone;
};