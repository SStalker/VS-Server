#include <list>

#include "Client.h"

class Chatroom {

public:
	Chatroom();
	void addClient(int user_id);
	void removeClient(int user_id);

private:
	std::list<Client> userlist;
	int chatroom_id;
};