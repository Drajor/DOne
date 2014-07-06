#ifndef LOGINSERVERLIST_H_
#define LOGINSERVERLIST_H_

#include "../common/servertalk.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/queue.h"
#include "../common/eq_packet_structs.h"
#include "../common/Mutex.h"
#include "../common/EmuTCPConnection.h"

#ifdef _WINDOWS
	void AutoInitLoginServer(void *tmp);
#else
	void *AutoInitLoginServer(void *tmp);
#endif

class LoginServer;

class LoginServerList{
public:
	LoginServerList();
	~LoginServerList();

	void	Add(const char*, uint16, const char*, const char*);
	void	InitLoginServer();

	bool	Process();
	bool	SendStatus();

	bool	SendPacket(ServerPacket *pack);
	bool	Connected();
	bool	AllConnected();

protected:
	LinkedList<LoginServer*> list;
};




#endif /*LOGINSERVERLIST_H_*/
