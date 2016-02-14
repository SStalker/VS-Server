#include "Database.h"

Database::Database(){}

Database::~Database()
{
    if(conn != nullptr && w != nullptr){
        delete w;
        delete conn;
    }

}

bool Database::connect(){

	try{
		conn = new connection("dbname=chat user=postgres password=postgres hostaddr=127.0.0.1 port=5432");

		if (conn->is_open()) {
			std::cout << "Opened database successfully: " << conn->dbname() << std::endl;
		} else {
			std::cout << "Can't open database" << std::endl;
			return false;
		}

        w = new nontransaction(*conn);

        std::cout << "ID from test@test.de: " << getUserID("test@test.de") << std::endl;

        return true;

	}catch (const std::exception &e){
		std::cerr << e.what() << std::endl;
		return false;
	}
}

bool Database::registerClient(rapidjson::Document &doc){

    if( doc["values"].HasMember("email") && doc["values"].HasMember("password") && doc["values"].HasMember("nickname")
            && doc["values"]["email"].IsString() && doc["values"]["password"].IsString() && doc["values"]["nickname"].IsString()){

        pqxx::result r = w->exec(
                    "INSERT INTO users(email,password,nickname) "
                    "VALUES (" +
                    w->quote(doc["values"]["email"].GetString()) + ", " +
                w->quote(doc["values"]["password"].GetString()) + ", " +
                w->quote(doc["values"]["nickname"].GetString()) +
                ")\
            ");
        return true;
    }else{
        return false;
    }
}

bool Database::loginClient(rapidjson::Document &doc){

    std::cout << "Database::loginClient()" << std::endl;

    //ToDo pw validation
    pqxx::result r = w->exec(
               "SELECT password From users WHERE email=" + w->quote(doc["values"]["email"].GetString())
    );

    std::string pass = r[0]["password"].as<std::string>();

    //remove whitespaces
    pass = std::regex_replace(pass, std::regex("\\s+"), "");

    std::cout << pass.c_str() << " == " << doc["values"]["password"].GetString() << std::endl;
    if( r.size() == 1 && doc["values"]["password"].IsString() && strcmp(pass.c_str(),doc["values"]["password"].GetString()) == 0 ){
        pqxx::result r = w->exec(
                   "UPDATE users set online=true WHERE email=" + w->quote(doc["values"]["email"].GetString())
        );

        //Check if login was successfully and return bool
        pqxx::result check = w->exec(
                    "SELECT * FROM users WHERE email=" + w->quote(doc["values"]["email"].GetString())
                );
        if(check.affected_rows() == 1){
            return true;
        }else{
            pqxx::result r = w->exec(
                       "UPDATE users set online=false WHERE email=" + w->quote(doc["values"]["email"].GetString())
            );
            return false;
        }

    }else{
        return false;
    }


}

void Database::logoutClient(int uid){

    std::cout << "Database::logoutClient()" << std::endl;

    pqxx::result r = w->exec(
                "UPDATE users set online=false WHERE id=" + w->quote(uid)
            );
}

void Database::addFriendToChat(std::string cid, std::string uid){

    std::cout << "Database::addFriend()" << std::endl;

    pqxx::result r = w->exec(
                "INSERT INTO invites(uid, cid) "
                    "VALUES (" +
                    w->quote(uid) + ", " +
                    w->quote(cid) +
                    ")\
    ");
}

bool Database::friendRequest(int uid, int fid){
    std::cout << "Database::friendRequest()" << std::endl;

    if(uid == fid){
        return false;
    }

    pqxx::result check = w->exec(
                    "SELECT uid,fid FROM user_friend "
                    "WHERE (uid=" + w->quote(uid) + " AND fid=" + w->quote(fid) + ") "
                       "OR (uid=" + w->quote(fid) + " AND fid=" + w->quote(uid) + ") "
                    "UNION ALL "
                    "SELECT uid,fid FROM friend_invites "
                    "WHERE (uid=" + w->quote(uid) + " AND fid=" + w->quote(fid) + ") "
                       "OR (uid=" + w->quote(fid) + " AND fid=" + w->quote(uid) + ") "
                );

    if(check.affected_rows() == 0 ){
        pqxx::result r = w->exec(
                    "INSERT INTO friend_invites(uid, fid) "
                    "VALUES (" +
                    w->quote(uid) + ", " +
                    w->quote(fid) +
                    ")\
                ");
        return true;
    }else{
        return false;
    }
}

