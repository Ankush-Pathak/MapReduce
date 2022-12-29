//
// Created by ankpath on 9/13/22.
//
#include<iostream>
#include<string>
#include <vector>

#include "HDFS/Common/Utils.h"
#include "HDFS/Response/Response.h"
#include "HDFS/Data/DataStore.h"
#include "HDFS/Connection/Connection.h"

#ifndef ASSIGNMENT1_REQUEST_H
#define ASSIGNMENT1_REQUEST_H

class Connection;

enum CommandType {
    SET, GET, NOT_SUPPORTED
};

class Request {
    std::string rawRequest, rawCommand;
    CommandType commandType;

    virtual void parseRequest() = 0;

protected:

    std::vector<std::string> tokenizedCommand;
    std::string rawPayload;
public:

    static const unsigned long MAX_REQUEST_SIZE_BYTES = 1000000; // 1 MB

    Request(const std::string &rawCommand, CommandType commandType);

    const std::string &getRawCommand() const;

    void setRawCommand(const std::string &rawCommand);

    CommandType getCommandType() const;

    void setCommandType(CommandType commandType);

    void setTokenizedCommand(std::vector<std::string> &tokens);

    std::vector<std::string> getTokenizedCommand() const;

    const std::string &getRawPayload() const;

    void setRawPayload(const std::string &rawPayload);

    virtual Response processRequestAndGetResponse(DataStore *dataStore) = 0;
    virtual int recvPayload(Connection connection) = 0;
    virtual bool isValid() = 0;

    static CommandType getCommand(const std::string &rawRequest) {
        if (rawRequest.empty())
            return NOT_SUPPORTED;
        if (rawRequest.substr(0, 3) == "get")
            return GET;
        if (rawRequest.substr(0, 3) == "set")
            return SET;
        return NOT_SUPPORTED;
    }
};

#endif //ASSIGNMENT1_REQUEST_H

