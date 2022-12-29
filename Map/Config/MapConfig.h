//
// Created by ankpath on 10/1/22.
//

#ifndef ASSIGNMENT2_MAPCONFIG_H
#define ASSIGNMENT2_MAPCONFIG_H

#include <string>
#include <vector>
#include "Common/File/File.h"

typedef unsigned long long ull;

class MapConfig {
private:
    ServiceAddress mapperAddr;
    ServiceAddress hDFSAddr;
    ll noOfRecords;
public:
    MapConfig(const ServiceAddress &mapperAddr, const ServiceAddress &hDFSAddr);

    const ServiceAddress &getMapperAddr() const;

    void setMapperAddr(const ServiceAddress &mapperAddr);

    const ServiceAddress &getHdfsAddr() const;

    void setHdfsAddr(const ServiceAddress &hDfsAddr);

    ll getNoOfRecords() const;

    void setNoOfRecords(ll noOfRecords);
};


#endif //ASSIGNMENT2_MAPCONFIG_H
