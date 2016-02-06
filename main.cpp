#include "Server.h"
#include <signal.h>
#include <stdio.h>

WSServer chatserver;

void stopServer(int signum){
    std::cout << "Interrupt signal (" << signum << ") received.\n";

    chatserver.stopServer(1);

    exit(signum);
}

int main() {

    signal(SIGINT, stopServer);
    chatserver.run(1919, 1920);
}


