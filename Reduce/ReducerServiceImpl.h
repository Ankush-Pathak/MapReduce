//
// Created by ankpath on 10/5/22.
//

#ifndef ASSIGNMENT2_REDUCERSERVICEIMPL_H
#define ASSIGNMENT2_REDUCERSERVICEIMPL_H

#include "MapReduce.grpc.pb.h"
#include "Reduce/Core/Reduce.h"
#include "spdlog/spdlog.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void reducerAutolysis();

class ReducerServiceImpl : public ReducerService::Service{
    Status startReducer(ServerContext *serverContext, const ReducerConfigRequest *reducerConfigRequest,
                       ReducerResult *reducerResult) override {
        if(reduce != nullptr) {
            return Status(grpc::StatusCode::ALREADY_EXISTS, "Reducer already running");
        }
        spdlog::info("Starting reducer");
        reduce = new Reduce(ServiceAddress(reducerConfigRequest->hdfsservice().host(),
                                           std::stoi(reducerConfigRequest->hdfsservice().port())), 0);
        std::thread reduceThread(&Reduce::runReducer, reduce);
        reduceThread.detach();
        reducerResult->set_percentdone(0);
        return Status::OK;
    }

    void populateResult(::grpc::ServerWriter<::ReducerResult> *writer) const {
        int percentDone = reduce->getPercentDone();
        spdlog::info("Reporting status: {}", percentDone);
        if(percentDone >= 100 && reduce->getResults().size() > 0) {
            for(const auto &result: reduce->getResults()) {
                ReducerResult reducerResult;
                reducerResult.set_percentdone(percentDone);
                reducerResult.set_key(result.first);
                reducerResult.set_value(result.second);
                writer->Write(reducerResult);
            }
        } else {
            ReducerResult reducerResult;
            reducerResult.set_percentdone(percentDone);
            writer->Write(reducerResult);
        }
    }

    Status getStatus(::grpc::ServerContext *context, const ::Empty *request,
                     ::grpc::ServerWriter<::ReducerResult> *writer) override {
        spdlog::info("Status queried");
        if(reduce == nullptr)
            return Status(grpc::StatusCode::UNAVAILABLE, "Reducer not started");
        populateResult(writer);
        return Status::OK;
    }

    Status die(::grpc::ServerContext *context, const ::Empty *request, ::Empty *response) override {
        spdlog::info("Reducer will die soon now");
        std::thread imploder(&reducerAutolysis);
        imploder.detach();
        return Status::OK;
    }

};


#endif //ASSIGNMENT2_REDUCERSERVICEIMPL_H
