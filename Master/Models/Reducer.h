//
// Created by ankpath on 10/5/22.
//

#ifndef ASSIGNMENT2_REDUCER_H
#define ASSIGNMENT2_REDUCER_H


#include "Common/EnumsAndConstants.h"
#include "Reduce/Config/ReduceConfig.h"

class Reducer {
    State state;
    int noOfRestarts;
    ReduceConfig reducerConfig;
    int percentDone;
    int precariousLevel;
public:
    Reducer(const ReduceConfig &reduceConfig, State state, int percentDone);

    int getNoOfStarts() const;

    void setNoOfStarts(int noOfStarts);

    void incrementStarts();

    void die();

    State getState() const;

    void setState(State state);

    const ReduceConfig &getReducerConfig() const;

    void setReducerConfig(const ReduceConfig &reducerConfig);

    int getPercentDone() const;

    void setPercentDone(int percentDone);

    int getPrecariousLevel() const;

    void setPrecariousLevel(int precariousLevel);

    int incrementPrecariousLevel(int increment);

    void takeDownHDFS();
};


#endif //ASSIGNMENT2_REDUCER_H
