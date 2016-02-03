#include "Server.h"
#include <fstream>

/*	WSServer::WSServer(){

	}

	void WSServer::run(){}
	void WSServer::on_open(connection_hdl hdl){}
	void WSServer::on_close(connection_hdl hdl){}
	void WSServer::on_message(connection_hdl hdl, server::message_ptr msg){}
*/

WSServer::WSServer() : m_next_sessionid(1) {

        //std::signal(SIGINT, WSServer::stopServer);

        // Check the db connection
        if(!db.connect())
            return;

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

            /* ToDo:-create validator
             *      -create multiple missings otpions
             */

            if(document.HasMember("request")){
                if(document["request"].IsString()){
                    if(strcmp(document["request"].GetString(),"registersite") == 0){
                        //deploy new template for webclient
                        try{
                            buildTemplateJson( "../webclient-templates/register.html", "registersite",hdl, msg);
                        }catch(const std::exception& e){
                            //do something in case of failure
                            createError(e, hdl, msg);
                        }
                    }else/**/
                    if(strcmp(document["request"].GetString(),"registration") == 0){
                        //register new client in database
                        try{
                            db.registerClient(document);
                        }catch(const pqxx::pqxx_exception& e){
                            //do something in case of failure
                            createError(e.base(), hdl, msg);
                        }
                    }else if(strcmp(document["request"].GetString(),"login") == 0){
                        //login client
                        try{
                            //std::cout << document["values"]["email"].GetString() << std::endl;
                            //db.loginClient(document);
                        }catch( const pqxx::pqxx_exception& e){
                            createError(e.base(), hdl, msg);
                        }
                    }
                }
            }

            //Echo
            std::stringstream ss;
            ss << "Der Nachrichten Typ ist: " << document["request"].GetString();
            std::string val = ss.str();
            document["request"].SetString(val.c_str(), val.length());

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

    void WSServer::stopServer(int signum){
        //websocketpp::endpoint::stop_listening();
        //close every connection here and thats ist

    }

    void WSServer::createError(const std::exception& e, connection_hdl& hdl, server::message_ptr msg){
        try{

            rapidjson::StringBuffer error_buffer;
            rapidjson::Writer<rapidjson::StringBuffer> error_writer(error_buffer);

            error_writer.StartObject();

            error_writer.String("response");
            error_writer.String("error");

            error_writer.String("values");
            error_writer.StartObject();

            error_writer.String("err_msg");
            error_writer.String(e.what());

            error_writer.EndObject();
            error_writer.EndObject();

            const std::string err_out = error_buffer.GetString();
            m_server.send(hdl, err_out, msg->get_opcode());

        }catch(const pqxx::pqxx_exception& e){
            std::cout << "An error occured while sending an error: " << e.base().what() << std::endl;
            throw e.base();
        }
        return;
    }


    void WSServer::buildTemplateJson(std::string filename, std::string responsetype, connection_hdl hdl, server::message_ptr msg){
        std::cout << "filename: " << filename << std::endl;
        try{
            std::string content;
            std::ifstream ifs(filename);
            if(ifs.is_open()){
                std::string content1((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                content=content1;
            }else{
                std::cout << "Could not open ifs" << std::endl;
            }

            //Debug
            std::cout << "Debug: " << content << std::endl;

            ifs.close();

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);


            writer.StartObject();

            writer.String("response");
            writer.String(responsetype.c_str());

            writer.String("Value1");
            writer.StartObject();

            writer.String("message");
            writer.String(content.c_str());

            writer.EndObject();

            writer.EndObject();

            std::cout << "Debug 1" << content << std::endl;

            const std::string html_content = buffer.GetString();

            //DEBUGG
            std::cout << "DEBUGG: " << html_content << std::endl;

            m_server.send(hdl, html_content, msg->get_opcode());
        }catch(const pqxx::pqxx_exception& e){
            std::cout << "An error occured while sending a template: " << e.base().what() << std::endl;
            throw e.base();
        }catch(const std::exception& e){
            std::cout << "An error occured while reading a template: " << e.what() << std::endl;
            throw e;
        }

        return;
    }
