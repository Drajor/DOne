#pragma once

class EQStreamInterface;
class Zone;

class ZoneClientConnection {
public:
	ZoneClientConnection(EQStreamInterface* pStreamInterface, Zone* pZone);
	~ZoneClientConnection();
	void update();
private:
	const EQStreamInterface* mStreamInterface;
	const Zone* mZone;
};