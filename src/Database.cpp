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

        w = new work(*conn);



        pqxx::result r = w->exec("SELECT * FROM users");
        w->commit();

        //const auto ci = result::const_iterator;
        for (auto i : r)
         {

           std::cout << i["id"] << std::endl;
         }

        delete w;
        //conn->disconnect ();

        return true;

	}catch (const std::exception &e){
		std::cerr << e.what() << std::endl;
		return false;
	}
}

void Database::registerClient(rapidjson::Document &doc){

    pqxx::result r = w->exec(
                "INSERT INTO users(email,password,firstname,lastname,birthday,operator) "
                    "VALUES (" +
                    w->quote(doc["email"].GetString()) + ", " +
                    w->quote(doc["password"].GetString()) + ", " +
                    w->quote(doc["firstname"].GetString()) + ", " +
                    w->quote(doc["lastname"].GetString()) + ", " +
                    w->quote(doc["birthday"].GetString()) + ", " +
                    w->quote(doc["op"].GetString()) +
                    ")\
    ");

    w->commit();

}

void Database::loginClient(rapidjson::Document &doc){

    pqxx::result r = w->exec(
               "UPDATE users set online=1 WHERE email=" + w->quote(doc["email"].GetString())
    );
    w->commit();
}

void Database::logoutClient(rapidjson::Document &doc){

    pqxx::result r = w->exec(
               "UPDATE users set online=0 WHERE email=" + w->quote(doc["email"].GetString())
    );
    w->commit();
}

void Database::addFriend(rapidjson::Document &doc){

    pqxx::result r = w->exec(
                "INSERT INTO invites(from, to) "
                    "VALUES (" +
                    w->quote(doc["uid"].GetString()) + ", " +
                    w->quote(doc["cid"].GetString()) +
                    ")\
    ");

    w->commit();
}

void Database::removeFriend(rapidjson::Document &doc){

    pqxx::result r = w->exec(
                "DELETE from COMPANY where ID = " + w->quote(doc["cid"].GetString())
    );

    w->commit();
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

std::list<std::string> Database::getNewFriendshipRequests(){

    std::list<std::string> list;
    return list;
}

std::list<std::string> Database::getNewOfflineMessages(){

    std::list<std::string> list;
    return list;
}
