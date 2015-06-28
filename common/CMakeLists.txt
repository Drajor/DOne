CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(common_sources
	BasePacket.cpp
	crash.cpp
	CRC16.cpp
	crc32.cpp
	emu_opcodes.cpp
	EmuTCPConnection.cpp
	EmuTCPServer.cpp
	EQPacket.cpp
	EQStream.cpp
	EQStreamFactory.cpp
	EQStreamIdent.cpp
	EQStreamProxy.cpp
	misc.cpp
	MiscFunctions.cpp
	Mutex.cpp
	opcodemgr.cpp
	packet_dump.cpp
	packet_dump_file.cpp
	packet_functions.cpp
	StructStrategy.cpp
	TCPConnection.cpp
	TCPServer.cpp
	timeoutmgr.cpp
	timer.cpp
	platform.cpp
	patches/Underfoot.cpp
	StackWalker/StackWalker.cpp
	tinyxml/tinystr.cpp
	tinyxml/tinyxml.cpp
	tinyxml/tinyxmlerror.cpp
	tinyxml/tinyxmlparser.cpp
)

SET(common_headers
	BasePacket.h
	crash.h
	CRC16.h
	crc32.h
	emu_opcodes.h
	emu_oplist.h
	EmuTCPConnection.h
	EmuTCPServer.h
	eq_packet_structs.h
	EQPacket.h
	EQStream.h
	EQStreamFactory.h
	EQStreamIdent.h
	EQStreamIntf.h
	EQStreamProxy.h
	EQStreamType.h
	logsys.h
	logtypes.h
	mail_oplist.h
	misc.h
	MiscFunctions.h
	Mutex.h
	op_codes.h
	opcodemgr.h
	packet_dump.h
	packet_dump_file.h
	packet_functions.h
	queue.h
	servertalk.h
    StringUtil.h
	StructStrategy.h
	TCPConnection.h
	TCPServer.h
	timeoutmgr.h
	timer.h
	types.h
	platform.h
	patches/SSDeclare.h
	patches/SSDefine.h
	patches/SSRegister.h
	patches/Underfoot.h
	patches/Underfoot_ops.h
	patches/Underfoot_structs.h
	StackWalker/StackWalker.h
	tinyxml/tinystr.h
	tinyxml/tinyxml.h
)

SOURCE_GROUP(Patches FILES
	patches/SSDeclare.h
	patches/SSDefine.h
	patches/SSRegister.h
	patches/Underfoot.h
	patches/Underfoot_ops.h
	patches/Underfoot_structs.h
	patches/Underfoot.cpp
)

SOURCE_GROUP(StackWalker FILES
	StackWalker/StackWalker.h
	StackWalker/StackWalker.cpp
)

SOURCE_GROUP(TinyXML FILES
	tinyxml/tinystr.h
	tinyxml/tinyxml.h
	tinyxml/tinystr.cpp
	tinyxml/tinyxml.cpp
	tinyxml/tinyxmlerror.cpp
	tinyxml/tinyxmlparser.cpp
)

INCLUDE_DIRECTORIES(Patches SocketLib StackWalker TinyXML)

ADD_LIBRARY(Common ${common_sources} ${common_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
