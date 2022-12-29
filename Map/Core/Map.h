//
// Created by ankpath on 10/2/22.
//

#ifndef ASSIGNMENT2_MAP_H
#define ASSIGNMENT2_MAP_H

#include <vector>
#include <mutex>
#include "Common/ServiceAddress.h"
#include "Common/EnumsAndConstants.h"
#include "MapReduce.grpc.pb.h"
typedef std::vector<std::pair<std::string,std::string>> pairs;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class Map {
    ServiceAddress hDFServiceAddr;
    std::vector<ServiceAddress> reducerDataStores;
    std::mutex statusVarLock;
    int percentDone;
    ll noOfRecords;
public:

    void map(const std::string &lineOffset, const std::string &line, pairs &keyValuePairs);

    void pushToHDFS(const pairs &keyValuePairs);

    Map(const ll &noOfRecords, const ServiceAddress &hDfServiceAddr,
        const std::vector<ServiceAddress> &reducerDataStores);

    int getPercentDone();

    void setPercentDone(int percentDone);

    void runMapper();

    void getHDFSMetaData();
};

extern Map *mapPtr;


#endif //ASSIGNMENT2_MAP_H
