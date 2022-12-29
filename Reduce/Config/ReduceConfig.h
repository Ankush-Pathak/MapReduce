//
// Created by ankpath on 10/5/22.
//

#ifndef ASSIGNMENT2_REDUCECONFIG_H
#define ASSIGNMENT2_REDUCECONFIG_H


#include <unordered_set>
#include "Common/ServiceAddress.h"

class ReduceConfig {
    ServiceAddress hDFSAddr;
    ServiceAddress reduceAddr;
public:
    ReduceConfig(const ServiceAddress &reduceAddr, const ServiceAddress &hDfsAddr);

    const ServiceAddress &getHdfsAddr() const;

    void setHdfsAddr(const ServiceAddress &hDfsAddr);

    const ServiceAddress &getReduceAddr() const;

    void setReduceAddr(const ServiceAddress &reduceAddr);

};


#endif //ASSIGNMENT2_REDUCECONFIG_H
