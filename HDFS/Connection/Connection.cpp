//
// Created by ankpath on 9/13/22.
//
#include <unistd.h> // For read
#include <sys/socket.h>
#include <iostream>
#include <sys/poll.h>


#include "Connection.h"


Connection::Connection(const int connectionFd) : connectionFD(connectionFd) {

}

int Connection::read(std::string &buffer, unsigned int size) {
    struct pollfd fd;
    fd.fd = connectionFD;
    fd.events = POLLIN;
    switch (poll(&fd, 1, 30 * 1000)) {
        case -1: // Error
        case 0: // Timeout
            return -1;
    }
    int readCount = ::read(connectionFD, cStrBuffer, size);
    if (readCount < 0) {
        std::cout << "Read failed " << strerror(errno) << std::endl;
        return readCount;
    }
    buffer.clear();
    buffer.append(cStrBuffer, readCount);
    return readCount;
}

int Connection::readCommand(std::string &buffer, unsigned int size) {
    long readCount, i = 0;
    std::string singleByteBuffer = " ";
    while (i < size) {
        readCount = read(singleByteBuffer, 1);
        if (readCount <= 0)
            return readCount;
        if (singleByteBuffer[0] == '\r') {
            readCount = read(singleByteBuffer, 1);
            if (readCount <= 0)
                return readCount;
            i++;
            if (singleByteBuffer[0] == '\n') {
                i++;
                break;
            } else
                return -1;
        }
        buffer.append(singleByteBuffer, 0, 1);
        i++;
    }
    return i;

}

int Connection::send(const std::string &buffer, const unsigned int size) {
    int sendCount = ::send(connectionFD, buffer.c_str(), size, 0);
    if (sendCount < 0 || sendCount < size)
        std::cerr << "Send error: " << strerror(errno) << std::endl;
    return sendCount;
}

void Connection::close() {
    ::close(connectionFD);
}

int Connection::send(const Response &response) {
    return send(response.getRawResponse(), response.getResponseLength());
}
