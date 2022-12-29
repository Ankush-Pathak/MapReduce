#include "spdlog/spdlog.h"
#include "Master/MapReduceService/MapReduceImpl.h"

//
// Created by ankpath on 10/1/22.
//
#include <string>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

void startRPCServer(int port) {
    std::string addrTuple("0.0.0.0:" + std::to_string(port));
    MapReduceImpl mapReduce;
    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    grpc::ServerBuilder builder;
    builder.AddListeningPort(addrTuple, grpc::InsecureServerCredentials());
    builder.RegisterService(&mapReduce);
    // Finally assemble the server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    spdlog::info("Server up on: {}", port);
    server->Wait();
}

int main(int argc, char *argv[]) {
    int currentThread = 0, port = 4000;
    if (argc > 1)
        port = std::stoi(std::string(argv[1]));
    spdlog::set_level(LOG_LEVEL);
    spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    startRPCServer(port);

}