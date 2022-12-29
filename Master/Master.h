//
// Created by ankpath on 9/30/22.
//
#include <vector>
#include <condition_variable>
#include "Common/Services/MapReduce.pb.h"
#include "Master/Models/Mapper.h"
#include "Master/StatusChecker/StatusChecker.h"
#include "Master/Models/Reducer.h"

#ifndef ASSIGNMENT2_MASTER_H
#define ASSIGNMENT2_MASTER_H


class Master {
    Config masterConfig;
    Result result;
    State state;
    std::string stateMessage;
    std::vector<Mapper> mappers;
    std::vector<Reducer> reducers;
    std::map<std::string, std::string> reducerResults;
    StatusChecker statusChecker;

public:

//    std::vector<Reducer> reducers;
    static std::mutex masterMutex;
    static int waitingForRemoteProcesses;
    static std::mutex barrierMutex;
    static std::condition_variable barrierConditionVar;

    Master(Config config) : masterConfig(config), state(INIT) {
    }

    const Config &getMasterConfig() const;

    void setMasterConfig(const Config &masterConfig);

    State getMasterState() const;

    void setMasterState(State masterState);

    void buildMappersAndReducers();

    void initMappers();

    void processInput();

    void spawnMappers();

    void spawnMapper(Mapper &mapper);

    void spawnReducer(Reducer &reducer);

    void standBarrier(bool isMapBarrier);

    void initReducers();

    void spawnReducers();

    void collectOutput();

    int getWaitingForMappers() const;

    void setWaitingForMappers(int waitingForMappers);

    std::vector<Mapper> &getMappers();

    void setMappers(const std::vector<Mapper> &mappers);

    const std::string &getStateMessage() const;

    void setStateMessage(const std::string &stateMessage);

    void killMappers();

    void killReducers();

    Result & getResult();

    void setResult(const Result &result);

    std::vector<Reducer> &getReducers();

    void setReducers(const std::vector<Reducer> &reducers);

    const std::map<std::string, std::string> & getReducerResults() const;

    void pushToReducerResults(const std::unordered_map<std::string, std::string> &results);

    void pushToReducerResults(const std::string &key, const std::string &value);

    void moveExecutablesAndStartHDFS();

    void moveMappersAndStartHDFS();

    void moveReducersAndStartHDFS();

    void pushResultToHDFS();
};

extern Master *master;


#endif //ASSIGNMENT2_MASTER_H
