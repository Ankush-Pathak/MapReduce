//
// Created by ankpath on 9/30/22.
//

//#include "Master.h"
#include <sys/stat.h>
#include <grpcpp/grpcpp.h>
#include "Common/Services/MapReduce.pb.h"
#include "Master.h"
#include "spdlog/spdlog.h"
#include "MapReduce.grpc.pb.h"
#include "Utils/Utils.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

Master *master = nullptr;
std::mutex Master::masterMutex;
int Master::waitingForRemoteProcesses = 0;
std::mutex Master::barrierMutex;
std::condition_variable Master::barrierConditionVar;

const Config &Master::getMasterConfig() const {
    return masterConfig;
}

void Master::setMasterConfig(const Config &masterConfig) {
    Master::masterConfig = masterConfig;
}

State Master::getMasterState() const {
    return state;
}

void Master::setMasterState(State masterState) {
    Master::state = masterState;
}

// TODO: Adapt for remote files
void Master::processInput() {
    spdlog::info("Starting input processing");
    struct stat buffer;
    File file(masterConfig.inputfilepath(), masterConfig.filehost());
    if (stat(masterConfig.inputfilepath().c_str(), &buffer) != 0) {
        state = ERROR;
        stateMessage = fmt::format("Could not find file: {}, error: {}", masterConfig.inputfilepath(), strerror(errno));
        char buff[1024];
        if(getcwd(buff, 1024) == NULL) {
            spdlog::error("getcwd failed {}", strerror(errno));
        }
        spdlog::error("Current directory: {}", buff);
        spdlog::error(stateMessage);
        return;
    }
    file.open();
    ll noOfLines = file.getNoOfLines();
    ll splitSize = noOfLines / mappers.size();
    ll linesPushed;
    for(int i = 0; i < mappers.size() - 1; i++) {
        spdlog::info("Working on input for mapper {}, split size: {} of {}", i + 1, splitSize, noOfLines);
        const ServiceAddress &hDFSAddr = mappers[i].getMapConfig().getHdfsAddr();
        linesPushed = file.populateHDFS(hDFSAddr, splitSize, '\n');
        mappers[i].getMapConfig().setNoOfRecords(linesPushed);
    }
    spdlog::info("Working on input for last mapper");
    const ServiceAddress &hDFSAddr = mappers.back().getMapConfig().getHdfsAddr();
    linesPushed = file.populateHDFS(hDFSAddr, -1, '\n');
    mappers.back().getMapConfig().setNoOfRecords(linesPushed);
}


void Master::spawnMappers() {
    waitingForRemoteProcesses = 0;
    for (Mapper &mapper: mappers) {
        if(state == ERROR)
            return;
        spawnMapper(mapper);
    }
}

void Master::standBarrier(bool isMapBarrier) {
    spdlog::info("Invoking status checker");
    if(isMapBarrier)
        statusChecker.startCheckingMappers();
    else
        statusChecker.startCheckingReducers();
    spdlog::info("Bringing barrier up");
    std::unique_lock<std::mutex> lockGuard(barrierMutex);
    barrierConditionVar.wait(lockGuard, [] { return waitingForRemoteProcesses == 0; });
    spdlog::info("Barrier is down");
}

void Master::spawnReducers() {
    waitingForRemoteProcesses = 0;
    for(Reducer &reducer: reducers) {
        if(state == ERROR)
            return;
        spawnReducer(reducer);
    }

}

void Master::collectOutput() {
    for(const auto &pair: reducerResults) {
        spdlog::debug("{}: {}", pair.first, pair.second);
    }

}

void Master::buildMappersAndReducers() {
    spdlog::info("Compiling Mapper and Reducer");
    struct stat mapperStat, reducerStat;
    if (stat(masterConfig.mapperfilepath().c_str(), &mapperStat) != 0 ||
        stat(masterConfig.reducerfilepath().c_str(), &reducerStat) != 0) {
        state = ERROR;
        std::string message = fmt::format("Reducer({}) or mapper({}) file does not exist",
                                          masterConfig.mapperfilepath(), masterConfig.reducerfilepath());
        spdlog::error(message);
        stateMessage = message;
        return;
    }
    std::string command =
            "bash ../compile_mapper_reducer.sh " + masterConfig.mapperfilepath() + " " + masterConfig.reducerfilepath() + " 2>&1 >> compiler.log";
    if (system(command.c_str()) != 0) {
        state = ERROR;
        stateMessage = "Mapper or reducer compilation failed.";
        spdlog::error(stateMessage);
    }

}

