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

// type of the ssl context pointer is long so alias it
typedef websocketpp::lib::shared_ptr<boost::asio::ssl::context> context_ptr;

struct connection_data {
    int sessionid;
    std::string name;
};

class WSServer {
public:
	WSServer();
	void on_open(connection_hdl hdl);
	void on_close(connection_hdl hdl);
	void on_message(connection_hdl hdl, server::message_ptr msg);
	std::string get_password();
	context_ptr on_tls_init(websocketpp::connection_hdl hdl);
	connection_data& get_data_from_hdl(connection_hdl hdl);
	void run(uint16_t port_plain, uint16_t port_tls);
    void stopServer();

private:
    typedef std::map<connection_hdl,connection_data,std::owner_less<connection_hdl>> con_list;

    int m_next_sessionid;
    server m_server;
    server_tls tls;
    con_list m_connections;
    boost::asio::io_service ios;
    Database db;

    const std::string response(std::string key, std::string responsetype, std::vector<std::pair<std::string, std::string>> response);
    const std::string responseSearchedList(std::string key, std::string responsetype, std::list<foundUsers> responseList);
    const std::string createError(const std::exception& e, std::string from );
    const std::string readTemplate(std::string filename);
    const std::string buildTemplateJson(std::string filename, std::string responsetype);
};
