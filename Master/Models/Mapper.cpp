//
// Created by ankpath on 10/1/22.
//

#include "Mapper.h"
//#include "spdlog/fmt/bundled/format.h"
#include "MapReduce.grpc.pb.h"
#include "grpcpp/create_channel.h"
#include "spdlog/spdlog.h"
#include "Utils/Utils.h"

Mapper::Mapper(const MapConfig &mapConfig, State state, int bytesDone) : mapConfig(mapConfig),
                                                                         state(state),
                                                                         percentDone(bytesDone),
                                                                         noOfStarts(0), precariousLevel(0) {}

MapConfig & Mapper::getMapConfig() {
    return mapConfig;
}

void Mapper::setMapConfig(const MapConfig &mapConfig) {
    Mapper::mapConfig = mapConfig;
}

State Mapper::getState() const {
    return state;
}

void Mapper::setState(State state) {
    Mapper::state = state;
}

int Mapper::getNoOfStarts() const {
    return noOfStarts;
}

void Mapper::setNoOfStarts(int noOfStarts) {
    Mapper::noOfStarts = noOfStarts;
}

void Mapper::incrementStarts() {
    noOfStarts++;
}

void Mapper::die() {
//    std::string addrTuple = fmt::format("{}:{}", mapConfig.getMapperAddr().getHost(), mapConfig.getMapperAddr().getPort());
    std::shared_ptr<MapperService::Stub> stub = MapperService::NewStub(
            grpc::CreateChannel(mapConfig.getMapperAddr().getTuple(), grpc::InsecureChannelCredentials()));
    MapperResult mapperResult;
    grpc::ClientContext clientContext;
    std::chrono::system_clock::time_point deadline = std::chrono::system_clock::now() + std::chrono::seconds(2);
    clientContext.set_deadline(deadline);
    grpc::Status status = stub->die(&clientContext, Empty(), &mapperResult);
    if (!status.ok()) {
        spdlog::error("Die call failed to {} because {} {}", mapConfig.getMapperAddr().getTuple(), status.error_message(), status.error_details());
        return;
    }
    spdlog::info("Mapper {} says it's dying", mapConfig.getMapperAddr().getTuple());
    std::this_thread::sleep_for(std::chrono::seconds(6));
//    takeDownHDFS();

}

int Mapper::getPercentDone() const {
    return percentDone;
}

void Mapper::setPercentDone(int percentDone) {
    Mapper::percentDone = percentDone;
}

int Mapper::getPrecariousLevel() const {
    return precariousLevel;
}

void Mapper::setPrecariousLevel(int precariousLevel) {
    Mapper::precariousLevel = precariousLevel;
}

int Mapper::incrementPrecariousLevel(int increment) {
    precariousLevel += increment;
    return precariousLevel;
}

void Mapper::takeDownHDFS() {
    std::shared_ptr<HDFSService::Stub> stub = HDFSService::NewStub(
            grpc::CreateChannel(mapConfig.getHdfsAddr().getTuple(), grpc::InsecureChannelCredentials()));
    grpc::ClientContext clientContext;
    Empty empty;
    clientContext.set_deadline(getDeadline(3));
    grpc::Status status = stub->die(&clientContext, Empty(), &empty);
    if (!status.ok()) {
        spdlog::error("Die call failed to {} because {} {}", mapConfig.getHdfsAddr().getTuple(), status.error_message(), status.error_details());
        return;
    }
    spdlog::info("HDFS {} says it's dying", mapConfig.getHdfsAddr().getTuple());
}
