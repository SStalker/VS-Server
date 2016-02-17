/*#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class WSServer {

public:
	WSServer();
	void run();
	void on_open(connection_hdl hdl);
	void on_close(connection_hdl hdl);
	void on_message(connection_hdl hdl, server::message_ptr msg);
};*/

#include <iostream>
#include <map>
#include <exception>
#include <vector>
#include <fstream>
#include <sstream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

#include <rapidjson/document.h>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "Database.h"

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server_tls;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace std;

// type of the ssl context pointer is long so alias it
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

struct connection_data {
    int sessionid;
    string name;
};

class WSServer {
public:
	WSServer();
	void on_open(connection_hdl hdl);
	void on_close(connection_hdl hdl);
	void on_message(connection_hdl hdl, server::message_ptr msg);
	string get_password();
	context_ptr on_tls_init(websocketpp::connection_hdl hdl);
	connection_data& get_data_from_hdl(connection_hdl hdl);
	void run(uint16_t port_plain, uint16_t port_tls);
    void stopServer();

    connection_hdl get_hdl_from_session(int sessionID);
private:
    typedef map<connection_hdl,connection_data,owner_less<connection_hdl>> con_list;

    int m_next_sessionid;
    server m_server;
    server_tls tls;
    con_list m_connections;
    boost::asio::io_service ios;
    Database db;

    const string response(string key, string responsetype, vector<pair<string, string>> response);
    const string responseSearchedList(string key, string responsetype, list<foundUsers> responseList);
    const string sendFriendlist(string key, string responsetype, list<friendListUser> responseList);
    const string sendChats(string key, string responsetype, list<chatList> chats);
    const string createError(const exception& e, string from );
    const string readTemplate(string filename);
    const string buildTemplateJson(string filename, string responsetype);
};
