//
// Created by ankpath on 9/13/22.
//

#include "StorageRequest.h"
#include "HDFS/Connection/Connection.h"

StorageRequest::StorageRequest(const std::string &rawRequest, CommandType commandType) : Request(rawRequest,
                                                                                                 commandType) {
    if (tokenizedCommand.size() >= 5)
        parseRequest();
}

void StorageRequest::parseRequest() {

    key = tokenizedCommand[1];
    flags = std::stoi(tokenizedCommand[2]);
    expiryTime = std::stoi(tokenizedCommand[3]);
    dataLength = std::stoi(tokenizedCommand[4]);

    if (tokenizedCommand.size() >= 6 && tokenizedCommand[5] == "noreply")
        isNoReply = true;
    else
        isNoReply = false;
}

int StorageRequest::recvPayload(Connection connection) {
    std::string buffer;
    buffer.reserve(dataLength);
    rawPayload.reserve(dataLength);

    long totalReadCount = 0;
    while (dataLength - totalReadCount != 0) {
        long readCount = connection.read(buffer, dataLength - totalReadCount);
        if (readCount < 0)
            return -1;
        totalReadCount += readCount;
        rawPayload.append(buffer);
    }
    if (connection.read(buffer, 2) < 2 || buffer != "\r\n")
        return -1;

    return rawPayload.length() + 2;
}

bool StorageRequest::isValid() {
    auto actualDataLength = getRawPayload().length();
    if (tokenizedCommand.size() < 5)
        return false;
    if (actualDataLength > dataLength || actualDataLength < dataLength)
        return false;
    return true;
}

Response StorageRequest::processRequestAndGetResponse(DataStore *dataStore) {
    return dataStore->push_data(key, DataValue(rawPayload, flags, expiryTime, dataLength), isNoReply);
}


