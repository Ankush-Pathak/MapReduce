//
// Created by ankpath on 10/2/22.
//

#include "MapperServiceImpl.h"

Map *mapPtr = nullptr;
void mapperAutolysis() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    spdlog::info("Bye bye");
    exit(0);
}