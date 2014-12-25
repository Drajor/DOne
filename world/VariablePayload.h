#pragma once

#include "Types.h"
#include "../common/EQPacket.h"

namespace Payload {

	template <typename T, EmuOpcode OpCode>
	struct VariableT {
		virtual T* convert(const EQApplicationPacket* pPacket) = 0;
	};

	struct ChannelMessage : public VariableT<ChannelMessage, OP_ChannelMessage> {
		String mSender;
		String mTarget;
		u32 mChannelID = 0;
	};
}