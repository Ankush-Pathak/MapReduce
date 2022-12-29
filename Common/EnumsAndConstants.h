//
// Created by ankpath on 10/1/22.
//

#ifndef ASSIGNMENT2_ENUMSANDCONSTANTS_H
#define ASSIGNMENT2_ENUMSANDCONSTANTS_H
typedef unsigned long long ull;
typedef long long ll;

#include "spdlog/spdlog.h"

enum State {
    INIT, SPAWNED, WORKING, DONE, ERROR
};
extern spdlog::level::level_enum LOG_LEVEL;


#endif //ASSIGNMENT2_ENUMSANDCONSTANTS_H
