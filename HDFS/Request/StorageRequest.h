//
// Created by ankpath on 9/13/22.
//

#ifndef ASSIGNMENT1_STORAGEREQUEST_H
#define ASSIGNMENT1_STORAGEREQUEST_H


#include "Request.h"
#include "HDFS/Connection/Connection.h"

class StorageRequest : public Request {


    std::string key, data;
    int flags;
    unsigned long expiryTime, dataLength;
    bool isNoReply;

    void parseRequest();

public:
    StorageRequest(const std::string &rawRequest, CommandType commandType);

    bool isValid();

    Response processRequestAndGetResponse(DataStore *dataStore);

    int recvPayload(Connection connection);
};


#endif //ASSIGNMENT1_STORAGEREQUEST_H
