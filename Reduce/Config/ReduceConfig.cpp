//
// Created by ankpath on 10/5/22.
//

#include "ReduceConfig.h"

ReduceConfig::ReduceConfig(const ServiceAddress &reduceAddr, const ServiceAddress &hDfsAddr) : hDFSAddr(hDfsAddr),
                                                                                               reduceAddr(reduceAddr)
                                                                                                {}

const ServiceAddress &ReduceConfig::getHdfsAddr() const {
    return hDFSAddr;
}

void ReduceConfig::setHdfsAddr(const ServiceAddress &hDfsAddr) {
    hDFSAddr = hDfsAddr;
}

const ServiceAddress &ReduceConfig::getReduceAddr() const {
    return reduceAddr;
}

void ReduceConfig::setReduceAddr(const ServiceAddress &reduceAddr) {
    ReduceConfig::reduceAddr = reduceAddr;
}
