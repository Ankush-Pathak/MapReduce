//
// Created by ankpath on 10/5/22.
//

#ifndef ASSIGNMENT2_REDUCE_H
#define ASSIGNMENT2_REDUCE_H


#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <vector>
#include "Common/ServiceAddress.h"
#include "Common/EnumsAndConstants.h"

class Reduce {
    ServiceAddress hDFSAddr;
    std::unordered_map<std::string, std::string> results;
    int percentDone;
    ll noOfRecords;
public:
    static std::mutex statusVarLock;
    void runReducer();
    void fetchAndProcessInput();

    std::string reduce(const std::string &key, const std::vector<std::string> &values);

    Reduce(ServiceAddress hDfsAddr, ll noOfRecords);

    int getPercentDone() const;

    void setPercentDone(int percentDone);

    const std::unordered_map<std::string, std::string> & getResults() const;

    void setResults(const std::unordered_map<std::string, std::string> &results);

    ll getNoOfRecords() const;

    void setNoOfRecords(ll noOfRecords);

    void getHDFSMetaData();

};

extern Reduce *reduce;

#endif //ASSIGNMENT2_REDUCE_H
