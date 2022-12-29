//
// Created by ankpath on 9/13/22.
//

#ifndef ASSIGNMENT1_RETRIEVALREQUEST_H
#define ASSIGNMENT1_RETRIEVALREQUEST_H


#include "Request.h"

class RetrievalRequest : public Request {
    std::vector<std::string> keys;

    void parseRequest();

public:

    RetrievalRequest(const std::string &rawRequest, CommandType commandType);

    Response processRequestAndGetResponse(DataStore *dataStore);

    int recvPayload(Connection connection);

    bool isValid();
};


#endif //ASSIGNMENT1_RETRIEVALREQUEST_H
