#include "Database.h"

Database::Database(){}

bool Database::connect(){

	try{
		conn = new connection("dbname=chat user=postgres password=postgres hostaddr=127.0.0.1 port=5432");
		
		if (conn->is_open()) {
			std::cout << "Opened database successfully: " << conn->dbname() << std::endl;
		} else {
			std::cout << "Can't open database" << std::endl;
			return false;
		}

		conn->disconnect ();

	}catch (const std::exception &e){
		std::cerr << e.what() << std::endl;
		return false;
	}
}

void Database::registerClient(){}
void Database::addFriend(){}
void Database::removeFriend(){}
void Database::createChatroom(){}
void Database::removeChatroom(){}
void Database::chatroomRemoveClient(){}
void Database::chatroomAddClient(){}
void Database::chatroomNewMessage(){}
void Database::newMessage(){}
void Database::setStatus(){}
void Database::setOperator(){}
