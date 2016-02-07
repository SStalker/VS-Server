#include "Server.h"

typedef std::pair<std::string, std::string> param;


WSServer::WSServer() : m_next_sessionid(1) {

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

        try {
            rapidjson::Document document;
            document.Parse(msg->get_payload().c_str());
            /* ToDo:-create validator
             *      -create multiple missings otpions
             */

            if(document.HasMember("request")){
                if(document["request"].IsString()){
#if 0
                    if(strcmp(document["request"].GetString(),"registersite") == 0){
                        //deploy new template for webclient
                        try{
                            m_server.send(hdl, buildTemplateJson( "../webclient-templates/register.html", "registersite"), msg->get_opcode());
                        }catch(const std::exception& e){
                            //do something in case of failure
                            m_server.send(hdl, createError(e, "server"), msg->get_opcode());
                        }
                    }else
#endif
                    if(strcmp(document["request"].GetString(),"registration") == 0){
                        //register new client in database
                        try{
                            //Register new user in database
                            db.registerClient(document);
                            //Build response vector if successfully registered
                            std::vector<std::pair<std::string, std::string>> values;
                            values.push_back(std::pair<std::string, std::string>("message","success"));

                            //Build and Send response
                            m_server.send(hdl, response(document["request"].GetString(), values), msg->get_opcode());

                        }catch(const pqxx::pqxx_exception& e){
                            //do something in case of failure
                            m_server.send(hdl, createError(e.base(), "database"), msg->get_opcode());
                        }
                    }else if(strcmp(document["request"].GetString(),"login") == 0){
                        //login client
                        try{
                            std::vector<std::pair<std::string, std::string> > values;
                            if(db.loginClient(document)){
                                //if login ok
                                values.push_back(std::pair<std::string, std::string>("login","success"));
                                values.push_back( param("uid", db.getUserID("test@test.de")) );

                                //add sessionid to response
                                std::stringstream sid;
                                sid << m_connections[hdl].sessionid;
                                values.push_back( param("sid", sid.str()));

                                //If client is webclient add template to the response
                                if(document["values"].HasMember("webclient")){
                                    values.push_back( param("template", readTemplate( "../webclient-templates/intern.html")));
                                }

                                m_server.send(hdl, response(document["request"].GetString(), values) ,msg->get_opcode());
                                //send chat template
                                //m_server.send(hdl,buildTemplateJson( "../webclient-templates/intern.html", document["request"].GetString()) , msg->get_opcode());
                            }else{
                                values.push_back(std::pair<std::string, std::string>("login","failed"));
                                m_server.send(hdl, response(document["request"].GetString(), values) ,msg->get_opcode());
                            }

                        }catch( const pqxx::pqxx_exception& e){
                            m_server.send(hdl, createError(e.base(), "database"), msg->get_opcode());
                        }catch( const std::exception& e){
                            m_server.send(hdl, createError(e, "server"), msg->get_opcode());
                        }
                    }
                }
            }

            //Echo
            std::stringstream ss;
            ss << "Echo";
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

    void WSServer::stopServer(){
        std::cout << "Stop Server ---- Close Connections" << std::endl;

        websocketpp::lib::error_code ec;
        m_server.stop_listening(ec);

        if (ec) {
            // Failed to stop listening. Log reason using ec.message().
            std::cout << "Could not stop listening at m_server: " << ec.message() << std::endl;
            return;
        }

        tls.stop_listening(ec);

        if (ec) {
            // Failed to stop listening. Log reason using ec.message().
            std::cout << "Could not stop listening at tls: " << ec.message() << std::endl;
            return;
        }


        // Close all existing websocket connections.
        std::string data = "Terminating connection...";
        for(auto hdl: m_connections){
            m_server.close(hdl.first, websocketpp::close::status::going_away, data, ec);
        }

        // Stop the endpoint.
        m_server.stop();
        tls.stop();

        ios.stop();
    }

    const std::string WSServer::response(std::string responsetype, std::vector<std::pair<std::string, std::string>> response){
        try{
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);            

            writer.StartObject();

            writer.Key("response");
            writer.String(responsetype.c_str());

            writer.Key("values");
            writer.StartObject();

            for(std::pair<std::string, std::string> values : response){
                writer.Key(values.first.c_str());
                writer.String(values.second.c_str());
            }

            writer.EndObject();
            writer.EndObject();

            return buffer.GetString();
        }catch (const std::exception& e) {
            std::cout << "Failed to build json: (" << e.what() << ")" << std::endl;
        }


    }

    const std::string WSServer::createError(const std::exception& e, std::string from){
        try{

            rapidjson::StringBuffer error_buffer;
            rapidjson::Writer<rapidjson::StringBuffer> error_writer(error_buffer);

            error_writer.StartObject();

            error_writer.Key("response");
            error_writer.String("error");

            error_writer.Key("values");
            error_writer.StartObject();

            error_writer.Key("from");
            error_writer.String(from.c_str());

            error_writer.Key("err_msg");
            error_writer.String(e.what());

            error_writer.EndObject();
            error_writer.EndObject();

            return error_buffer.GetString();

        }catch (const std::exception& e) {
            std::cout << "Failed to build json: (" << e.what() << ")" << std::endl;
        }
    }

    const std::string WSServer::readTemplate(std::string filename){
        std::ifstream ifs(filename);
        if(ifs.is_open()){
            std::string content1((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            return content1.c_str();
        }else{
            ifs.close();
            throw std::logic_error("Could not open file");
        }
    }

    const std::string WSServer::buildTemplateJson(std::string filename, std::string responsetype){

        std::string temp;

        try{
            temp = readTemplate(filename);
        }catch(const std::exception& e){
            std::cout << e.what() << std::endl;
            throw e;
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        writer.StartObject();

        writer.Key("response");
        writer.String(responsetype.c_str());

        writer.Key("values");
        writer.StartObject();

        writer.Key("template");
        writer.String(temp.c_str());

        writer.EndObject();
        writer.EndObject();

        return buffer.GetString();
    }
