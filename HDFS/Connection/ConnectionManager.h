//
// Created by ankpath on 9/13/22.
//
#include <sys/socket.h> // For socket functions
#include <netinet/in.h> // For sockaddr_in
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include "Connection.h"

#ifndef ASSIGNMENT1_CONNECTIONMANAGER_H
#define ASSIGNMENT1_CONNECTIONMANAGER_H


class ConnectionManager {
    sockaddr_in sockAddrIn;
    int sockFD, sockAddrLen;

    void bind(int port);

    void createSocket();

    void listen();

public:

    ConnectionManager(int port);

    Connection accept();

    void shutdown();
};


#endif //ASSIGNMENT1_CONNECTIONMANAGER_H
