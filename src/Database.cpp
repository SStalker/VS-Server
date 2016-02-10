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

bool Database::logoutClient(rapidjson::Document &doc){

    if(doc["values"].HasMember("uid") && doc["values"]["uid"].IsString()){
        std::cout << "Database::logoutClient()" << std::endl;

        pqxx::result r = w->exec(
                    "UPDATE users set online=false WHERE id=" + w->quote(doc["values"]["uid"].GetString())
                );
        return true;
    }else{
        return false;
    }

}

void Database::addFriend(rapidjson::Document &doc){

    std::cout << "Database::addFriend()" << std::endl;

    pqxx::result r = w->exec(
                "INSERT INTO invites(from, to) "
                    "VALUES (" +
                    w->quote(doc["values"]["uid"].GetString()) + ", " +
                    w->quote(doc["values"]["cid"].GetString()) +
                    ")\
    ");
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

void Database::setOperator(rapidjson::Document &doc){

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
