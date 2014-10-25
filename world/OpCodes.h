#pragma once

namespace Raw {
	enum {
		OP_LootItem_ResponseRequest = 1264,
		/*
		Size = 164
		Seen after client sends OP_LootItem and server has not yet responded.
		The client sends this when ever the user tries to perform another actions.
		It appears as a 'UI lockup'.
		*/
	};
	
}