int Master::getWaitingForMappers() const {
    return waitingForRemoteProcesses;
}

void Master::setWaitingForMappers(int waitingForMappers) {
    Master::waitingForRemoteProcesses = waitingForMappers;
}

std::vector<Mapper> &Master::getMappers() {
    return mappers;
}

void Master::setMappers(const std::vector<Mapper> &mappers) {
    Master::mappers = mappers;
}

void Master::spawnMapper(Mapper &mapper) {
    std::string command = "ssh " + mapper.getMapConfig().getMapperAddr().getHost() + " 'bash start_mapper.sh " +
              std::to_string(mapper.getMapConfig().getMapperAddr().getPort()) + " 2>&1 >>mapper"+ std::to_string(mapper.getMapConfig().getMapperAddr().getPort()) + ".log'";
    spdlog::info("Starting mapper: {}", mapper.getMapConfig().getMapperAddr().getTuple());
    mapper.incrementStarts();

    if (system(command.c_str()) != 0) {
        state = ERROR;
        stateMessage = fmt::format("Mapper could not be spawned on host {}",
                                   mapper.getMapConfig().getMapperAddr().getHost());
        spdlog::error(stateMessage);
        return;
    }
    mapper.setState(SPAWNED);
    waitingForRemoteProcesses++;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::shared_ptr<MapperService::Stub> stub = MapperService::NewStub(
            grpc::CreateChannel(mapper.getMapConfig().getMapperAddr().getTuple(), grpc::InsecureChannelCredentials()));
    MapperResult mapperResult;
    ClientContext clientContext;
    clientContext.set_deadline(getDeadline(2));
    MapperConfigRequest mapperConfigRequest;
    Host *host = mapperConfigRequest.mutable_hdfsservice();
    host->set_host(mapper.getMapConfig().getHdfsAddr().getHost());
    host->set_port(std::to_string(mapper.getMapConfig().getHdfsAddr().getPort()));
    for(Reducer &reducer: reducers) {
        auto hdfsService = mapperConfigRequest.mutable_reducerhdfsservices()->Add();
        hdfsService->set_host(reducer.getReducerConfig().getHdfsAddr().getHost());
        hdfsService->set_port(std::to_string(reducer.getReducerConfig().getHdfsAddr().getPort()));
    }
    spdlog::info("Configuring mapper: {}", mapper.getMapConfig().getMapperAddr().getTuple());
    Status status = stub->startMapper(&clientContext, mapperConfigRequest, &mapperResult);
    if (!status.ok()) {
        spdlog::error("Remote mapper config failed.");
        spdlog::error("Error: {} {}", status.error_details(), status.error_message());
        mapper.setState(ERROR);
    } else {
        spdlog::info("Mapper Done: {}%", mapperResult.percentdone());
        mapper.setState(WORKING);
    }
}

const std::string &Master::getStateMessage() const {
    return stateMessage;
}

void Master::setStateMessage(const std::string &stateMessage) {
    Master::stateMessage = stateMessage;
}

void Master::killMappers() {
    for (Mapper &mapper: mappers) {
        mapper.die();
        mapper.setState(ERROR);
    }

}

Result & Master::getResult() {
    return result;
}

void Master::setResult(const Result &result) {
    Master::result = result;
}

void Master::initReducers() {
    for(const Host& host: masterConfig.reducers()) {
        ReduceConfig reduceConfig(
                ServiceAddress(host.host(), std::stoi(host.port())),
                ServiceAddress(host.host(), std::stoi(host.port()) - 1));
        reducers.push_back(Reducer(reduceConfig, INIT, 0));

    }
    spdlog::info("Reducer init done");
}

