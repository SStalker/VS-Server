#include <iostream>
#include <list>
#include <pqxx/pqxx>
#include <rapidjson/document.h>

using namespace pqxx;


class Database{

public:
	Database();
    ~Database();
	bool connect();
    void registerClient(rapidjson::Document &doc);
    void loginClient(rapidjson::Document &doc);
    void logoutClient(rapidjson::Document &doc);
	void addFriend(rapidjson::Document &doc);
	void removeFriend(rapidjson::Document &doc);
	void createChatroom(rapidjson::Document &doc);
	void removeChatroom(rapidjson::Document &doc);
	void chatroomRemoveClient(rapidjson::Document &doc);
	void chatroomAddClient(rapidjson::Document &doc);
	void chatroomNewMessage(rapidjson::Document &doc);
	void newMessage(rapidjson::Document &doc);
	void setStatus(rapidjson::Document &doc);
	void setOperator(rapidjson::Document &doc);
    std::list<std::string> getNewFriendshipRequests();
    std::list<std::string> getNewOfflineMessages();

private:
	connection *conn;
    nontransaction *w;
};
