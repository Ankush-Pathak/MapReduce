//
// Created by ankpath on 10/1/22.
//

#ifndef ASSIGNMENT2_MAPPER_H
#define ASSIGNMENT2_MAPPER_H


#include <mutex>
#include "Map/Config/MapConfig.h"
#include "Common/EnumsAndConstants.h"


class Mapper {
    MapConfig mapConfig;
    State state;
    int percentDone;
    int noOfStarts;
    int precariousLevel;
public:
    Mapper(const MapConfig &mapConfig, State state, int percentDone);

    MapConfig & getMapConfig();

    void setMapConfig(const MapConfig &mapConfig);

    State getState() const;

    void setState(State state);

    int getPercentDone() const;

    void setPercentDone(int percentDone);

    int getPrecariousLevel() const;

    void setPrecariousLevel(int precariousLevel);

    int incrementPrecariousLevel(int increment);

    int getNoOfStarts() const;

    void setNoOfStarts(int noOfStarts);

    void incrementStarts();

    void die();

    void takeDownHDFS();
};


#endif //ASSIGNMENT2_MAPPER_H
