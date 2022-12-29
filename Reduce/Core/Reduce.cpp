//
// Created by ankpath on 10/5/22.
//

#include "Reduce.h"
#include "MapReduce.grpc.pb.h"
#include "Utils/Utils.h"
#include "spdlog/spdlog.h"

#include <utility>
#include <memory>
#include <grpcpp/grpcpp.h>
#include <sstream>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

std::mutex Reduce::statusVarLock;
Reduce *reduce = nullptr;
Reduce::Reduce(ServiceAddress hDfsAddr, ll noOfRecords) : hDFSAddr(std::move(hDfsAddr)),
                                                          noOfRecords(noOfRecords), percentDone(0) {}

void Reduce::fetchAndProcessInput() {
    int currentRecord = 1;
    getHDFSMetaData();
    if(noOfRecords == 0)
        setPercentDone(100);

    std::shared_ptr<HDFSService::Stub> reducerHDFSServiceStub = HDFSService::NewStub(
            grpc::CreateChannel(hDFSAddr.getTuple(), grpc::InsecureChannelCredentials()));
    ClientContext clientContext;
    clientContext.set_deadline(getDeadline(1200));
    Empty empty;
    StreamResponse streamResponse;
    std::unique_ptr<grpc::ClientReader<StreamResponse>> clientReader(reducerHDFSServiceStub->streamData(&clientContext, empty));
    while(clientReader->Read(&streamResponse) && currentRecord <= noOfRecords) {
        std::vector<std::string> values;
        std::istringstream iStringStream(streamResponse.value());
        std::string token;
        while (iStringStream >> token) {
            values.push_back(token);
        }
        std::string result = reduce(streamResponse.key(), values);
        {
            std::lock_guard lockGuard(statusVarLock);
            results.insert({streamResponse.key(), result});
        }
        ll percent = currentRecord * 100 / noOfRecords;
        setPercentDone(percent);
        if(percent % 10 == 0)
            spdlog::info("Percent done: {}", currentRecord * 100 / noOfRecords);
        currentRecord++;
    }

    Status status = clientReader->Finish();
    if(!status.ok()) {
        spdlog::error("{} returned error when fetching. Error: {} {}", hDFSAddr.getTuple(), status.error_details(), status.error_message());
        setPercentDone(-1);
    } else {
        spdlog::info("Input processed and reduced");
    }
}

int Reduce::getPercentDone() const {
    std::lock_guard lockGuard(statusVarLock);
    return percentDone;
}

void Reduce::setPercentDone(int percentDone) {
    std::lock_guard lockGuard(statusVarLock);
    Reduce::percentDone = percentDone;
}

void Reduce::runReducer() {
    spdlog::info("Running reducer");
    fetchAndProcessInput();
}

const std::unordered_map<std::string, std::string> & Reduce::getResults() const {
    return results;
}

void Reduce::setResults(const std::unordered_map<std::string, std::string> &results) {
    Reduce::results = results;
}

ll Reduce::getNoOfRecords() const {
    return noOfRecords;
}

void Reduce::setNoOfRecords(ll noOfRecords) {
    Reduce::noOfRecords = noOfRecords;
}

void Reduce::getHDFSMetaData() {
    std::shared_ptr<HDFSService::Stub> hdfsServiceStub = HDFSService::NewStub(
            grpc::CreateChannel(hDFSAddr.getTuple(), grpc::InsecureChannelCredentials()));
    ClientContext clientContext;
    clientContext.set_deadline(getDeadline(2));
    Empty empty;
    HDFSMetaData hdfsMetaData;
    Status status = hdfsServiceStub->getMetaData(&clientContext, empty, &hdfsMetaData);
    if(status.ok()) {
        noOfRecords = hdfsMetaData.noofrecords();
        spdlog::info("No of records to process: {}", noOfRecords);
    } else {
        spdlog::error("Could not fetch no of records to process");
        spdlog::error("Error: {} {}", status.error_message(), status.error_details());
    }
}
