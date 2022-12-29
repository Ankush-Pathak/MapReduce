//
// Created by ankpath on 9/13/22.
//
#include <iostream>
#include <utility>
#include <vector>
#include <sstream>

#include "Request.h"

const std::string &Request::getRawCommand() const {
    return rawCommand;
}

void Request::setRawCommand(const std::string &rawCommand) {
    this->rawCommand = rawCommand;
}

CommandType Request::getCommandType() const {
    return commandType;
}

void Request::setCommandType(CommandType commandType) {
    Request::commandType = commandType;
}

void Request::setTokenizedCommand(std::vector<std::string> &tokens) {
    this->tokenizedCommand = std::move(tokens);
}

std::vector<std::string> Request::getTokenizedCommand() const {
    return this->tokenizedCommand;
}

const std::string &Request::getRawPayload() const {
    return rawPayload;
}

void Request::setRawPayload(const std::string &rawPayload) {
    Request::rawPayload = rawPayload;
}

Request::Request(const std::string &rawCommand, CommandType commandType) : commandType(commandType) {
    std::stringstream stringStream(rawCommand);
    std::getline(stringStream, this->rawCommand);
    tokenizedCommand = getStringTokens(rawCommand, ' ');
}

