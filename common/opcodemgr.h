/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#ifndef OPCODE_MANAGER_H
#define OPCODE_MANAGER_H

#include "types.h"
#include "Mutex.h"
#include "emu_opcodes.h"

#include <map>

class OpcodeManager {
public:
	OpcodeManager();
	virtual ~OpcodeManager() {}

	virtual bool LoadOpcodes(const char *filename, bool report_errors = false) = 0;

	virtual uint16 EmuToEQ(const EmuOpcode emu_op) = 0;
	virtual EmuOpcode EQToEmu(const uint16 eq_op) = 0;

	static const char *EmuToName(const EmuOpcode emu_op);
	const char *EQToName(const uint16 emu_op);

	//This has to be public for stupid visual studio
	class OpcodeSetStrategy {
	public:
		virtual ~OpcodeSetStrategy() {}	//shut up compiler!
		virtual void Set(EmuOpcode emu_op, uint16 eq_op) = 0;
	};

protected:
	bool loaded; //true if all opcodes loaded
	Mutex MOpcodes; //this only protects the local machine
					//in a shared manager, this dosent protect others

	static bool LoadOpcodesFile(const char *filename, OpcodeSetStrategy *s, bool report_errors);
};

//keeps opcodes in regular heap memory
class RegularOpcodeManager : public OpcodeManager {
public:
	RegularOpcodeManager();
	virtual ~RegularOpcodeManager();

	virtual bool LoadOpcodes(const char *filename, bool report_errors = false);

	virtual uint16 EmuToEQ(const EmuOpcode emu_op);
	virtual EmuOpcode EQToEmu(const uint16 eq_op);

	//implement our editing interface
	virtual void SetOpcode(EmuOpcode emu_op, uint16 eq_op);

protected:
	class NormalMemStrategy : public OpcodeManager::OpcodeSetStrategy {
	public:
		virtual ~NormalMemStrategy() {} //shut up compiler!
		RegularOpcodeManager *it;
		void Set(EmuOpcode emu_op, uint16 eq_op);
	};
	friend class NormalMemStrategy;

	uint16 *emu_to_eq;
	EmuOpcode *eq_to_emu;
	uint32 EQOpcodeCount;
	uint32 EmuOpcodeCount;
};

#endif

