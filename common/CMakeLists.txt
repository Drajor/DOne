CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(common_sources
	BasePacket.cpp
	crash.cpp
	CRC16.cpp
	crc32.cpp
	debug.cpp
	emu_opcodes.cpp
	EmuTCPConnection.cpp
	EmuTCPServer.cpp
	eqemu_exception.cpp
	EQPacket.cpp
	EQStream.cpp
	EQStreamFactory.cpp
	EQStreamIdent.cpp
	EQStreamProxy.cpp
	logsys.cpp
	logsys_eqemu.cpp
	misc.cpp
	MiscFunctions.cpp
	Mutex.cpp
	opcodemgr.cpp
	packet_dump.cpp
	packet_dump_file.cpp
	packet_functions.cpp
    StringUtil.cpp
	StructStrategy.cpp
	TCPConnection.cpp
	TCPServer.cpp
	timeoutmgr.cpp
	timer.cpp
	platform.cpp
	patches/Underfoot.cpp
	SocketLib/Base64.cpp
	SocketLib/File.cpp
	SocketLib/HttpdCookies.cpp
	SocketLib/HttpdForm.cpp
	SocketLib/HttpdSocket.cpp
	SocketLib/HTTPSocket.cpp
	SocketLib/MemFile.cpp
	SocketLib/Mime.cpp
	SocketLib/Parse.cpp
	SocketLib/socket_include.cpp
	SocketLib/Utility.cpp
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
	debug.h
	emu_opcodes.h
	emu_oplist.h
	EmuTCPConnection.h
	EmuTCPServer.h
	eq_constants.h
	eq_packet_structs.h
	eqemu_exception.h
	EQPacket.h
	EQStream.h
	EQStreamFactory.h
	EQStreamIdent.h
	EQStreamIntf.h
	EQStreamLocator.h
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
	version.h
	platform.h
	patches/SSDeclare.h
	patches/SSDefine.h
	patches/SSRegister.h
	patches/Underfoot.h
	patches/Underfoot_ops.h
	patches/Underfoot_structs.h
	SocketLib/Base64.h
	SocketLib/File.h
	SocketLib/HttpdCookies.h
	SocketLib/HttpdForm.h
	SocketLib/HttpdSocket.h
	SocketLib/HTTPSocket.h
	SocketLib/IFile.h
	SocketLib/MemFile.h
	SocketLib/Mime.h
	SocketLib/Parse.h
	SocketLib/socket_include.h
	SocketLib/Utility.h
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

SOURCE_GROUP(SocketLib FILES
	SocketLib/Base64.h
	SocketLib/File.h
	SocketLib/HttpdCookies.h
	SocketLib/HttpdForm.h
	SocketLib/HttpdSocket.h
	SocketLib/HTTPSocket.h
	SocketLib/IFile.h
	SocketLib/MemFile.h
	SocketLib/Mime.h
	SocketLib/Parse.h
	SocketLib/socket_include.h
	SocketLib/Utility.h
	SocketLib/Base64.cpp
	SocketLib/File.cpp
	SocketLib/HttpdCookies.cpp
	SocketLib/HttpdForm.cpp
	SocketLib/HttpdSocket.cpp
	SocketLib/HTTPSocket.cpp
	SocketLib/MemFile.cpp
	SocketLib/Mime.cpp
	SocketLib/Parse.cpp
	SocketLib/socket_include.cpp
	SocketLib/Utility.cpp
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

IF(UNIX)
	ADD_DEFINITIONS(-fPIC)
	SET_SOURCE_FILES_PROPERTIES("patches/SoD.cpp" "patches/SoF.cpp" "patches/RoF.cpp" "patches/Underfoot.cpp" PROPERTIES COMPILE_FLAGS -O0)
ENDIF(UNIX)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/Bin)
