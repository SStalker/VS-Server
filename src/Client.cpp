#include "Client.h"

Client::Client(WSServer &server){}
void Client::login(std::string name, std::string password){}
void Client::logout(){}
void Client::sendMessage(int from, int to, std::string msg){}
void Client::addFriend(int friend_id){}
void Client::removeFriend(int friend_id){}
void Client::blockClient(int client_id){}
void Client::changeStatus(std::string status){}
void Client::decodeBase64(std::string code){}