void Master::spawnReducer(Reducer &reducer) {
    std::string reducerHost = reducer.getReducerConfig().getReduceAddr().getHost();
    std::string command = fmt::format("ssh {} 'bash start_reducer.sh {} 2>&1 >>reducer.log'",
                                      reducerHost,
                                      reducer.getReducerConfig().getReduceAddr().getPort());
    spdlog::info("Starting reducer: {}", reducer.getReducerConfig().getReduceAddr().getTuple());
    reducer.incrementStarts();
    if (system(command.c_str()) != 0) {
        state = ERROR;
        stateMessage = fmt::format("Reducer could not be spawned on host {}",
                                   reducerHost);
        spdlog::error(stateMessage);
        return;
    }
    reducer.setState(SPAWNED);
    waitingForRemoteProcesses++;

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::shared_ptr<ReducerService::Stub> stub = ReducerService::NewStub(
            grpc::CreateChannel(reducer.getReducerConfig().getReduceAddr().getTuple(),
                                grpc::InsecureChannelCredentials()));
    ReducerResult reducerResult;
    ClientContext clientContext;
    clientContext.set_deadline(getDeadline(2));
    ReducerConfigRequest reducerConfigRequest;
    auto hdfsService = reducerConfigRequest.mutable_hdfsservice();
    hdfsService->set_host(reducer.getReducerConfig().getHdfsAddr().getHost());
    hdfsService->set_port(std::to_string(reducer.getReducerConfig().getHdfsAddr().getPort()));

    spdlog::info("Configuring reducer: {}", reducer.getReducerConfig().getReduceAddr().getTuple());
    Status status = stub->startReducer(&clientContext, reducerConfigRequest, &reducerResult);
    if (!status.ok()) {
        spdlog::error("Remote reducer config failed. {}: {}", status.error_details(), status.error_message());
        reducer.setState(ERROR);
    } else {
        spdlog::info("Reducer Done: {}%", reducerResult.percentdone());
        reducer.setState(WORKING);
    }

}

void Master::killReducers() {
    for(Reducer &reducer: reducers) {
        reducer.die();
        reducer.setState(ERROR);
    }
}

std::vector<Reducer> &Master::getReducers() {
    return reducers;
}

void Master::setReducers(const std::vector<Reducer> &reducers) {
    Master::reducers = reducers;
}

const std::map<std::string, std::string> & Master::getReducerResults() const {
    return reducerResults;
}

void Master::pushToReducerResults(const std::unordered_map<std::string, std::string> &results) {
    reducerResults.insert(results.begin(), results.end());
}

void Master::moveExecutablesAndStartHDFS() {
    moveMappersAndStartHDFS();
    moveReducersAndStartHDFS();

}

void Master::moveMappersAndStartHDFS() {
    std::string command;
    for(Mapper &mapper: mappers) {
        const std::string &mapperHost = mapper.getMapConfig().getMapperAddr().getHost();
        const int &mapperPort = mapper.getMapConfig().getMapperAddr().getPort();
        const int &hdfsPort = mapper.getMapConfig().getHdfsAddr().getPort();
        command = fmt::format("ssh {} 'mkdir -p .mapreduce' && "
                              "rsync -c Mapper {}:.mapreduce/Mapper && rsync -c HDFS {}:.mapreduce/HDFS && "
                              "rsync -c ../start_mapper.sh {}:start_mapper.sh && "
                              "rsync -c ../start_hdfs.sh {}:start_hdfs.sh",
                              mapperHost, mapperHost, mapperHost, mapperHost, mapperHost);
        spdlog::info("Moving mapper stuff to target host {}", mapperHost);
        if (system(command.c_str()) != 0) {
            state = ERROR;
            stateMessage = fmt::format("Mapper executables movement to target host {} failed",
                                       mapperHost);
            spdlog::error(stateMessage);
            return;
        }

        command = fmt::format("ssh {} 'bash start_hdfs.sh {} 2>&1 >>hdfs{}.log'",
                              mapperHost,
                              std::to_string(hdfsPort),
                              std::to_string(hdfsPort));
        spdlog::info("Starting HDFS on {}:{}", mapperHost, hdfsPort);
        if (system(command.c_str()) != 0) {
            state = ERROR;
            stateMessage = fmt::format("HDFS for mapper startup on host {} failed",
                                       mapperHost);
            spdlog::error(stateMessage);
            return;
        }
    }
}

