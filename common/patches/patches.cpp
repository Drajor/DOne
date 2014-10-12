
#include "../debug.h"
#include "patches.h"

#include "Underfoot.h"

void RegisterAllPatches(EQStreamIdentifier &into) {
	Underfoot::Register(into);
}