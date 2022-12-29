//
// Created by ankpath on 10/5/22.
//

#include <memory>
#include "Reducer.h"
#include "MapReduce.grpc.pb.h"
#include "spdlog/spdlog.h"
#include "Utils/Utils.h"

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

Reducer::Reducer(const ReduceConfig &reduceConfig, State state, int percentDone)
        : reducerConfig(reduceConfig), state(state), percentDone(0), precariousLevel(0) {
    noOfRestarts = 0;
}

int Reducer::getNoOfStarts() const {
    return noOfRestarts;
}

void Reducer::setNoOfStarts(int noOfStarts) {

}

void Reducer::incrementStarts() {
    noOfRestarts++;
}

void Reducer::die() {
    std::shared_ptr<ReducerService::Stub> stub = ReducerService::NewStub(
            grpc::CreateChannel(reducerConfig.getReduceAddr().getTuple(), grpc::InsecureChannelCredentials()));
    Empty empty;
    grpc::ClientContext clientContext;
    clientContext.set_deadline(getDeadline(2));
    grpc::Status status = stub->die(&clientContext, Empty(), &empty);
    if (!status.ok()) {
        spdlog::error("Die call failed to {} because {} {}", reducerConfig.getReduceAddr().getTuple(), status.error_message(), status.error_details());
        return;
    }
    spdlog::info("Reducer {} says it's dying", reducerConfig.getReduceAddr().getTuple());
//    takeDownHDFS();
    std::this_thread::sleep_for(std::chrono::seconds(6));
}

State Reducer::getState() const {
    return state;
}

void Reducer::setState(State state) {
    Reducer::state = state;
}

const ReduceConfig &Reducer::getReducerConfig() const {
    return reducerConfig;
}

void Reducer::setReducerConfig(const ReduceConfig &reducerConfig) {
    Reducer::reducerConfig = reducerConfig;
}

int Reducer::getPercentDone() const {
    return percentDone;
}

void Reducer::setPercentDone(int percentDone) {
    Reducer::percentDone = percentDone;
}

int Reducer::getPrecariousLevel() const {
    return precariousLevel;
}

void Reducer::setPrecariousLevel(int precariousLevel) {
    Reducer::precariousLevel = precariousLevel;
}

int Reducer::incrementPrecariousLevel(int increment) {
    precariousLevel += increment;
    return precariousLevel;
}

void Reducer::takeDownHDFS() {
    std::shared_ptr<HDFSService::Stub> stub = HDFSService::NewStub(
            grpc::CreateChannel(reducerConfig.getHdfsAddr().getTuple(), grpc::InsecureChannelCredentials()));
    grpc::ClientContext clientContext;
    Empty empty;
    clientContext.set_deadline(getDeadline(3));
    grpc::Status status = stub->die(&clientContext, Empty(), &empty);
    if (!status.ok()) {
        spdlog::error("Die call failed to {} because {} {}", reducerConfig.getHdfsAddr().getTuple(), status.error_message(), status.error_details());
        return;
    }
    spdlog::info("HDFS {} says it's dying", reducerConfig.getHdfsAddr().getTuple());
}
