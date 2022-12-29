//
// Created by ankpath on 10/5/22.
//

#include "ReducerServiceImpl.h"

void reducerAutolysis() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    spdlog::info("Bye bye");
    exit(0);
}