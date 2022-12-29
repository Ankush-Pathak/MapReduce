//
// Created by ankpath on 9/13/22.
//

#ifndef ASSIGNMENT1_RESPONSE_H
#define ASSIGNMENT1_RESPONSE_H


#include <string>

enum ResponseType {
    STORED
};

class Response {
    std::string rawResponse;
    ResponseType responseType;

public:
    explicit Response(const std::string &rawResponse, ResponseType responseType = STORED);

    void appendResponse(const Response &response) {
        this->rawResponse.append(response.rawResponse);
    }

    ResponseType getResponseType() const;

    void setResponseType(ResponseType responseType);

    const std::string &getRawResponse() const;

    void setRawResponse(const std::string &rawResponse);

    // TODO: Create classes for each response
    static Response storedResponse() {
        return Response("STORED\r\n");
    }

    static Response notStoredResponse() {
        return Response("NOT_STORED\r\n");
    }

    static Response
    valueResponse(const std::string &key, const int flags, unsigned long dataLength, const std::string &data) {
        return Response(
                "VALUE " + key + " " + std::to_string(flags) + " " + std::to_string(dataLength) + "\r\n" + data +
                "\r\n");
    }

    static Response errorResponse() {
        return Response("ERROR\r\n");
    }

    static Response clientErrorResponse(const std::string &errorMsg) {
        return Response("CLIENT_ERROR " + errorMsg + "\r\n");
    }

    static Response serverErrorResponse(const std::string &errorMsg) {
        return Response("SERVER_ERROR " + errorMsg + "\r\n");
    }

    static Response emptyResponse() {
        return Response("");
    }

    static Response endResponse() {
        return Response("END\r\n");
    }

    const unsigned int getResponseLength() const;
};


#endif //ASSIGNMENT1_RESPONSE_H
