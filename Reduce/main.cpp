//
// Created by ankpath on 10/5/22.
//
#include <string>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "ReducerServiceImpl.h"

void startRPCServer(int port) {
    std::string addrTuple("0.0.0.0:" + std::to_string(port));
    ReducerServiceImpl reducerService;
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(addrTuple, grpc::InsecureServerCredentials());
    builder.RegisterService(&reducerService);
    // Finally assemble the server.
    std::unique_ptr<Server> server(builder.BuildAndStart());
    spdlog::info("Server started on {}", port);
    server->Wait();
}

int main(int argc, char *argv[]) {
    spdlog::set_level(LOG_LEVEL);
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    int currentThread = 0, port = 11211;
    if (argc > 1)
        port = std::stoi(std::string(argv[1]));
    startRPCServer(port);
}