//
// Created by ankpath on 9/13/22.
//
#include <iostream>
#include <string>
#include <vector>
#include "HDFS/Data/DataStore.h"


#ifndef ASSIGNMENT1_UTILS_H
#define ASSIGNMENT1_UTILS_H

extern std::string DATA_STORE_FILENAME;
const int THREAD_POOL_SIZE = 100;
extern std::recursive_mutex THREAD_LOCK;


std::vector<std::string> getStringTokens(const std::string &buffer, char separator);

DataStore getGlobalDataStore(const std::string &filename);

void flushGlobalDataStore(const std::string &filename, const DataStore &dataStore);

#endif //ASSIGNMENT1_UTILS_H