void Database::removeFriend(rapidjson::Document &doc){

    std::cout << "Database::removeFriend()" << std::endl;

    pqxx::result r = w->exec(
                "DELETE from COMPANY where ID = " + w->quote(doc["values"]["cid"].GetString())
    );
}

void Database::createChatroom(rapidjson::Document &doc){

}

void Database::removeChatroom(rapidjson::Document &doc){

}

void Database::chatroomRemoveClient(rapidjson::Document &doc){

}

void Database::chatroomAddClient(rapidjson::Document &doc){

}

void Database::chatroomNewMessage(rapidjson::Document &doc){

}

void Database::newMessage(rapidjson::Document &doc){

}

void Database::setStatus(rapidjson::Document &doc){

}

void Database::setSessionID(int uid, int sessionid){

    pqxx::result r = w->exec(
               "UPDATE users set sessionid=" + w->quote(sessionid) + " WHERE id=" + w->quote(uid)
    );

}

void Database::setOperator(rapidjson::Document &doc){

}


std::list<foundUsers> Database::getFriendRequests(int uid){
    std::list <foundUsers> requests;

    std::cout << "uid for request: " << uid << std::endl;

    pqxx::result r = w->exec(
                        "SELECT u.email, u.nickname FROM friend_invites fi "
                        "JOIN users u "
                            "ON fi.uid=u.id "
                        "WHERE fi.fid=" + w->quote(uid)
                );

    for(auto user : r){
        foundUsers row;

        row.email = user["email"].as<std::string>();
        row.nickname = user["nickname"].as<std::string>();

        std::cout << row.email << " (" << row.nickname << ")" << std::endl;

        requests.push_back(row);
    }

    return requests;
}

std::list< foundUsers > Database::getSearchedUsers(std::string search, int uid){

    std::list<foundUsers> found;
    pqxx::result r = w->exec(
                "SELECT email, nickname FROM users WHERE (email LIKE " +  w->quote( search ) + " OR nickname LIKE " + w->quote( search ) + ") AND NOT id=" + w->quote( uid )
            );
    for(auto user : r){
        foundUsers row;

        row.email = user["email"].as<std::string>();
        row.nickname = user["nickname"].as<std::string>();

        found.push_back(row);
    }

    return found;
}

std::list<std::string> Database::getFriendlist(rapidjson::Document &doc){

    std::list<std::string> list;
    pqxx::result r = w->exec(
        "SELECT u.email FROM users AS u, user_friend AS uf"
        "WHERE uf.ukd=" + w->quote( doc["values"]["uid"].GetString() ) +
        "AND"
        "uf.fid=u.id"
    );

    for( auto client : r){
        std::string mail = client["email"].as<std::string>();
        std::cout << client["email"] << std::endl;
        list.push_back( mail );
    }

    return list;
}

int Database::getUserIDFromSession(int sessionid){
    pqxx::result r = w->exec(""
                             "SELECT id FROM users "
                             "WHERE sessionid =" + w->quote( sessionid )
    );

    if(r.affected_rows() == 1){
        return r[0]["id"].as<int>();
    }else{
        return -1;
    }
}

int Database::getSessionIDFromUser(int id){
    pqxx::result r = w->exec(""
                             "SELECT sessionid FROM users "
                             "WHERE id =" + w->quote( id )
    );

    return r[0]["sessionid"].as<int>();

}

bool Database::userOnline(int id){
    pqxx::result r = w->exec(""
                             "SELECT online FROM users "
                             "WHERE id =" + w->quote( id )
    );

    return r[0]["online"].as<bool>();

}

foundUsers Database::getPubClientInformation(int id){
    pqxx::result r = w->exec(""
                             "SELECT email, nickname FROM users "
                             "WHERE id=" + w->quote( id )
    );
    foundUsers user;

    if(r.affected_rows() == 1){

        user.email = r[0]["email"].as<std::string>();
        user.nickname = r[0]["nickname"].as<std::string>();
    }

    return user;
}


std::string Database::getUserID(std::string email){

    pqxx::result r = w->exec(
        "SELECT id FROM users "
        "WHERE email=" + w->quote( email )
    );

    std::string id = r[0]["id"].as<std::string>();
    return id;
}

std::list<std::string> Database::getNewFriendshipRequests(rapidjson::Document &doc){

    std::list<std::string> list;
    return list;
}

std::list<std::string> Database::getNewOfflineMessages(rapidjson::Document &doc){

    std::list<std::string> list;
    return list;
}
