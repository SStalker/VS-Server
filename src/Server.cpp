#include "Server.h"

typedef pair<string, string> param;


WSServer::WSServer() : m_next_sessionid(1) {

        // Check the db connection
        if(!db.connect())
            return;

        m_server.init_asio(&ios);
        m_server.set_reuse_addr(true);
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
        
        cout << "Closing connection " << data.name 
                  << " with sessionid " << data.sessionid << endl;
        db.setSessionID(db.getUserIDFromSession(data.sessionid), -1);
        
        m_connections.erase(hdl);
    }
    
    void WSServer::on_message(connection_hdl hdl, server::message_ptr msg) {

        try {
            rapidjson::Document document;
            document.Parse(msg->get_payload().c_str());

            /* ToDo:-create validator
             *      -create multiple missings otpions
             */

            if(document.HasMember("request")
                    && document.HasMember("values")){
                if(document["request"].IsString() && document["values"].IsObject()){
                    if(strcmp(document["request"].GetString(),"registration") == 0){
                        //register new client in database
                        try{
                            vector<pair<string, string>> values;
                            //Register new user in database
                            if(db.registerClient(document)){
                                //Build response vector if successfully registered
                                values.push_back(param("message","success"));

                                //Build and Send response
                                m_server.send(hdl, response("response", document["request"].GetString(), values), msg->get_opcode());
                            }else{
                                values.push_back(param("message","failed"));

                                //Build and Send response
                                m_server.send(hdl, response("response", document["request"].GetString(), values), msg->get_opcode());
                            }

                        }catch(const pqxx::pqxx_exception& e){
                            //do something in case of failure
                            m_server.send(hdl, createError(e.base(), "database"), msg->get_opcode());
                        }
                    }else if(strcmp(document["request"].GetString(),"login") == 0){
                        //login client (ToDo: validate if all parameter are ok
                        try{
                            vector<pair<string, string> > values;
                            if(db.loginClient(document)){
                                //if login ok

                                string userid = db.getUserID(document["values"]["email"].GetString());

                                values.push_back(pair<string, string>("login","success"));

                                //add sessionid to response
                                int session = m_connections[hdl].sessionid;
                                db.setSessionID(atoi(userid.c_str()), session);

                                map<string, string> map;

                                db.getUserDataFrom(userid, map);

                                for( auto &kv : map){
                                    cout << kv.first << " has value: " << kv.second << endl;
                                    values.push_back( param(kv.first, kv.second) );
                                }

                                //If client is webclient add template to the response
                                if(document["values"].HasMember("webclient")){
                                    values.push_back( param("template", readTemplate( "../webclient-templates/intern.html")));
                                }

                                m_server.send(hdl, response("response", document["request"].GetString(), values) ,msg->get_opcode());

                                // To be sure we delete all elements
                                map.clear();

                                //Send friendlist
                                list<friendListUser> friendlist;
                                friendlist = db.getFriendlist(db.getUserIDFromSession(session));
                                m_server.send(hdl, sendFriendlist("pushmsg","friendlist", friendlist), msg->get_opcode());

                                //Send friend open requests
                                list<foundUsers> requests;
                                requests = db.getFriendRequests(db.getUserIDFromSession(session));
                                m_server.send(hdl, responseSearchedList("pushmsg", "friendRequest", requests), msg->get_opcode());

                                //notify friends
                                values.clear();
                                values.push_back(param("email",document["values"]["email"].GetString()));
                                values.push_back(param("online","true"));

                                int id;
                                for(auto con: m_connections){
                                    id = db.getUserIDFromSession(con.second.sessionid);
                                    for(auto user: friendlist){
                                        if(user.id == id){
                                            m_server.send(con.first, response("pushmsg","notifyOnline", values), websocketpp::frame::opcode::text);
                                            continue;
                                        }
                                    }
                                }

                            }else{
                                values.push_back(pair<string, string>("login","failed"));
                                m_server.send(hdl, response("response", document["request"].GetString(), values) ,msg->get_opcode());
                            }

                        }catch( const pqxx::pqxx_exception& e){
                            m_server.send(hdl, createError(e.base(), "database"), msg->get_opcode());
                        }catch( const exception& e){
                            m_server.send(hdl, createError(e, "server"), msg->get_opcode());
                        }
                    }else if(strcmp(document["request"].GetString(),"logout") == 0 ){

                        vector<pair<string, string> > values;
                        int sid = m_connections[hdl].sessionid;

                        int uid = db.getUserIDFromSession(sid);

                        try{
                            //logout user from session
                            db.logoutClient(uid);

                            //update session id in DB
                            db.setSessionID(uid ,-1);

                            //Notify Client about succesfull logout
                            values.push_back(param("logout","success"));
                            m_server.send(hdl, response("response", document["request"].GetString(), values) ,msg->get_opcode());

                            //notifyFriends
                            values.clear();
                            values.push_back(param("email",db.getEmail(uid)));
                            values.push_back(param("online","false"));
                            vector<int> friendIds = db.getFrindIds(uid);
                            int id;
                            for(auto con: m_connections){
                                id = db.getUserIDFromSession(con.second.sessionid);
                                for(auto friendid: friendIds){
                                    if(friendid == id){
                                        m_server.send(con.first, response("pushmsg","notifyOnline", values), websocketpp::frame::opcode::text);
                                        continue;
                                    }
                                }
                            }

                        }catch(const pqxx::pqxx_exception& e){
                            m_server.send(hdl, createError(e.base(), "database"), msg->get_opcode());
                        }
                    }else if(strcmp(document["request"].GetString(),"searchUser") == 0 ){
                        //Vaidate JSON format
                        if(document["values"].HasMember("searchUser") && document["values"]["searchUser"].IsString()){
                            //Get all users where search term is true
                            stringstream search;
                            search << "%" << document["values"]["searchUser"].GetString() << "%";
                            list<foundUsers> found = db.getSearchedUsers(search.str(), db.getUserIDFromSession(m_connections[hdl].sessionid));

                            //Send List du Client
                            m_server.send(hdl, responseSearchedList("response", document["request"].GetString(), found) ,msg->get_opcode());
                        }

                    }else if(strcmp(document["request"].GetString(),"addFriend") == 0 ){
                        vector<pair<string, string> > responseValues;
                        //Vaidate JSON format
                        if(document["values"].HasMember("friendMail") && document["values"]["friendMail"].IsString() ){

                            //Get Client id's
                            int friendID = atoi( db.getUserID( document["values"]["friendMail"].GetString() ).c_str() );
                            int uid = db.getUserIDFromSession(m_connections[hdl].sessionid);

                            //Add to friendlist if possible
                            if(friendID != -1 && db.friendRequest(uid, friendID)){

                                //Send notification to Clients
                                if(db.userOnline(friendID)){
                                    list<foundUsers> values;
                                    foundUsers from = db.getPubClientInformation(uid);
                                    values.push_back(from);

                                    for(auto con: m_connections){
                                        cout << "Send Friend request" << endl;
                                        if(db.getSessionIDFromUser(friendID) == con.second.sessionid){
                                            m_server.send(con.first, responseSearchedList("pushmsg", "friendRequest" ,values) ,websocketpp::frame::opcode::text);
                                            db.setFriendRequestTransmition(uid,friendID);
                                        }
                                    }
                                    //Notify client
                                    responseValues.push_back(param("friendRequest", "success"));
                                    m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                                }
                            }else{
                                //Notify client
                                responseValues.push_back(param("friendRequest", "failure"));
                                m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                            }
                        }

                    }else if(strcmp(document["request"].GetString(),"acceptFriend") == 0 ){
                        vector<pair<string, string> > responseValues;
                        //Vaidate JSON format
                        if(document["values"].HasMember("friendMail") && document["values"].HasMember("answer") && document["values"]["friendMail"].IsString() &&  document["values"]["answer"].IsBool()){
                            //Get Client id's
                            int friendID = atoi( db.getUserID( document["values"]["friendMail"].GetString() ).c_str() );
                            int uid = db.getUserIDFromSession(m_connections[hdl].sessionid);
                            //Check if request was accepted by user
                            if(document["values"]["answer"].GetBool()){

                                //Set friendship as accepted
                                db.acceptFriendRequest(uid, friendID);
                                responseValues.push_back(param("acceptRequest","success"));
                                responseValues.push_back(param("friendMail", document["values"]["friendMail"].GetString()));

                                //notify accepting client
                                m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );

                                //Create chat for friends
                                int cid = db.createChat(uid, friendID);

                                // Get friendListUser for new friend to add to present friendlist in Client
                                list<friendListUser> tmp;
                                tmp.push_back(db.getFriendListUserFromID(friendID, cid));
                                m_server.send(hdl, sendFriendlist("pushmsg", "newFriendListUser", tmp) ,msg->get_opcode());

                                //Send Friendlist user if user is online, else do nothing.
                                if(db.userOnline(friendID)){
                                    //Get friendListUser for yourself to add to present friendlist in friends CLient
                                    tmp.clear();
                                    tmp.push_back(db.getFriendListUserFromID(uid, cid));
                                    m_server.send(get_hdl_from_session(db.getSessionIDFromUser(friendID)), sendFriendlist("pushmsg", "newFriendListUser", tmp) ,msg->get_opcode());
                                }
                            }else{
                                // If not accepted
                                if(db.removeRequest(uid, friendID)){
                                    responseValues.push_back(param("acceptRequest","success"));
                                    m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                                }else{
                                    responseValues.push_back(param("acceptRequest","failure"));
                                    m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                                }

                            }

                        }else{
                            //Notify CLient that JSON was not valid/missing a parameter
                            responseValues.push_back(param("acceptRequest", "failure"));
                            m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                        }
                    }else if(strcmp(document["request"].GetString(),"removeFriend") == 0 ){
                        vector<pair<string, string> > responseValues;
                        //Vaidate JSON format
                        if(document["values"].HasMember("friendMail") && document["values"]["friendMail"].IsString() ){
                            //Get user id's
                            int uid = db.getUserIDFromSession(m_connections[hdl].sessionid);
                            int fid = atoi(db.getUserID(document["values"]["friendMail"].GetString()).c_str());

                            //Delete from friends and chats aswell as from chatlist
                            db.removeFriend(uid, fid);

                            //Notify client
                            responseValues.push_back(param("removeRequest", "success"));
                            m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                        }else{
                            //Notify client that JSON was not valid
                            responseValues.push_back(param("removeRequest", "failure"));
                            m_server.send(hdl,response("response", document["request"].GetString(), responseValues), msg->get_opcode() );
                        }
                    }else if(strcmp(document["request"].GetString(),"newMessage") == 0 ){

                        vector<pair<string, string> > values;

                        //Vaidate JSON format
                        if(document["values"].HasMember("messageFrom") && document["values"]["messageFrom"].IsString() &&
                            document["values"].HasMember("messageTo") && document["values"]["messageTo"].IsInt() &&
                            document["values"].HasMember("message") && document["values"]["message"].IsString()
                        ){

                            cout << "Got an valid message" << endl;

                            int uid = db.getUserIDFromSession(m_connections[hdl].sessionid);
                            int cid = document["values"]["messageTo"].GetInt();
                            vector<int> uidResult = db.getUserID(uid, cid);

                            // set the response
                            values.push_back( param("messageFrom", document["values"]["messageFrom"].GetString()) );
                            values.push_back( param("messageTo", to_string(document["values"]["messageTo"].GetInt())) );
                            values.push_back( param("message",document["values"]["message"].GetString()) );

                            // if it is a simple user
                            //if(db.belongsChatIDToUser(cid)){
                            if(uidResult.size() == 1){
                                cout << "Message should be send to user" << endl;
                                int uidFromCid = uidResult[0];
                                if(db.userOnline( uidFromCid )){
                                    cout << "User " << uidFromCid << " is online" << endl;
                                    int sessionID = db.getSessionIDFromUser(uidFromCid);
                                    m_server.send(get_hdl_from_session(sessionID), response("response", document["request"].GetString(), values), msg->get_opcode());
                                    cout << "Message will be stored from online user" << endl;
                                    db.newMessage(uid, cid, document["values"]["message"].GetString(), true);
                                }else{
                                    cout << "User " << uidFromCid << " is offline message will be stored" << endl;
                                    db.newMessage(uid, cid, document["values"]["message"].GetString(), false);
                                }


                            // if it is a chatroom
                            } else if(uidResult.size() > 1){
                                cout << "Message should be send to chatroom" << endl;
                                // online chatroomusers
                                vector<int> sessions;
                                db.getAllOnlineUsersOfChatroom(cid, sessions);

                                for(int session : sessions){
                                    m_server.send(get_hdl_from_session(session), response("response", document["request"].GetString(), values), msg->get_opcode());
                                }
                                // TODO: the offline users msg must be saved in db for later transmitting
                                 db.newMessage(uid, cid, document["values"]["message"].GetString(), true);
                            } else {
                                //uidResult < 1 error
                                values.push_back(param("err_msg", "no chat found"));
                                m_server.send(hdl, response("response", "error", values) ,msg->get_opcode());
                            }
                        }else{
                            values.push_back( param(document["request"].GetString(), "failure") );
                            m_server.send(hdl,response("response", document["request"].GetString(), values), msg->get_opcode() );
                        }
                    }
                }
            }
        } catch (const websocketpp::lib::error_code& e) {
            cout << "Echo failed because: " << e
                      << "(" << e.message() << ")" << endl;
        }
    }

    // No change to TLS init methods from echo_server_tls
    string WSServer::get_password() {
        return "testtest";
    }

    context_ptr WSServer::on_tls_init(websocketpp::connection_hdl hdl) {
    
        cout << "on_tls_init called with hdl: " << hdl.lock().get() << endl;
        context_ptr ctx(new boost::asio::ssl::context(boost::asio::ssl::context::tlsv1));

        try {
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use);
            ctx->set_password_callback(bind(&WSServer::get_password, this));
            ctx->use_certificate_chain_file("server.pem");
            ctx->use_private_key_file("server.pem", boost::asio::ssl::context::pem);
        } catch (exception& e) {
            cout << e.what() << endl;
        }
        return ctx;
}
    
    connection_data& WSServer::get_data_from_hdl(connection_hdl hdl) {
        auto it = m_connections.find(hdl);
        
        if (it == m_connections.end()) {
            // this connection is not in the list. This really shouldn't happen
            // and probably means something else is wrong.
            throw invalid_argument("No data avaliable for session");
        }
        
        return it->second;
    }

    connection_hdl WSServer::get_hdl_from_session(int sessionID){

        for(auto con: m_connections){
            if(con.second.sessionid == sessionID){
                return con.first;
            }
        }
    }
    
    void WSServer::run(uint16_t port_plain, uint16_t port_tls) {

        m_server.listen(port_plain);
        m_server.start_accept();

        tls.listen(port_tls);
        tls.start_accept();

        ios.run();
    }

    void WSServer::stopServer(){
        cout << "Stop Server ---- Close Connections" << endl;

        websocketpp::lib::error_code ec;
        m_server.stop_listening(ec);

        if (ec) {
            // Failed to stop listening. Log reason using ec.message().
            cout << "Could not stop listening at m_server: " << ec.message() << endl;
            return;
        }

        tls.stop_listening(ec);

        if (ec) {
            // Failed to stop listening. Log reason using ec.message().
            cout << "Could not stop listening at tls: " << ec.message() << endl;
            return;
        }


        // Close all existing websocket connections.
        string data = "Terminating connection...";
        for(auto hdl: m_connections){
            m_server.close(hdl.first, websocketpp::close::status::going_away, data, ec);
        }

        // Stop the endpoint.
        m_server.stop();
        tls.stop();

        ios.stop();
    }


    const string WSServer::response(string key, string responsetype, vector<pair<string, string>> response){
        try{
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);            

            writer.StartObject();

            writer.Key(key.c_str());
            writer.String(responsetype.c_str());

            writer.Key("values");
            writer.StartObject();

            for(pair<string, string> values : response){
                writer.Key(values.first.c_str());
                writer.String(values.second.c_str());
            }

            writer.EndObject();
            writer.EndObject();

            return buffer.GetString();
        }catch (const exception& e) {
            cout << "Failed to build json: (" << e.what() << ")" << endl;
        }


    }

    const string WSServer::responseSearchedList(string key, string responsetype, list<foundUsers> responseList){
        try{
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

            writer.StartObject();

            writer.Key(key.c_str());
            writer.String(responsetype.c_str());

            writer.Key("values");
            writer.StartArray();

            for(foundUsers row : responseList){
                writer.StartObject();

                writer.Key("email");
                writer.String(row.email.c_str());

                writer.Key("nickname");
                writer.String(row.nickname.c_str());

                writer.EndObject();
            }
            writer.EndArray();
            writer.EndObject();

            return buffer.GetString();
        }catch (const exception& e) {
            cout << "Failed to build json: (" << e.what() << ")" << endl;
        }
    }

    const __cxx11::string WSServer::sendFriendlist(__cxx11::string key, __cxx11::string responsetype, list<friendListUser> responseList){
        try{
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

            writer.StartObject();

            writer.Key(key.c_str());
            writer.String(responsetype.c_str());

            writer.Key("values");
            writer.StartArray();

            for(friendListUser row : responseList){
                writer.StartObject();

                writer.Key("email");
                writer.String(row.email.c_str());

                writer.Key("nickname");
                writer.String(row.nickname.c_str());

                writer.Key("firstname");
                writer.String(row.firstname.c_str());

                writer.Key("lastname");
                writer.String(row.lastname.c_str());

                writer.Key("birthday");
                writer.String(row.birthday.c_str());

                writer.Key("image");
                writer.String(row.imageb64.c_str());

                writer.Key("online");
                writer.Bool(row.online);

                writer.Key("cid");
                writer.String(row.cid.c_str());

                writer.Key("chatname");
                writer.String(row.chatname.c_str());

                writer.Key("chatstatus");
                writer.String(row.chatstatusmsg.c_str());

                writer.EndObject();
            }
            writer.EndArray();
            writer.EndObject();

            return buffer.GetString();
        }catch (const exception& e) {
            cout << "Failed to build json: (" << e.what() << ")" << endl;
        }
    }

    const string WSServer::createError(const exception& e, string from){
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

        }catch (const exception& e) {
            cout << "Failed to build json: (" << e.what() << ")" << endl;
        }
    }

    const string WSServer::readTemplate(string filename){
        ifstream ifs(filename);
        if(ifs.is_open()){
            string content1((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
            ifs.close();
            return content1.c_str();
        }else{
            ifs.close();
            throw logic_error("Could not open file");
        }
    }

    const string WSServer::buildTemplateJson(string filename, string responsetype){

        string temp;

        try{
            temp = readTemplate(filename);
        }catch(const exception& e){
            cout << e.what() << endl;
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
