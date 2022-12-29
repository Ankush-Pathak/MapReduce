//
// Created by ankpath on 10/2/22.
//

#ifndef ASSIGNMENT2_MAPPERSERVICEIMPL_H
#define ASSIGNMENT2_MAPPERSERVICEIMPL_H


#include <thread>
#include "Common/Services/MapReduce.grpc.pb.h"
#include "Map/Core/Map.h"
#include "spdlog/spdlog.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void mapperAutolysis();

class MapperServiceImpl final : public MapperService::Service {
    Status startMapper(ServerContext *serverContext, const MapperConfigRequest *mapperConfigRequest,
                       MapperResult *mapperResult) override {
        // TODO: Protect map object with a mutex
        if (mapPtr != nullptr) {
            mapperResult->set_percentdone(-1);
            return {grpc::StatusCode::ALREADY_EXISTS, "Mapper already started"};
        }
        ServiceAddress hDFSAddr(mapperConfigRequest->hdfsservice().host(), std::stoi(mapperConfigRequest->hdfsservice().port()));
        std::vector<ServiceAddress> reducerDataStores;
        for (const auto& dataStore: mapperConfigRequest->reducerhdfsservices()) {
            reducerDataStores.push_back({dataStore.host(), stoi(dataStore.port())});
        }
        spdlog::info("Received start request, creating map object");
        mapPtr = new Map(0, hDFSAddr,
                         reducerDataStores);
        std::thread mapThread(&Map::runMapper, mapPtr);
        mapThread.detach();
        mapperResult->set_percentdone(0);
        spdlog::info("Mapper thread created and detached");
        return Status::OK;
    }

    Status getStatus(ServerContext *serverContext, const Empty *empty, MapperResult *mapperResult) override {
        spdlog::info("Status query received");
        if (mapPtr == nullptr) {
            mapperResult->set_percentdone(-1);
            return {grpc::StatusCode::UNAVAILABLE, "Mapper not started"};
        }
        mapperResult->set_percentdone(mapPtr->getPercentDone());
        spdlog::info("Returning status: {}", mapperResult->percentdone());
        return Status::OK;
    }

    Status die(ServerContext *serverContext, const Empty *empty, MapperResult *mapperResult) override {
        spdlog::info("Mapper is imploding soon");
        mapperResult->set_percentdone(-1);
        if (mapPtr != nullptr) {
            mapperResult->set_percentdone(mapPtr->getPercentDone());
        }
        std::thread imploder(&mapperAutolysis);
        imploder.detach();
        return Status::OK;
    }
};


#endif //ASSIGNMENT2_MAPPERSERVICEIMPL_H
