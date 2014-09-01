#include "UCS.h"
#include "Utility.h"
#include "Settings.h"
#include "UCSClientConnection.h"

#include "../common/EQStreamFactory.h"

bool UCS::initialise() {
	Log::status("[UCS] Initialising.");
	EXPECTED_BOOL(mInitialised == false);

	mStreamFactory = new EQStreamFactory(ChatStream, Settings::getUCSPort(), 45000);
	mOpcodeManagaer = new RegularOpcodeManager();

	EXPECTED_BOOL(mOpcodeManagaer->LoadOpcodes("mail_opcodes.conf"));
	EXPECTED_BOOL(mStreamFactory->Open());
	Log::info("[UCS] Listening on port " + std::to_string(Settings::getUCSPort()));

	mInitialised = true;
	Log::status("[UCS] Initialised.");
	return true;
}

void UCS::update() {
	// Check for incoming connections.
	EQStream* incomingStream = nullptr;
	while ((incomingStream = mStreamFactory->Pop())) {
		mConnections.push_back(new UCSClientConnection(incomingStream));
	}

	// Update existing connections.
	for (auto i : mConnections)
		i->update();
}
