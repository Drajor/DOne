#pragma once

#include "Types.h"

static const u32 MaxGroupMembers = 6;

namespace GroupRole {
	enum : u32 {
		MainTank = 1,
		MainAssist = 2,
		Puller = 3,
	};
}