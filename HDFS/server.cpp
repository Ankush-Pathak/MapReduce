#include <cstdlib>
#include <iostream>
#include <functional>
#include <thread>
#include <csignal>
#include "Connection/ConnectionManager.h"
#include "Request/Request.h"
#include "Request/RetrievalRequest.h"
#include "Request/StorageRequest.h"
#include "HDFSServiceImpl.h"
#include "spdlog/spdlog.h"
#include "Common/EnumsAndConstants.h"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

void transact(Connection connection, int currentThread);

void processAndRespond(Request *request, Connection &connection);

void joinCurrentThread(std::thread (&threadPool)[THREAD_POOL_SIZE], int currentThread) {
    if (threadPool[currentThread].joinable())
        threadPool[currentThread].join();
}

void startRPCServer(int port) {
    std::string addrTuple("0.0.0.0:" + std::to_string(port));
    HDFSServiceImpl hdfsService;
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(addrTuple, grpc::InsecureServerCredentials());
    builder.RegisterService(&hdfsService);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    server->Wait();
}


void sig_handler(int signal_num) {
    dataStore->flushDataStore(DATA_STORE_FILENAME);
    std::cout << "Bye bye." << std::endl;
    exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    std::thread threadPool[THREAD_POOL_SIZE];
    spdlog::set_level(LOG_LEVEL);
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    int currentThread = 0, port = 11210;
    if (argc > 1) {
        port = std::stoi(std::string(argv[1]));
    }
    DATA_STORE_FILENAME = DATA_STORE_FILENAME + std::to_string(port);
//    DataStore dataStore;
    dataStore->getDataStoreFromFile(DATA_STORE_FILENAME);
    if (argc > 2) {
        dataStore->clear();
    }
    std::thread rpcServer(&startRPCServer, port);
    rpcServer.detach();
    ConnectionManager connectionManager(port - 1);
    while (1) {
        joinCurrentThread(threadPool, currentThread);
        Connection connection = connectionManager.accept();
        threadPool[currentThread] = std::thread(transact, connection, currentThread);
        currentThread = (currentThread + 1) % THREAD_POOL_SIZE;
    }
}

void transact(Connection connection, int currentThread) {
    std::cout << currentThread + 1 << ": Starting transaction..." << std::endl;
    std::string buffer;
    buffer.reserve(Request::MAX_REQUEST_SIZE_BYTES);
    while (1) {
        buffer.clear();
        if (connection.readCommand(buffer, Request::MAX_REQUEST_SIZE_BYTES) <= 0) {
            break;
        }
        Request *request = nullptr;
        switch (Request::getCommand(buffer)) {
            case GET:
                request = new RetrievalRequest(buffer, GET);
                break;
            case SET:
                request = new StorageRequest(buffer, SET);
                if (request->recvPayload(connection) < 0) {
                    connection.send(Response::clientErrorResponse("Malformed request"));
                    delete request;
                    continue;
                }
                break;
            default:
                std::cout << "Request not supported" << std::endl;
                connection.send(Response::clientErrorResponse("Command not supported"));
        }
        processAndRespond(request, connection);
        delete request;
    }
    connection.close();
    std::cout << currentThread + 1 << ": Connection closed..." << std::endl;
}

void processAndRespond(Request *request, Connection &connection) {
    if (request != nullptr) {
        if (!request->isValid()) {
            connection.send(Response::clientErrorResponse("Malformed Request"));
            return;
        }
        Response response = request->processRequestAndGetResponse(dataStore);
        if (connection.send(response) < 0)
            connection.close();
    }
}