//
// Created by ankpath on 9/13/22.
//

#ifndef ASSIGNMENT1_CONNECTION_H
#define ASSIGNMENT1_CONNECTION_H


#include <string>
#include "HDFS/Response/Response.h"
#include "HDFS/Request/Request.h"

class Connection {
    int connectionFD;
    char cStrBuffer[Request::MAX_REQUEST_SIZE_BYTES];

public:
    Connection(const int connectionFd);

    int read(std::string &buffer, unsigned int size);

    int send(const std::string &buffer, const unsigned int size);

    int send(const Response &response);

    void close();

    int readCommand(std::string &buffer, unsigned int size);
};


#endif //ASSIGNMENT1_CONNECTION_H
