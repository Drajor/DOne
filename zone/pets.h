/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY except by those people which sell it, which
are required to give you total support for your newly bought product;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef PETS_H
#define PETS_H

#include "../common/types.h"

static const uint32 PET_BACKOFF = 1;
static const uint32 PET_GETLOST = 2;
static const uint32 PET_HEALTHREPORT = 4;
static const uint32 PET_GUARDHERE = 5;
static const uint32 PET_GUARDME = 6;
static const uint32 PET_ATTACK = 7;
static const uint32 PET_FOLLOWME = 8;
static const uint32 PET_SITDOWN = 9;
static const uint32 PET_STANDUP = 10;
static const uint32 PET_TAUNT = 11;
static const uint32 PET_HOLD = 12;
static const uint32 PET_NOTAUNT = 14;
static const uint32 PET_LEADER = 16;
static const uint32 PET_SLUMBER = 17;
static const uint32 PET_NOCAST = 18;
static const uint32 PET_FOCUS = 19;
static const uint32 PET_FOCUS_ON = 25;
static const uint32 PET_FOCUS_OFF = 26;
static const uint32 PET_HOLD_ON = 27;
static const uint32 PET_HOLD_OFF = 28;

class Pet : public NPC {
public:
	Pet(NPCType *type_data, Mob *owner, PetType type, uint16 spell_id, int16 power);

};

#endif

