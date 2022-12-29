//
// Created by ankpath on 9/13/22.
//

#include "Response.h"

Response::Response(const std::string &rawResponse, ResponseType responseType) : rawResponse(rawResponse),
                                                                                responseType(responseType) {}

const std::string &Response::getRawResponse() const {
    return rawResponse;
}

const unsigned int Response::getResponseLength() const {
    return rawResponse.length();
}

void Response::setRawResponse(const std::string &rawResponse) {
    Response::rawResponse = rawResponse;
}

ResponseType Response::getResponseType() const {
    return responseType;
}

void Response::setResponseType(ResponseType responseType) {
    Response::responseType = responseType;
}
