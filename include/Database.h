#include <iostream>
#include <sstream>
#include <list>
#include <regex>
#include <pqxx/pqxx>
#include <rapidjson/document.h>

using namespace pqxx;

struct foundUsers {
    std::string email;
    std::string nickname;
};

class Database{

public:
	Database();
    ~Database();
	bool connect();
    bool registerClient(rapidjson::Document &doc);
    bool loginClient(rapidjson::Document &doc);
    bool logoutClient(rapidjson::Document &doc);
    void addFriendToChat(std::string cid, std::string uid);
    bool friendRequest(int uid, int fid);
	void removeFriend(rapidjson::Document &doc);
	void createChatroom(rapidjson::Document &doc);
	void removeChatroom(rapidjson::Document &doc);
	void chatroomRemoveClient(rapidjson::Document &doc);
	void chatroomAddClient(rapidjson::Document &doc);
	void chatroomNewMessage(rapidjson::Document &doc);
	void newMessage(rapidjson::Document &doc);
	void setStatus(rapidjson::Document &doc);
	void setOperator(rapidjson::Document &doc);
    std::list<foundUsers> getSearchedUsers(std::string search, int uid);
    std::list<std::string> getNewFriendshipRequests(rapidjson::Document &doc);
    std::list<std::string> getNewOfflineMessages(rapidjson::Document &doc);
    std::list<std::string> getFriendlist(rapidjson::Document &doc);
    int getUserIDFromSession(int sessionid);
    int getSessionIDFromUser(int sessionid);
    bool userOnline(int id);
    foundUsers getPubClientInformation(int id);
    void setSessionID(int uid, int sessionid);

    std::string getUserID(std::string email);

private:
	connection *conn;
    nontransaction *w;
};
