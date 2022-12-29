//
// Created by ankpath on 9/13/22.
//

#include "RetrievalRequest.h"

RetrievalRequest::RetrievalRequest(const std::string &rawCommand, CommandType commandType) : Request(rawCommand,
                                                                                                     commandType) {
    if (isValid())
        parseRequest();
}

bool RetrievalRequest::isValid() {
    if (tokenizedCommand.size() < 2)
        return false;
    return true;
}

int RetrievalRequest::recvPayload(Connection connection) {
    return 0;
};


void RetrievalRequest::parseRequest() {

    auto iter = tokenizedCommand.begin();
    iter++;
    for (; iter != tokenizedCommand.end(); iter++) {
        keys.push_back(*iter);
    }
}

Response RetrievalRequest::processRequestAndGetResponse(DataStore *dataStore) {
    Response response = dataStore->get(keys[0]);
    for (int i = 1; i < keys.size(); i++) {
        response.appendResponse(dataStore->get(keys[i]));
    }
    response.appendResponse(Response::endResponse());
    return response;
}
