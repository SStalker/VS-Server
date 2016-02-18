#include <iostream>
#include <sstream>
#include <list>
#include <regex>
#include <pqxx/pqxx>
#include <vector>
#include <rapidjson/document.h>

using namespace pqxx;

struct foundUsers {
    std::string email;
    std::string nickname;
};

struct friendListUser{
    int id;
    std::string nickname;
    std::string email;
    bool online;
    std::string firstname;
    std::string lastname;
    std::string birthday;
    std::string imageb64;
    std::string cid;
    std::string chatname;
    std::string chatstatusmsg;
};

struct messageContainer{
    int id;
    std::string messageFrom;
    int messageTo ;
    std::string message;
    std::string created_at;
};

struct chatList{
    int id;
    std::string name;
    std::string status;
    std::vector<messageContainer> messages;
};

class Database{

public:
	Database();
    ~Database();
	bool connect();
    bool registerClient(rapidjson::Document &doc);
    bool loginClient(rapidjson::Document &doc);
    void logoutClient(int uid);
    void addFriendToChat(std::string cid, std::string uid);
    bool friendRequest(int uid, int fid);
    bool removeRequest(int uid, int fid);
    void removeFriend(int uid, int fid);
	void createChatroom(rapidjson::Document &doc);
	void removeChatroom(rapidjson::Document &doc);
	void chatroomRemoveClient(rapidjson::Document &doc);
	void chatroomAddClient(rapidjson::Document &doc);
	void chatroomNewMessage(rapidjson::Document &doc);
    int newMessage(int uid, int cid, std::string msg, bool transmitted);
    messageContainer getMessage(int mid);
    void setMessageTransmitted(int mid, bool transmitted);
	void setStatus(rapidjson::Document &doc);
	void setOperator(rapidjson::Document &doc);
    std::list<foundUsers> getSearchedUsers(std::string search, int uid);
    std::list<std::string> getNewFriendshipRequests(rapidjson::Document &doc);
    std::list<std::string> getNewOfflineMessages(rapidjson::Document &doc);
    std::list<friendListUser> getFriendlist(int uid);
    friendListUser getFriendListUserFromID(int uid, int cid);
    std::vector<int> getFrindIds(int uid);
    int getUserIDFromSession(int id);
    int getSessionIDFromUser(int sessionid);
    bool userOnline(int id);
    foundUsers getPubClientInformation(int id);
    void setSessionID(int uid, int sessionid);
    void setFriendRequestTransmition(int uid, int fid);
    void setFriendRequestTransmition(int uid);
    void acceptFriendRequest(int uid, int fid);
    std::list<foundUsers> getFriendRequests(int uid);
    int createChat(int uid, int friendID);
    std::list<chatList>getChatsByUid(int uid);
    chatList getChatById(int cid);
    std::string getNickname(int uid);
    std::string getEmail(int uid);


    std::string getUserID(std::string email);
    std::vector<int> getUserID(int uid, int cid);
    void getUserDataFrom(std::string uid, std::map<std::string, std::string> &map);

    bool belongsChatIDToUser(int cid);

    void getAllOnlineUsersOfChatroom(int cid, std::vector<int> &list);
private:
	connection *conn;
    nontransaction *w;
};
