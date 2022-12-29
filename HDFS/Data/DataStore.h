//
// Created by ankpath on 9/13/22.
//
#include <boost/serialization/unordered_map.hpp>
#include <string>
#include "HDFS/Response/Response.h"
#include "DataValue.h"
#include "Common/EnumsAndConstants.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sys/stat.h>
#include <mutex>
#include <fstream>

#ifndef ASSIGNMENT1_DATASTORE_H
#define ASSIGNMENT1_DATASTORE_H


class DataStore {
    std::unordered_map<std::string, DataValue> dataStore;
    std::fstream oFStream;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &archive, const unsigned int version) {
        archive & dataStore;
    }

public:
    DataStore();

    ll size();

    Response push_data(const std::string &key, const DataValue &dataValue, bool b);

    void push_data(const std::string &key, const DataValue &dataValue);

    Response get(const std::string &key);

    bool get(const std::string &key, std::string &value);

    void clear();

    void append(const std::string &key, const std::string& value, const bool isNoReply);

    const std::unordered_map<std::string, DataValue> & getMap() const;

    void getDataStoreFromFile(const std::string &filename);

    void flushDataStore(const std::string &filename);

};

extern DataStore *dataStore;


#endif //ASSIGNMENT1_DATASTORE_H
