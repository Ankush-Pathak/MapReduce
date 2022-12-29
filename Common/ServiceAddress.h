//
// Created by ankpath on 10/1/22.
//

#ifndef ASSIGNMENT2_SERVICEADDRESS_H
#define ASSIGNMENT2_SERVICEADDRESS_H


#include <string>

class ServiceAddress {
    std::string host;
    int port;
public:
    ServiceAddress(const std::string &host, int port) : host(host), port(port) {}

    const std::string &getHost() const {
        return host;
    }

    void setHost(const std::string &host) {
        ServiceAddress::host = host;
    }

    int getPort() const {
        return port;
    }

    void setPort(int port) {
        ServiceAddress::port = port;
    }

    std::string getTuple() const {
        return host + ":" + std::to_string(port);
    }
};


#endif //ASSIGNMENT2_SERVICEADDRESS_H
