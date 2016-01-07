#include <list>
#include <string>

#include "Server.h"

class Client{

public:
	Client(WSServer &server);
	void login(std::string name, std::string password);
	void logout();
	void sendMessage(int from, int to, std::string msg);
	void addFriend(int friend_id);
	void removeFriend(int friend_id);
	void blockClient(int client_id);
	void changeStatus(std::string status);
	void decodeBase64(std::string code);

private:

	WSServer server;
	int session;
	std::string status;
	bool isAdmin;
	std::list<Client> friendlist;
	std::list<Client> blocklist;
};