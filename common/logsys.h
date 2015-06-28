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
#ifndef LOGSYS_H_
#define LOGSYS_H_

#define LOG_CATEGORY(category) LOG_ ##category ,
typedef enum {
	#include "logtypes.h"
	NUMBER_OF_LOG_CATEGORIES
} LogCategory;


#define LOG_TYPE(category, type, enabled) category##__##type ,
typedef enum {
	#include "logtypes.h"
	NUMBER_OF_LOG_TYPES
} LogType;

inline void _log(int, const char *, ...) {}
inline void _raw(int, const char *, ...) {}
inline void _hex(int, const char *, ...) {}

#endif

