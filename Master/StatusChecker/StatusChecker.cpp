//
// Created by ankpath on 10/2/22.
//

#include <mutex>
#include "StatusChecker.h"
#include "Master/Master.h"
#include "MapReduce.grpc.pb.h"
#include "spdlog/spdlog.h"
#include "grpcpp/create_channel.h"
#include "Utils/Utils.h"

StatusChecker::StatusChecker() {}

void StatusChecker::startCheckingMappers() {
    spdlog::info("Starting Map status checking thread");
    checkerThread = new std::thread(&StatusChecker::pollMappers, this);
    checkerThread->detach();
}

void StatusChecker::pollMappers() {
    while (1) {
        {
            std::lock_guard masterLockGuard(Master::masterMutex);
            std::lock_guard lockGuard(Master::barrierMutex);
            int healthyMappers = 0;
            int avgDone = 0;
            int index = 0;
            for (Mapper &mapper: master->getMappers()) {
                if (mapper.getState() == WORKING) {
                    spdlog::info("Checking in on working mapper: {}", mapper.getMapConfig().getMapperAddr().getTuple());
                    std::shared_ptr<MapperService::Stub> stub = MapperService::NewStub(
                            grpc::CreateChannel(mapper.getMapConfig().getMapperAddr().getTuple(), grpc::InsecureChannelCredentials()));
                    MapperResult mapperResult;
                    grpc::ClientContext clientContext;
                    clientContext.set_deadline(getDeadline(2));
                    if (!stub->getStatus(&clientContext, Empty(), &mapperResult).ok() || mapperResult.percentdone() == -1) {
                        spdlog::error("Mapper: {} seems to be down or in an error state", mapper.getMapConfig().getMapperAddr().getTuple());
                        if (mapper.getNoOfStarts() > 3) {
                            master->setMasterState(ERROR);
                            master->setStateMessage(
                                    fmt::format("Mapper {} start attempts more than three, imploding...", mapper.getMapConfig().getMapperAddr().getTuple()));
                            master->killMappers();
                            Master::waitingForRemoteProcesses = 0;
                            break;
                        }
                        mapper.setState(ERROR);
                        spdlog::info("Respawning mapper: {}", mapper.getMapConfig().getMapperAddr().getTuple());
                        master->spawnMapper(mapper);
                    } else {
                        spdlog::info("Mapper {} done: {}%",
                                     mapper.getMapConfig().getMapperAddr().getTuple(), mapperResult.percentdone());
                        healthyMappers++;
                        avgDone += mapperResult.percentdone();
                        if(mapper.getPercentDone() >= mapperResult.percentdone()) {
                            if(mapper.getPrecariousLevel() >= 3) {
                                if (mapper.getNoOfStarts() > 3) {
                                    master->setMasterState(ERROR);
                                    master->setStateMessage(
                                            fmt::format("Mapper {} start attempts more than three, imploding...", mapper.getMapConfig().getMapperAddr().getTuple()));
                                    master->killMappers();
                                    Master::waitingForRemoteProcesses = 0;
                                    break;
                                }
                                spdlog::warn("Mapper {} has precarious level {}, restarting", mapper.getMapConfig().getMapperAddr().getTuple(), mapper.getPrecariousLevel());
                                mapper.setState(ERROR);
                                mapper.die();
                                Master::waitingForRemoteProcesses--;
                                master->spawnMapper(mapper);
                            }
                            mapper.incrementPrecariousLevel(1);
                        } else if (mapperResult.percentdone() >= 100) {
                                mapper.setState(DONE);
                                mapper.die();
                                Master::waitingForRemoteProcesses--;
                        } else {
                            mapper.setPercentDone(mapperResult.percentdone());
                        }
                    }
                } else if (mapper.getState() == ERROR){
                    if (mapper.getNoOfStarts() > 3) {
                        master->setMasterState(ERROR);
                        master->setStateMessage(
                                fmt::format("Mapper {} start attempts more than three, imploding...", mapper.getMapConfig().getMapperAddr().getTuple()));
                        master->killMappers();
                        Master::waitingForRemoteProcesses = 0;
                        break;
                    }
                    mapper.setState(ERROR);
                    spdlog::info("Respawning mapper: {}", mapper.getMapConfig().getMapperAddr().getTuple());
                    master->spawnMapper(mapper);
                }
                index++;
            }
            avgDone /= master->getMappers().size();
            std::chrono::milliseconds msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
            std::string ts = std::to_string(msSinceEpoch.count());
            Result &result = master->getResult();
            result.set_timestamp(ts);
            result.set_totalmappers(master->getMappers().size());
            result.set_healthymappers(healthyMappers);
            result.set_averagemappingpercentagedone(avgDone);
            if (Master::waitingForRemoteProcesses <= 0) {
                spdlog::info("Mappers done");
                if (Master::waitingForRemoteProcesses < 0)
                    spdlog::error("Inconsistent state, waitingForRemoteProcesses is negative");
                Master::barrierConditionVar.notify_one();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

}

void StatusChecker::startCheckingReducers() {
    spdlog::info("Starting Reducer status checking thread");
    checkerThread = new std::thread(&StatusChecker::pollReducers, this);
    checkerThread->detach();
}

void StatusChecker::pollReducers() {
    while (1) {
        {
            std::lock_guard masterLockGuard(Master::masterMutex);
            std::lock_guard lockGuard(Master::barrierMutex);
            int healthyReducers = 0;
            int avgDone = 0;
            int index = 0;
            for (Reducer &reducer: master->getReducers()) {
                if (reducer.getState() == WORKING) {
                    spdlog::info("Checking in on working reducer: {}", reducer.getReducerConfig().getReduceAddr().getTuple());
                    std::shared_ptr<ReducerService::Stub> stub = ReducerService::NewStub(
                            grpc::CreateChannel(reducer.getReducerConfig().getReduceAddr().getTuple(), grpc::InsecureChannelCredentials()));
                    grpc::ClientContext clientContext;
                    clientContext.set_deadline(getDeadline(300));
                    Empty empty;
                    ReducerResult lReducerResult;
                    ll percentDone = -1;
                    std::shared_ptr<grpc::ClientReader<ReducerResult>> clientReader(stub->getStatus(&clientContext, empty));
                    while(clientReader->Read(&lReducerResult)) {
                        percentDone = lReducerResult.percentdone();
                        if(percentDone>=100 && lReducerResult.key().size() > 0) {
                            master->pushToReducerResults(lReducerResult.key(), lReducerResult.value());
                        }
                    }
                    grpc::Status status = clientReader->Finish();
                    if (!status.ok()) {
                        spdlog::error("Reducer: {} seems to be down", reducer.getReducerConfig().getReduceAddr().getTuple());
                        if (reducer.getNoOfStarts() > 3) {
                            master->setMasterState(ERROR);
                            master->setStateMessage(
                                    fmt::format("Reducer {} start attempts more than three, imploding...", reducer.getReducerConfig().getReduceAddr().getTuple()));
                            master->killReducers();
                            Master::waitingForRemoteProcesses = 0;
                            break;
                        }
                        reducer.setState(ERROR);
                        spdlog::info("Respawning reducer: {}", reducer.getReducerConfig().getReduceAddr().getTuple());
                        master->spawnReducer(reducer);
                    } else {
                        spdlog::info("Reducer {} done: {}%", reducer.getReducerConfig().getReduceAddr().getTuple(), percentDone);
                        healthyReducers++;
                        avgDone += percentDone;
                        if(reducer.getPercentDone() >= percentDone) {
                            if(reducer.getPrecariousLevel() >= 3) {
                                if (reducer.getNoOfStarts() > 3) {
                                    master->setMasterState(ERROR);
                                    master->setStateMessage(
                                            fmt::format("Reducer {} start attempts more than three, imploding...", reducer.getReducerConfig().getReduceAddr().getTuple()));
                                    master->killReducers();
                                    Master::waitingForRemoteProcesses = 0;
                                    break;
                                }
                                spdlog::warn("Reducer {} has precarious level {}, restarting", reducer.getReducerConfig().getReduceAddr().getTuple(), reducer.getPrecariousLevel());
                                reducer.setState(ERROR);
                                reducer.die();
                                Master::waitingForRemoteProcesses--;
                                master->spawnReducer(reducer);
                            }
                            reducer.incrementPrecariousLevel(1);
                        } else if (percentDone >= 100) {
                            spdlog::info("Collected results from: {} Size: {}", reducer.getReducerConfig().getReduceAddr().getTuple(), master->getReducerResults().size());
                            reducer.setState(DONE);
                            reducer.die();
                            Master::waitingForRemoteProcesses--;
                        } else {
                            reducer.setPercentDone(percentDone);
                        }
                    }
                } else if(reducer.getState() == ERROR){
                    if (reducer.getNoOfStarts() > 3) {
                        master->setMasterState(ERROR);
                        master->setStateMessage(
                                fmt::format("Reducer {} start attempts more than three, imploding...", reducer.getReducerConfig().getReduceAddr().getTuple()));
                        master->killReducers();
                        Master::waitingForRemoteProcesses = 0;
                        break;
                    }
                    reducer.setState(ERROR);
                    spdlog::info("Respawning reducer: {}", reducer.getReducerConfig().getReduceAddr().getTuple());
                    master->spawnReducer(reducer);
                }
                index++;
            }
            avgDone /= master->getReducers().size();
            std::chrono::milliseconds msSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch());
            std::string ts = std::to_string(msSinceEpoch.count());
            Result &result = master->getResult();
            result.set_timestamp(ts);
            result.set_totalreducers(master->getReducers().size());
            result.set_healthyreducers(healthyReducers);
            result.set_averagereducingpercentagedone(avgDone);
            if (Master::waitingForRemoteProcesses <= 0) {
                spdlog::info("Reducers done");
                if (Master::waitingForRemoteProcesses < 0)
                    spdlog::error("Inconsistent state, waitingForRemoteProcesses is negative");
                Master::barrierConditionVar.notify_one();
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }


}
