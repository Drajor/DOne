#pragma once

namespace AccountStatus {
	enum : i32 {
		Banned = -2,
		Suspended = -1,
		Default = 0,
		BypassLock = 20,
	};
}

namespace Limits { namespace Account {

	//static const auto MAX_NUM_CHARACTERS = 18;
	//static const auto MAX_EQUIPMENT_SLOTS = 9;

} // namespace Account.
} // namespace Limits.