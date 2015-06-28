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
#ifndef BASEPACKET_H_
#define BASEPACKET_H_

#include "types.h"
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef WIN32
	#define _WINSOCKAPI_
	#include <time.h>
	#include <windows.h>
	#include <winsock2.h>
#else
	#include <sys/time.h>
	#include <netinet/in.h>
#endif

class BasePacket {
public:
	unsigned char *pBuffer;
	uint32 size;
	
	uint32 mSourceIP = 0;
	uint32 mDestinationIP = 0;
	uint16 mSourcePort = 0;
	uint16 mDestinationPort = 0;
	timeval timestamp;

	virtual void build_raw_header_dump(char *buffer, uint16 seq=0xffff) const;
	virtual void build_header_dump(char *buffer) const;
	virtual void DumpRawHeader(uint16 seq=0xffff, FILE *to = stdout) const;
	virtual void DumpRawHeaderNoTime(uint16 seq=0xffff, FILE *to = stdout) const;

	void setTimeInfo(uint32 ts_sec, uint32 ts_usec) { timestamp.tv_sec=ts_sec; timestamp.tv_usec=ts_usec; }
	void copyInfo(const BasePacket *p) { mSourceIP=p->mSourceIP; mSourcePort=p->mSourcePort; mDestinationIP=p->mDestinationIP; mDestinationPort=p->mDestinationPort; timestamp.tv_sec=p->timestamp.tv_sec; timestamp.tv_usec=p->timestamp.tv_usec; }

protected:
	virtual ~BasePacket();
	BasePacket() { pBuffer=nullptr; size=0; /*_wpos = 0; _rpos = 0;*/ }
	BasePacket(const unsigned char *buf, const uint32 len);
};

extern void DumpPacketHex(const BasePacket* app);
extern void DumpPacketAscii(const BasePacket* app);
extern void DumpPacketBin(const BasePacket* app);

#endif /*BASEPACKET_H_*/

