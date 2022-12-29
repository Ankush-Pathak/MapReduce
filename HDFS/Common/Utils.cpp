//
// Created by ankpath on 9/13/22.
//
#include<iostream>
#include<vector>
#include<sstream>
#include "HDFS/Data/DataStore.h"
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

std::recursive_mutex THREAD_LOCK;
std::string DATA_STORE_FILENAME = "cache.store";
std::vector<std::string> getStringTokens(const std::string &buffer, char separator) {
    std::vector<std::string> tokens;
    std::stringstream stringStream(buffer);
    std::string token;
    while (std::getline(stringStream, token, separator)) {
        tokens.push_back(token);
    }
    return tokens;
}


