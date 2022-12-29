//
// Created by ankpath on 10/2/22.
//

#include <grpcpp/grpcpp.h>
#include <bits/stdc++.h>

#include "Map.h"
#include "MapReduce.pb.h"
#include "MapReduce.grpc.pb.h"
#include "spdlog/spdlog.h"
#include "Utils/Utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

Map::Map(const ll &noOfRecords, const ServiceAddress &hDfServiceAddr,
         const std::vector<ServiceAddress> &reducerDataStores) : noOfRecords(noOfRecords),
                                                                 hDFServiceAddr(hDfServiceAddr),
                                                                 reducerDataStores(reducerDataStores),
                                                                 percentDone(0){
}

int Map::getPercentDone() {
    std::lock_guard lg(statusVarLock);
    return percentDone;
}

void Map::setPercentDone(int percentDone) {
    std::lock_guard lg(statusVarLock);
    Map::percentDone = percentDone;
}

void Map::runMapper() {
    getHDFSMetaData();
    ll currentRecord = 1;
    ClientContext clientContext;
    clientContext.set_deadline(getDeadline(1200));
    StreamResponse streamResponse;
    Empty empty;
    std::shared_ptr<HDFSService::Stub> hdfsServiceStub = HDFSService::NewStub(
            grpc::CreateChannel(hDFServiceAddr.getTuple(), grpc::InsecureChannelCredentials()));
    std::unique_ptr<grpc::ClientReader<StreamResponse>> clientReader(hdfsServiceStub->streamData(&clientContext, empty));

    pairs keyValuePairs;
    while (clientReader->Read(&streamResponse) && currentRecord <= noOfRecords) {
        map(streamResponse.key(), streamResponse.value(), keyValuePairs);
        ll percent = currentRecord * 50 / noOfRecords;
        setPercentDone(percent);
        if(percent % 10 == 0)
            spdlog::info("Percent done: {}", percent);
        currentRecord++;
    }
    Status status = clientReader->Finish();
    if(status.ok()) {
        spdlog::info("Input stream processed");
    } else {
        spdlog::error("Error when processing stream");
        spdlog::error("Error: {} {}", status.error_details(), status.error_message());
        setPercentDone(-1);
    }
    pushToHDFS(keyValuePairs);
    setPercentDone(100);
    spdlog::info("Input stream mapped");
}

void Map::pushToHDFS(const pairs &keyValuePairs) {
    std::vector<std::shared_ptr<grpc::ClientWriter<SetRequest>>> writers;
    std::vector<std::shared_ptr<HDFSService::Stub>> stubs;
    for(const auto &reducerDS: reducerDataStores) {
        stubs.push_back(HDFSService::NewStub(
                grpc::CreateChannel(reducerDS.getTuple(), grpc::InsecureChannelCredentials())));
        auto &stub = stubs.back();
        writers.push_back(std::shared_ptr<grpc::ClientWriter<SetRequest>>(stub->streamAppend(new ClientContext(), new Empty())));
    }
    ll i = 1, maxI = keyValuePairs.size();
    for(const auto &pair: keyValuePairs) {
        int reducer = std::hash<std::string>{}(pair.first) % reducerDataStores.size();
        SetRequest setRequest;
        setRequest.set_key(pair.first);
        setRequest.set_value(pair.second);
        auto &writer = writers[reducer];
        if(!writer->Write(setRequest)) {
            spdlog::error("Write failed");
        }
        ll percent = 50 + (i++ * 50 / maxI);
        setPercentDone(percent);
        if(percent % 10 == 0 && i % 100000 == 0){
            spdlog::info("Percent done: {} {} of {}", percent, i, maxI);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    for(i = 0; i < writers.size(); i++) {
        auto &writer = writers[i];
        writer->WritesDone();
        Status status = writer->Finish();
        if(!status.ok()) {
            spdlog::error("Writes to {} failed {} {}", reducerDataStores[i].getTuple(), status.error_details(), status.error_message());
        }
    }

}

void Map::getHDFSMetaData() {
    std::shared_ptr<HDFSService::Stub> hdfsServiceStub = HDFSService::NewStub(
            grpc::CreateChannel(hDFServiceAddr.getTuple(), grpc::InsecureChannelCredentials()));
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
