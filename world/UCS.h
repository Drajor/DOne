#pragma once

#include <list>

class EQStreamFactory;
class OpcodeManager;
class UCSClientConnection;

class UCS {
public:
	const bool initialise();
	void update();

private:

	bool mInitialised = false;
	std::list<UCSClientConnection*> mConnections;
	EQStreamFactory* mStreamFactory = nullptr;
	OpcodeManager* mOpcodeManagaer = nullptr;

};