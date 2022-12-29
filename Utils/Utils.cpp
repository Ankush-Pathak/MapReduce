//
// Created by ankpath on 10/5/22.
//
#include "Utils.h"

std::chrono::time_point<std::chrono::system_clock> getDeadline(int seconds) {
    return std::chrono::system_clock::now() + std::chrono::seconds(seconds);
}