#ifndef LOGINSERVERLIST_H_
#define LOGINSERVERLIST_H_

#include "../common/types.h"
#include <list>

#ifdef _WINDOWS
	void AutoInitLoginServer(void *tmp);
#else
	void *AutoInitLoginServer(void *tmp);
#endif

class LoginServer;
class ServerPacket;

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
	std::list<LoginServer*> mLoginServers;
};




#endif /*LOGINSERVERLIST_H_*/
