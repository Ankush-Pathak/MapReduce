//
// Created by ankpath on 10/4/22.
//

#ifndef ASSIGNMENT2_MAPREDUCEIMPL_H
#define ASSIGNMENT2_MAPREDUCEIMPL_H

#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include "Common/Services/MapReduce.grpc.pb.h"
#include "Master/Master.h"
#include <thread>
//#include "MapReduce.grpc.pb.h"
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

void threadStartMapReduce(const Config *config);

Config *persistentConfig = nullptr;

class MapReduceImpl final : public MapReduce::Service {
public:
    Status startMapReduce(ServerContext *serverContext, const Config *config, Result *result) override {
        if (master != nullptr && master->getMasterState() != DONE)
            return Status::CANCELLED;
        persistentConfig = new Config(*config);
        spdlog::info("Invoking MapReduce thread");
        std::thread mapReduceThread(&threadStartMapReduce, persistentConfig);
        mapReduceThread.detach();

        std::chrono::milliseconds msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch());
        std::string ts = std::to_string(msSinceEpoch.count());
        result->set_timestamp(ts);
        result->set_healthymappers(0);
        result->set_totalmappers(0);
        result->set_healthyreducers(0);
        result->set_totalreducers(0);
        result->set_averagemappingpercentagedone(0);
        result->set_averagereducingpercentagedone(0);

        return Status::OK;
    }

    Status getStatus(ServerContext *serverContext, const Empty *empty, Result *result) override {
        std::lock_guard lg(Master::masterMutex);
        if (master == nullptr)
            return Status(grpc::StatusCode::UNAVAILABLE, "Map reduce not started");
        result->CopyFrom(master->getResult());
        return Status::OK;
    }

};

void threadStartMapReduce(const Config *config) {
    spdlog::info("Starting MapReduce");
    delete master;
    std::chrono::time_point start = std::chrono::system_clock::now();
    master = new Master(*config);
    master->buildMappersAndReducers();
    if(master->getMasterState() != ERROR)
        master->initMappers();
    if(master->getMasterState() != ERROR)
        master->initReducers();
    if(master->getMasterState() != ERROR)
        master->moveExecutablesAndStartHDFS();
    if(master->getMasterState() != ERROR)
        master->processInput();
    if(master->getMasterState() != ERROR)
        master->spawnMappers();
    if(master->getMasterState() != ERROR)
        master->standBarrier(true);
    if(master->getMasterState() != ERROR)
        master->spawnReducers();
    if(master->getMasterState() != ERROR)
        master->standBarrier(false);
    if(master->getMasterState() != ERROR)
        master->pushResultToHDFS();
    if(master->getMasterState() == ERROR)
        spdlog::error("Master encountered an error, please try again");
    master->setMasterState(DONE);
    std::chrono::time_point end = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    spdlog::info("Run took: {} ms", ms.count());
}

#endif //ASSIGNMENT2_MAPREDUCEIMPL_H