void Master::moveReducersAndStartHDFS() {
    std::string command;
    for(Reducer &reducer: reducers) {
        const std::string &reducerHost = reducer.getReducerConfig().getReduceAddr().getHost();
        const int &reducerPort = reducer.getReducerConfig().getReduceAddr().getPort();
        const int &hdfsPort = reducerPort - 1;
        command = fmt::format("ssh {} 'mkdir -p .mapreduce' && "
                              "rsync -c Reducer {}:.mapreduce/Reducer && rsync -c HDFS {}:.mapreduce/HDFS && "
                              "rsync -c ../start_reducer.sh {}:start_reducer.sh && "
                              "rsync -c ../start_hdfs.sh {}:start_hdfs.sh",
                              reducerHost, reducerHost, reducerHost, reducerHost, reducerHost);
        spdlog::info("Moving reducer stuff to target host {}", reducerHost);
        if (system(command.c_str()) != 0) {
            state = ERROR;
            stateMessage = fmt::format("Reducer executables movement to target host {} failed",
                                       reducerHost);
            spdlog::error(stateMessage);
            return;
        }

        command = fmt::format("ssh {} 'bash start_hdfs.sh {} 2>&1 >>hdfs{}.log'",
                              reducerHost,
                              std::to_string(hdfsPort),
                              std::to_string(hdfsPort));
        spdlog::info("Starting HDFS on {}:{}", reducerHost, hdfsPort);
        if (system(command.c_str()) != 0) {
            state = ERROR;
            stateMessage = fmt::format("HDFS for reducer startup on host {} failed",
                                       reducerHost);
            spdlog::error(stateMessage);
            return;
        }
    }
}

void Master::initMappers() {
    for(const Host& host: masterConfig.mappers()) {
        MapConfig mapConfig(ServiceAddress(host.host(), std::stoi(host.port())),
                            ServiceAddress(host.host(), std::stoi(host.port()) - 1));
        mappers.push_back(Mapper(mapConfig, INIT, 0));
    }
    spdlog::info("Mapper init done");
}

void Master::pushToReducerResults(const std::string &key, const std::string &value) {
    reducerResults.insert({key, value});
}

void Master::pushResultToHDFS() {
    std::fstream outputFile("output", std::ios::out);
    ServiceAddress outputHDFSService(masterConfig.outputhdfs().host(), std::stoi(masterConfig.outputhdfs().port()));
    std::string command = fmt::format("nohup ./HDFS {} clear 2>&1 >>hdfs{}.log &", masterConfig.outputhdfs().port(), masterConfig.outputhdfs().port());
    if(system(command.c_str()) != 0) {
        spdlog::error("Could not start HDFS for dumping output");
        return;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::shared_ptr<HDFSService::Stub> stub = HDFSService::NewStub(
            grpc::CreateChannel(outputHDFSService.getTuple(),
                                grpc::InsecureChannelCredentials()));
    for(const auto &pair: reducerResults) {
        ClientContext clientContext;
        clientContext.set_deadline(getDeadline(2));
        SetRequest setRequest;
        SetResponse setResponse;
        setRequest.set_key(pair.first);
        setRequest.set_value(pair.second);
        Status status = stub->set(&clientContext, setRequest, &setResponse);
        if(!status.ok()) {
            spdlog::error("Could not push {}", pair.first);
        }
        outputFile<<pair.first<<" "<<pair.second<<std::endl;
    }
    outputFile.flush();
    outputFile.close();
    ClientContext clientContext;
    clientContext.set_deadline(getDeadline(2));
    Empty empty;
    stub->die(&clientContext, Empty(),&empty);
    spdlog::info("Results pushed to HDFS {}", outputHDFSService.getTuple());
}
