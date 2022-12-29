//
// Created by ankpath on 9/13/22.
//

#include "ConnectionManager.h"
#include "Connection.h"

ConnectionManager::ConnectionManager(const int port) {
    createSocket();
    bind(port);
    listen();

    sockAddrLen = sizeof(sockAddrIn);
}

void ConnectionManager::createSocket() {
    int iSetOption = 1;
    sockFD = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, (char *) &iSetOption,
               sizeof(iSetOption));
    if (sockFD < 0) {
        std::cout << "Socket creation failed" << errno << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ConnectionManager::bind(const int port) {
    sockAddrIn.sin_family = AF_INET;
    sockAddrIn.sin_addr.s_addr = INADDR_ANY;
    sockAddrIn.sin_port = htons(port);

    if (::bind(sockFD, (struct sockaddr *) &sockAddrIn, sizeof(sockAddrIn)) < 0) {
        std::cout << "Failed to bind to port " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ConnectionManager::listen() {
    if (::listen(sockFD, 100) < 0) {
        std::cout << "Failed to listen " << errno << std::endl;
        exit(EXIT_FAILURE);
    }
}

Connection ConnectionManager::accept() {
    int connectionFD = ::accept(sockFD, (struct sockaddr *) &sockAddrIn, (socklen_t *) &sockAddrLen);

    if (connectionFD < 0) {
        std::cout << "Could not accept connection " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    return Connection(connectionFD);
}

void ConnectionManager::shutdown() {
    close(sockFD);
}
