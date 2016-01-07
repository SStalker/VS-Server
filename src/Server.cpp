#include "Server.h"

	
/*	WSServer::WSServer(){

	}

	void WSServer::run(){}
	void WSServer::on_open(connection_hdl hdl){}
	void WSServer::on_close(connection_hdl hdl){}
	void WSServer::on_message(connection_hdl hdl, server::message_ptr msg){}
	*/

WSServer::WSServer() : m_next_sessionid(1) {
        m_server.init_asio(&ios);
                
        m_server.set_open_handler(bind(&WSServer::on_open,this,::_1));
        m_server.set_close_handler(bind(&WSServer::on_close,this,::_1));
        m_server.set_message_handler(bind(&WSServer::on_message,this,::_1,::_2));

        tls.init_asio(&ios);
        tls.set_message_handler(bind(&WSServer::on_message,this,::_1,::_2));
        
        // TLS endpoint has an extra handler for the tls init
        tls.set_tls_init_handler(bind(&WSServer::on_tls_init,this, ::_1));
    }
    
    void WSServer::on_open(connection_hdl hdl) {
        connection_data data;
        
        data.sessionid = m_next_sessionid++;
        data.name = "";
        
        m_connections[hdl] = data;
    }
    
    void WSServer::on_close(connection_hdl hdl) {
        connection_data& data = get_data_from_hdl(hdl);
        
        std::cout << "Closing connection " << data.name 
                  << " with sessionid " << data.sessionid << std::endl;
        
        m_connections.erase(hdl);
    }
    
    void WSServer::on_message(connection_hdl hdl, server::message_ptr msg) {
        connection_data& data = get_data_from_hdl(hdl);
        
        if (data.name == "") {
            data.name = msg->get_payload();
            std::cout << "Setting name of connection with sessionid " 
                      << data.sessionid << " to " << data.name << std::endl;
        } else {
            std::cout << "Got a message from connection " << data.name 
                      << " with sessionid " << data.sessionid << std::endl;
        }

        
        std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

        try {
            rapidjson::Document document;
            document.Parse(msg->get_payload().c_str());

            rapidjson::Value::ConstMemberIterator itr = document.FindMember("message");
            
            std::stringstream ss;
            ss << "Nachricht lautete: " << document["data"].GetString();
            std::string val = ss.str();
            document["data"].SetString(val.c_str(), val.length());


            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

            document.Accept(writer);
            const char* output = buffer.GetString();

            m_server.send(hdl, output, msg->get_opcode());

        } catch (const websocketpp::lib::error_code& e) {
            std::cout << "Echo failed because: " << e
                      << "(" << e.message() << ")" << std::endl;
        }
    }

    // No change to TLS init methods from echo_server_tls
    std::string WSServer::get_password() {
        return "testtest";
    }

    context_ptr WSServer::on_tls_init(websocketpp::connection_hdl hdl) {
    
        std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
        context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use);
            ctx->set_password_callback(bind(&WSServer::get_password, this));
            ctx->use_certificate_chain_file("server.pem");
            ctx->use_private_key_file("server.pem", boost::asio::ssl::context::pem);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        return ctx;
}
    
    connection_data& WSServer::get_data_from_hdl(connection_hdl hdl) {
        auto it = m_connections.find(hdl);
        
        if (it == m_connections.end()) {
            // this connection is not in the list. This really shouldn't happen
            // and probably means something else is wrong.
            throw std::invalid_argument("No data avaliable for session");
        }
        
        return it->second;
    }
    
    void WSServer::run(uint16_t port_plain, uint16_t port_tls) {
        m_server.listen(port_plain);
        m_server.start_accept();

        tls.listen(port_tls);
        tls.start_accept();

        ios.run();
    }