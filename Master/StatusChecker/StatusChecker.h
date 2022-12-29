//
// Created by ankpath on 10/2/22.
//

#ifndef ASSIGNMENT2_STATUSCHECKER_H
#define ASSIGNMENT2_STATUSCHECKER_H


#include <thread>

class StatusChecker {
    std::thread *checkerThread;
public:
    void startCheckingMappers();

    void pollMappers();

    void startCheckingReducers();

    void pollReducers();

    StatusChecker();
};


#endif //ASSIGNMENT2_STATUSCHECKER_H
