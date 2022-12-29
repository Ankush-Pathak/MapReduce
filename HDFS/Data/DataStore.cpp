//
// Created by ankpath on 9/13/22.
//

#include <fstream>
#include "DataStore.h"
#include "HDFS/Common/Utils.h"
#include "Thirdparty/spdlog/include/spdlog/include/spdlog/spdlog.h"

DataStore *dataStore = new DataStore();

DataStore::DataStore() {
    dataStore.reserve(65000);
}


Response DataStore::push_data(const std::string &key, const DataValue &dataValue, const bool isNoReply) {
    THREAD_LOCK.lock();
//    if(dataStore.find(key) != dataStore.end()) {
//        THREAD_LOCK.unlock();
//        return isNoReply ? Response::emptyResponse(): Response::notStoredResponse();
//    }
    spdlog::debug("Storing {}: {}", key, dataValue.getData());
    dataStore[key] = dataValue;
    THREAD_LOCK.unlock();
    return isNoReply ? Response::emptyResponse() : Response::storedResponse();
}

void DataStore::push_data(const std::string &key, const DataValue &dataValue) {
    std::lock_guard lockGuard(THREAD_LOCK);
    dataStore[key] = dataValue;
}

Response DataStore::get(const std::string &key) {
    THREAD_LOCK.lock();
    auto valueItr = dataStore.find(key);
    if (valueItr == dataStore.end()) {
        THREAD_LOCK.unlock();
        return Response::emptyResponse();
    }
    THREAD_LOCK.unlock();
    return Response::valueResponse(key, valueItr->second.getFlags(), valueItr->second.getDataLength(),
                                   valueItr->second.getData());
}

void DataStore::getDataStoreFromFile(const std::string &filename) {
    struct stat statBuffer;
    if (stat(filename.c_str(), &statBuffer) == 0) {
        std::ifstream ifStream(filename);
        if (!ifStream.good())
            throw;
        boost::archive::binary_iarchive inputArchive(ifStream);
        inputArchive >> dataStore;
    }
}

void DataStore::flushDataStore(const std::string &filename) {
    if (!oFStream.is_open())
        oFStream.open(filename, std::fstream::out | std::fstream::binary);
    oFStream.clear();
    oFStream.seekp(0);
    if (!oFStream.good()) {
        std::cerr << "Error: " << strerror(errno);
        throw;
    }
    boost::archive::binary_oarchive outputArchive(oFStream);
    outputArchive << dataStore;
    oFStream.flush();
}

bool DataStore::get(const std::string &key, std::string &value) {
    THREAD_LOCK.lock();
    auto valueItr = dataStore.find(key);
    if (valueItr == dataStore.end()) {
        THREAD_LOCK.unlock();
        return false;
    }
    value = valueItr->second.getData();
    spdlog::debug("Get: {}: {}", key, value);
    THREAD_LOCK.unlock();
    return true;
}

void DataStore::append(const std::string &key, const std::string& value, const bool isNoReply) {
    std::lock_guard lockGuard(THREAD_LOCK);
    auto valueItr = dataStore.find(key);
    if(valueItr == dataStore.end()) {
        dataStore[key] = DataValue(value, 0, 0, value.length());
        return;
    }
    valueItr->second.appendData(" " + value);
}

void DataStore::clear() {
    std::lock_guard lockGuard(THREAD_LOCK);
    spdlog::info("Clearing data store");
    dataStore.clear();
}

ll DataStore::size() {
    std::lock_guard lockGuard(THREAD_LOCK);
    return dataStore.size();
}

const std::unordered_map<std::string, DataValue> & DataStore::getMap() const {
    return dataStore;
}


