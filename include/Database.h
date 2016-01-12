#include <iostream>
#include <pqxx/pqxx>

using namespace pqxx;


class Database{

public:
	Database();
    ~Database();
	bool connect();
	void registerClient();
	void addFriend();
	void removeFriend();
	void createChatroom();
	void removeChatroom();
	void chatroomRemoveClient();
	void chatroomAddClient();
	void chatroomNewMessage();
	void newMessage();
	void setStatus();
	void setOperator();

private:
	connection *conn;
    work *w;
};
