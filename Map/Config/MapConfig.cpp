//
// Created by ankpath on 10/1/22.
//

#include "MapConfig.h"


MapConfig::MapConfig(const ServiceAddress &mapperAddr, const ServiceAddress &hDFSAddr)
        : mapperAddr(mapperAddr),
          hDFSAddr(hDFSAddr) {

}

const ServiceAddress &MapConfig::getMapperAddr() const {
    return mapperAddr;
}

void MapConfig::setMapperAddr(const ServiceAddress &mapperAddr) {
    MapConfig::mapperAddr = mapperAddr;
}

const ServiceAddress &MapConfig::getHdfsAddr() const {
    return hDFSAddr;
}

void MapConfig::setHdfsAddr(const ServiceAddress &hDfsAddr) {
    hDFSAddr = hDfsAddr;
}

ll MapConfig::getNoOfRecords() const {
    return noOfRecords;
}

void MapConfig::setNoOfRecords(ll noOfRecords) {
    MapConfig::noOfRecords = noOfRecords;
}
