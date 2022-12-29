//
// Created by ankpath on 9/30/22.
//

#ifndef ASSIGNMENT1_MASTERCONFIG_H
#define ASSIGNMENT1_MASTERCONFIG_H


class MasterConfig {
    int primaryPort;
    int internalServerPort;
    std::string inputHost, inputFileLocation;
    unsigned int numberOfMappers, numberOfReducers;
    MapperConfig mapperConfig;
    ReducerConfig reducerConfig;

public:
    void readConfig();

};


#endif //ASSIGNMENT1_MASTERCONFIG_H
